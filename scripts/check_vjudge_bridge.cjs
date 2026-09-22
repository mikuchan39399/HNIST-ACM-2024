'use strict';
const assert = require('node:assert/strict');
const vm = require('node:vm');
const fs = require('node:fs');
const path = require('node:path');
const { problemUrl, isVjudge, createSession } = require('./vjudge_bridge.cjs');
const { zoiVjudgePage } = require('./vjudge-extension/page.js');
const extension = path.join(__dirname, 'vjudge-extension');

async function protocolChecks() {
    for (const url of ['http://vjudge.net/problem/CodeForces-1A', 'https://vjudge.net.evil.test/problem/A-B', 'https://evil@vjudge.net/problem/A-B', 'https://vjudge.net:99/problem/A-B', 'https://vjudge.net/contest/123', 'https://vjudge.net/contest/123#problem/A/other']) assert.equal(isVjudge(url), false);
    assert.equal(problemUrl('https://vjudge.net.cn/contest/123?x=y#problem/AA'), 'https://vjudge.net.cn/contest/123#problem/AA');
    const job = { url: 'https://vjudge.net/problem/CodeForces-1A', code: '秘密中文\nint main(){}' };
    let valid = true;
    const notices = [];
    const session = await createSession(job, () => { if (!valid) throw Error('changed'); }, { onFallback: language => notices.push(language) });
    try {
        const request = async (route, body = {}, headers = {}) => fetch(`http://127.0.0.1:${session.port}/${route}`, { method: 'POST', headers: { Authorization: `Bearer ${session.token}`, ...headers }, body: JSON.stringify({ url: job.url, ...body }) });
        assert.equal((await request('claim', {}, { Authorization: 'Bearer wrong' })).status, 403);
        assert.equal((await request('claim', {}, { Origin: 'https://evil.test' })).status, 403);
        assert.equal((await request('claim', { url: 'https://vjudge.net/problem/CodeForces-2A' })).status, 409);
        assert.equal((await request('permit')).status, 409);
        assert.deepEqual(await (await request('claim')).json(), job);
        assert.equal((await request('claim')).status, 409);
        valid = false; assert.equal((await request('permit', { languageFallback: true, language: 'C++14 (gcc 8.3)' })).status, 400);
        assert.deepEqual(notices, []);
        valid = true; assert.equal((await request('permit', { languageFallback: true })).status, 400);
        assert.equal((await request('permit', { languageFallback: true, language: 'C++14 (gcc 8.3)' })).status, 200);
        assert.equal((await request('permit', { languageFallback: true, language: 'C++14 (gcc 8.3)' })).status, 409);
        assert.deepEqual(notices, ['C++14 (gcc 8.3)']);
        assert.equal((await request('report', { ok: true, runId: '1234' })).status, 200);
        assert.equal((await session.done).runId, '1234');
    } finally { session.cancel(); }
    const timeout = await createSession(job, () => {}, { timeout: 20 });
    assert.match((await timeout.done).message, /未连接/);
    const probe = await createSession({ ...job, mode: 'check' }, () => {});
    try {
        const post = route => fetch(`http://127.0.0.1:${probe.port}/${route}`, { method: 'POST', headers: { Authorization: `Bearer ${probe.token}` }, body: JSON.stringify({ url: job.url, prepared: true }) });
        assert.equal((await post('claim')).status, 200);
        assert.equal((await post('permit')).status, 409); // Even an older worker cannot submit a probe.
        await post('report'); const result = await probe.done; assert.equal(result.prepared, true); assert.equal(result.ok, false);
    } finally { probe.cancel(); }
}

// DOM contract from VJudge's current submit form; the native request is a mock.
function pageFixture(url = 'https://vjudge.net/problem/CodeForces-1A', showDelay = 0) {
    let code = '', clicks = 0, opened = false, displayed = false, triggerClicks = 0, editedWhileHidden = false;
    const lang = { options: [{ value: '', textContent: '请选择' }, { value: '54', textContent: 'GNU G++17 7.3.0' }, { value: '89', textContent: 'GNU G++20 13.2 (64 bit)' }, { value: '91', textContent: 'GNU G++23 14.2 (64 bit)' }], value: '', dispatchEvent() {} };
    const radio = () => ({ checked: false, click() { this.checked = true; } });
    const priv = radio(), normal = radio(), personal = radio();
    let sharedDisabled = false;
    const defaultLabel = { classList: { contains: () => sharedDisabled } }, personalLabel = { classList: { contains: () => false } };
    const account = { disabled: false, value: '7', selectedOptions: [{ disabled: false }] };
    // CodeMirror 的文本模型会将 Windows/旧 Mac 换行规范成 LF
    const cm = { setValue: value => { if (!displayed) editedWhileHidden = true; code = value.replace(/\r\n?/g, '\n'); }, getValue: () => code, save() {} };
    const nodes = { '#submit-language': lang, '#open0': priv, '#submitter-type0': normal, '#submitter-type1': personal, 'label[for="submitter-type0"]': defaultLabel, 'label[for="submitter-type1"]': personalLabel, '#submit-remote-account': account, '.CodeMirror': { CodeMirror: cm } };
    nodes['#contest-num'] = { textContent: 'A - MEX Partition' }; nodes['.problem-origin'] = { textContent: 'CodeForces - 1A' };
    const form = { querySelector: selector => nodes[selector], closest: () => modal };
    const visible = { getClientRects: () => [1] };
    // Bootstrap initializes the form during show.bs.modal, then displays it after the backdrop transition.
    const trigger = { ...visible, closest: () => null, click() { triggerClicks++; opened = true; if (showDelay) setTimeout(() => { displayed = true; }, showDelay); else displayed = true; } };
    let response = { runId: 4321 }, status = 200;
    class XHR {
        open(method, route) { this.route = route; }
        addEventListener(_, fn) { this.callback = fn; }
        send() { this.status = status; this.responseText = JSON.stringify(response); this.callback(); }
    }
    const button = { disabled: false, click() { clicks++; const req = new XHR(); req.open('POST', url.includes('/contest/') ? '/contest/submit/123/A' : '/problem/submit/CodeForces-1A'); req.send(); } };
    const modal = { getClientRects: () => displayed ? [1] : [], querySelector: () => button };
    const document = { querySelectorAll: () => [trigger], querySelector: selector => selector === '#submit-form' && opened ? form : null };
    const context = { URL, Event, setTimeout, clearTimeout, Date, location: new URL(url), document, XMLHttpRequest: XHR };
    const run = vm.runInNewContext('(' + zoiVjudgePage.toString() + ')', context);
    const job = { url, code: '#pragma GCC optimize("O2")\nint main(){}' };
    return { run: stage => run(job, stage), nodes, lang, cm, job, context, XHR, priv, normal, personal, account, close: () => { displayed = false; }, triggerClicks: () => triggerClicks, editedWhileHidden: () => editedWhileHidden, disableShared: () => { sharedDisabled = true; }, clicks: () => clicks, response: (value, code = 200) => { response = value; status = code; } };
}

async function pageChecks() {
    for (const mode of ['submit', 'check']) {
        const delayed = pageFixture(undefined, 180); delayed.job.mode = mode;
        assert.equal((await delayed.run('prepare')).prepared, true);
        assert.equal(delayed.editedWhileHidden(), false, 'must wait for the modal before filling the pre-created form');
        assert.equal((await delayed.run('verify')).verified, true);
        assert.equal(delayed.triggerClicks(), 1); assert.equal(delayed.clicks(), 0);
        if (mode === 'submit') assert.equal((await delayed.run('submit')).ok, true);
        else { delayed.close(); assert.match((await delayed.run('verify')).error, /表单已关闭/); assert.equal(delayed.clicks(), 0); }
    }
    const existing = pageFixture(); await existing.run('prepare'); await existing.run('prepare');
    assert.equal(existing.triggerClicks(), 1, 'reuse an existing form instead of opening a second modal');
    existing.close(); assert.match((await existing.run('submit')).error, /表单已关闭/); assert.equal(existing.clicks(), 0);
    let now = 0;
    existing.context.Date = { now: () => now };
    existing.context.setTimeout = callback => { now += 10000; return setTimeout(callback, 0); };
    assert.match((await existing.run('prepare')).error, /表单未能显示/); assert.equal(existing.triggerClicks(), 1);
    for (const url of ['https://vjudge.net/problem/CodeForces-1A', 'https://vjudge.net/contest/123#problem/A']) {
        const f = pageFixture(url), nativeOpen = f.XHR.prototype.open;
        assert.equal((await f.run('prepare')).prepared, true); assert.equal(f.clicks(), 0);
        assert.equal(f.lang.value, '89'); assert.equal(f.cm.getValue(), f.job.code); assert.ok(f.priv.checked && f.normal.checked);
        const result = await f.run('submit'); assert.equal(result.ok, true); assert.equal(result.runId, '4321'); assert.equal(f.clicks(), 1); assert.equal(f.XHR.prototype.open, nativeOpen);
    }
    const languageCases = [
        { labels: ['GNU G++23 14.2 (64 bit)', 'GNU G++20 13.2', 'GNU G++17 7.3.0'], selected: 'GNU G++20 13.2', fallback: false },
        { labels: ['GNU G++20 13.2', 'GNU G++20 13.2 (64 bit)'], selected: 'GNU G++20 13.2 (64 bit)', fallback: false },
        { labels: ['GNU G++17 7.3.0', 'GNU G++23 14.2 (64 bit)'], selected: 'GNU G++23 14.2 (64 bit)', fallback: true },
        { labels: ['C++ (g++ 4.3.2)', 'C++ (gcc 8.3)', 'C++14 (clang 8.0)', 'C++14 (gcc 8.3)'], selected: 'C++14 (gcc 8.3)', fallback: true },
        { labels: ['GNU C++98', 'GNU C++03', 'GNU C++11'], selected: 'GNU C++11', fallback: true },
        { labels: ['GNU C++2a', 'GNU C++2b'], selected: 'GNU C++2a', fallback: false },
        { labels: ['GNU C++17', 'GNU C++26'], selected: 'GNU C++26', fallback: true },
        { labels: ['C++ (gcc 8.3)', 'C (gcc 20.1)'], selected: 'C++ (gcc 8.3)', fallback: true },
        { labels: ['C++20 (clang 17)', 'GNU C++17'], selected: 'GNU C++17', fallback: true },
    ];
    for (const test of languageCases) {
        const f = pageFixture();
        f.lang.options = [{ value: '', textContent: '请选择' }, ...test.labels.map((textContent, index) => ({ value: String(index + 1), textContent }))];
        const prepared = await f.run('prepare');
        assert.equal(prepared.language, test.selected); assert.equal(prepared.languageFallback, test.fallback);
        assert.equal(f.lang.options.find(o => o.value === f.lang.value).textContent, test.selected);
        assert.equal(f.clicks(), 0); assert.equal((await f.run('submit')).ok, true); assert.equal(f.clicks(), 1);
    }
    const disabled = pageFixture(); disabled.lang.options[2].disabled = true; disabled.lang.options[3].parentElement = { disabled: true };
    assert.equal((await disabled.run('prepare')).language, 'GNU G++17 7.3.0');
    const unsupported = pageFixture(); unsupported.lang.options = [{ value: '', textContent: '请选择' }, { value: '1', textContent: 'C++20 (clang 17)' }, { value: '2', textContent: 'C (gcc 8.3)' }];
    assert.match((await unsupported.run('prepare')).error, /没有提供可用的 GNU C\+\+/); assert.equal(unsupported.clicks(), 0);
    for (const newline of ['\r\n', '\r']) {
        const f = pageFixture(); f.job.code = '#pragma GCC optimize("O2")' + newline + 'int main(){' + newline + 'return 0;' + newline + '}';
        assert.equal((await f.run('prepare')).prepared, true); assert.equal((await f.run('submit')).ok, true); assert.equal(f.clicks(), 1);
    }
    const edited = pageFixture(); await edited.run('prepare'); edited.cm.setValue('changed'); assert.match((await edited.run('submit')).error, /代码内容发生变化/); assert.equal(edited.clicks(), 0);
    const moved = pageFixture(); await moved.run('prepare'); moved.context.location.hash = '#problem/B'; assert.match((await moved.run('submit')).error, /题目发生变化/);
    const failure = pageFixture(); await failure.run('prepare'); failure.response({ challenge: true }); assert.match((await failure.run('submit')).error, /未接收/); assert.equal(failure.clicks(), 1);
    const network = pageFixture(); await network.run('prepare'); network.response({ runId: 4321 }, 500); assert.match((await network.run('submit')).error, /未接收/);
    const probe = pageFixture(); probe.job.mode = 'check'; await probe.run('prepare'); assert.equal((await probe.run('verify')).verified, true); assert.match((await probe.run('submit')).error, /不能发送提交/); assert.equal(probe.clicks(), 0);
    probe.cm.setValue('changed'); assert.match((await probe.run('verify')).error, /代码内容发生变化/); assert.equal(probe.clicks(), 0);
    const changedLanguage = pageFixture(); await changedLanguage.run('prepare'); changedLanguage.lang.value = '54'; assert.match((await changedLanguage.run('submit')).error, /编译器发生变化/);
    const changedPrivacy = pageFixture(); await changedPrivacy.run('prepare'); changedPrivacy.priv.checked = false; assert.match((await changedPrivacy.run('submit')).error, /公开选项发生变化/);
    const changedAccount = pageFixture(); await changedAccount.run('prepare'); changedAccount.normal.checked = false; assert.match((await changedAccount.run('submit')).error, /提交账号发生变化/);
    const own = pageFixture(); own.disableShared(); assert.equal((await own.run('prepare')).prepared, true); assert.ok(own.personal.checked); assert.equal((await own.run('submit')).ok, true);
    const unbound = pageFixture(); unbound.disableShared(); unbound.account.disabled = true; unbound.job.mode = 'check'; assert.match((await unbound.run('prepare')).warning, /需要绑定/); assert.equal(unbound.clicks(), 0);
    const wrongProblem = pageFixture('https://vjudge.net/contest/123#problem/A'); wrongProblem.nodes['#contest-num'].textContent = 'B - Other'; assert.match((await wrongProblem.run('prepare')).error, /题号.*不一致/); assert.equal(wrongProblem.clicks(), 0);
}

async function workerChecks() {
    let listener;
    for (const mode of ['submit', 'check']) {
    for (const languageFallback of [false, true]) {
    const language = languageFallback ? 'C++14 (gcc 8.3)' : 'GNU G++20 13.2 (64 bit)';
    const stages = [], session = await createSession({ url: 'https://vjudge.net/contest/123#problem/A', code: 'int main(){}', mode }, () => stages.push('validate'), { onFallback: actual => { assert.equal(actual, language); stages.push('notify'); } });
    try {
        const chrome = { runtime: { onMessage: { addListener: fn => { listener = fn; } } }, scripting: { executeScript: async ({ args: [job, stage], world }) => { assert.equal(world, 'MAIN'); assert.equal(job.code, 'int main(){}'); stages.push(stage); return [{ result: stage === 'prepare' ? { prepared: true, language, languageFallback } : { ok: true, runId: '5678' } }]; } } };
        vm.runInNewContext(fs.readFileSync(path.join(extension, 'background.js'), 'utf8'), { chrome, importScripts() {}, zoiVjudgePage, URL, fetch, AbortSignal });
        const result = await new Promise(resolve => listener({ type: 'zoi-vjudge-submit', port: session.port, token: session.token, url: 'https://vjudge.net/contest/123#problem/A' }, { tab: { id: 1 }, frameId: 0, url: 'https://vjudge.net/contest/123#problem/A' }, resolve));
        if (mode === 'submit') { assert.equal(result.runId, '5678'); assert.deepEqual(stages, ['validate', 'prepare', 'validate', ...(languageFallback ? ['notify'] : []), 'submit']); assert.equal((await session.done).ok, true); }
        else { assert.equal(result.prepared, true); assert.ok(result.message.includes(language)); assert.equal(result.message.includes('自动回退'), languageFallback); assert.deepEqual(stages, ['validate', 'prepare', 'verify']); assert.equal((await session.done).prepared, true); }
    } finally { session.cancel(); }
    }
    }
    let submitted;
    const target = 'https://vjudge.net/contest/123#problem/A';
    const location = new URL(target + '&zoi-submit=23456.' + 'a'.repeat(64));
    vm.runInNewContext(fs.readFileSync(path.join(extension, 'handoff.js'), 'utf8'), { location, history: { state: {}, replaceState(_, __, value) { assert.equal(value, target); } }, chrome: { runtime: { sendMessage: async value => { submitted = value; return {}; } } }, alert: assert.fail });
    assert.equal(submitted.url, target); assert.equal(submitted.port, 23456);
    for (const early of [false, true]) {
        const notices = [];
        let onReady;
        const body = { append: node => notices.push(node) };
        const document = { body: early ? null : body, createElement: () => ({ style: {}, setAttribute(name, value) { this[name] = value; }, append(node) { this.close = node; }, remove() { this.removed = true; } }), addEventListener: (event, handler, options) => { assert.equal(event, 'DOMContentLoaded'); assert.ok(options.once); onReady = handler; } };
        vm.runInNewContext(fs.readFileSync(path.join(extension, 'handoff.js'), 'utf8'), { document, location, history: { state: {}, replaceState() {} }, chrome: { runtime: { sendMessage: async () => { if (early) throw Error('disconnected'); return { error: '<code changed>' }; } } }, alert: assert.fail });
        await new Promise(resolve => setImmediate(resolve));
        if (early) { assert.equal(notices.length, 0); document.body = body; onReady(); }
        assert.equal(notices.length, 1); assert.equal(notices[0].role, 'alert');
        assert.match(notices[0].textContent, early ? /扩展连接中断/ : /<code changed>/);
        notices[0].close.onclick(); assert.equal(notices[0].removed, true);
    }
    const manifest = JSON.parse(fs.readFileSync(path.join(extension, 'manifest.json')));
    for (const file of [manifest.background.service_worker, ...manifest.content_scripts.flatMap(s => s.js), 'page.js']) new vm.Script(fs.readFileSync(path.join(extension, file), 'utf8'));
    assert.deepEqual(manifest.permissions, ['scripting']);
}

async function vscodeNoticeChecks() {
    for (const languageFallback of [false, true]) {
        let opened;
        const launched = new Promise(resolve => { opened = resolve; });
        const warnings = [], infos = [], errors = [];
        const context = { module: { exports: {} }, process: { env: { PROGRAMFILES: path.join(__dirname, 'fake-program-files') } }, URL, setTimeout, clearTimeout, Buffer, __dirname,
            require: id => id === 'node:child_process' ? { spawn: (_, args) => {
                const child = new (require('node:events').EventEmitter)(); child.unref = () => {};
                queueMicrotask(() => { child.emit('spawn'); opened(args[1]); }); return child;
            } } : id === 'node:fs' ? { ...fs, existsSync: () => true } : require(id),
        };
        vm.runInNewContext(fs.readFileSync(path.join(__dirname, 'vjudge_bridge.cjs'), 'utf8'), context);
        const document = { version: 1, isClosed: false, getText: () => 'int main(){}' };
        const vscode = { workspace: { isTrusted: true, openTextDocument: async () => document }, Uri: { file: value => value }, window: {
            showWarningMessage: message => { warnings.push(message); return new Promise(() => {}); },
            showInformationMessage: message => infos.push(message), showErrorMessage: message => errors.push(message),
        } };
        const pending = context.module.exports.submit(vscode, { url: 'https://vjudge.net/problem/SPOJ-ABACABA', srcPath: 'test.cpp' }, async () => document.getText());
        const url = new URL(await launched), [, port, token] = url.hash.match(/^#zoi-submit=(\d+)\.([a-f0-9]+)$/); url.hash = '';
        const post = (route, body = {}) => fetch(`http://127.0.0.1:${port}/${route}`, { method: 'POST', headers: { Authorization: `Bearer ${token}` }, body: JSON.stringify({ url: url.href, ...body }) });
        try {
            const job = await (await post('claim')).json(); assert.ok(job.code.startsWith('#pragma GCC optimize("O2")\n'));
            assert.equal((await post('permit', { languageFallback, language: languageFallback ? 'C++14 (gcc 8.3)' : 'GNU G++20' })).status, 200);
            assert.equal(warnings.length, languageFallback ? 1 : 0);
            if (languageFallback) assert.match(warnings[0], /自动改用 C\+\+14 \(gcc 8\.3\).*O2/);
            await post('report', { ok: true, runId: '6789' }); await pending;
            assert.deepEqual(errors, []); assert.ok(infos.some(message => message.includes('#6789')));
        } finally { await post('report', { ok: false }).catch(() => {}); }
    }
}

(async () => { await protocolChecks(); await pageChecks(); await workerChecks(); await vscodeNoticeChecks(); console.log('PASS: VJudge protocol, delayed modal display + closed-form guards, GNU C++ fallback + VS Code warning, CRLF/CR editor normalization, probe preflight, native form results and non-blocking Chrome notices'); })().catch(e => { console.error(e); process.exitCode = 1; });
