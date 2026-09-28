'use strict';
const assert = require('node:assert/strict');
const vm = require('node:vm');
const fs = require('node:fs');
const path = require('node:path');
const { problemUrl, isAtcoder, submissionUrl, submit } = require('./atcoder_bridge.cjs');
const { createSession } = require('./vjudge_bridge.cjs');
const { zoiAtcoderPage } = require('./vjudge-extension/atcoder.js');
const extension = path.join(__dirname, 'vjudge-extension');
const taskUrl = 'https://atcoder.jp/contests/abc477/tasks/abc477_a';
const url = submissionUrl(taskUrl), listUrl = 'https://atcoder.jp/contests/abc477/submissions/me';

function fixture({ plain = false } = {}) {
    const job = { url, code: '#pragma GCC optimize("O2")\n// 中文\nint main() {}\n' };
    let code = '', capture;
    const calls = [], options = labels => labels.map((textContent, i) => ({ value: String(i + 1), textContent }));
    const task = { value: 'abc477_b', options: [{ value: 'abc477_a' }, { value: 'abc477_b' }] };
    const lang = { value: '', options: options(['C++23 (Clang 21.1.0)', 'C++ IOI-Style(GNU++20) (GCC 14.2.0)', 'C++23 (GCC 15.2.0)']) };
    const editor = { setValue: text => { code = text; }, getValue: () => code };
    const textarea = { value: '', getClientRects: () => plain ? [1] : [], dispatchEvent() {} };
    const captcha = { value: 'fixture-only' };
    const state = { csrf: 'fixture-only', beforeIds: ['10'], afterIds: ['11', '10'], code: job.code, detailTask: true, status: 200, responseUrl: listUrl, cancelled: false, onBefore: () => {}, onClick: () => {} };
    const button = { disabled: false, click() {
        state.onClick();
        if (!plain) textarea.value = editor.getValue(); // AtCoder's own submit handler.
        capture?.({ defaultPrevented: state.cancelled, preventDefault() {} });
    } };
    const nodes = { '[name="data.TaskScreenName"]': task, 'select[name="data.LanguageId"]': lang, 'textarea[name="sourceCode"]': textarea, '#submit': button, '[name="cf-turnstile-response"]': captcha };
    const form = { action: 'https://atcoder.jp/contests/abc477/submit', method: 'post', isConnected: true, querySelector: selector => nodes[selector], addEventListener: (_, fn) => { capture = fn; }, removeEventListener: (_, fn) => { if (capture === fn) capture = null; } };
    const listDoc = values => ({ querySelectorAll: () => values.map(id => ({ getAttribute: () => '/contests/abc477/submissions/' + id })) });
    const documents = { before: () => listDoc(state.beforeIds), after: () => listDoc(state.afterIds), detail: () => ({ querySelector: selector => selector === '#submission-code' ? { textContent: state.code } : state.detailTask ? {} : null }) };
    const context = { URL, URLSearchParams, Event, AbortSignal, setTimeout, clearTimeout, Date,
        location: new URL(url), window: { jQuery: e => ({ data: () => editor, trigger() {} }) },
        document: { querySelector: () => form },
        DOMParser: class { parseFromString(text) { return documents[text](); } },
        FormData: class extends Map { constructor() { super([['data.TaskScreenName', task.value], ['data.LanguageId', lang.value], ['sourceCode', textarea.value], ['csrf_token', state.csrf], ['cf-turnstile-response', captcha.value]]); } },
        fetch: async (address, init) => {
            const method = init.method || 'GET'; calls.push({ address, method, body: init.body });
            if (method === 'POST') return { ok: state.status === 200, url: state.responseUrl, text: async () => 'after' };
            if (address === listUrl) { state.onBefore(); return { ok: true, url: address, text: async () => 'before' }; }
            assert.equal(address, 'https://atcoder.jp/contests/abc477/submissions/11');
            return { ok: true, url: address, text: async () => 'detail' };
        },
    };
    const run = vm.runInNewContext('(' + zoiAtcoderPage.toString() + ')', context);
    return { job, state, form, task, lang, textarea, captcha, button, editor, context, calls, options, run: stage => run(job, stage), posts: () => calls.filter(c => c.method === 'POST').length };
}

async function pages() {
    for (const plain of [false, true]) {
        const f = fixture({ plain }); f.job.code = f.job.code.replace(/\n/g, '\r\n');
        const p = await f.run('prepare'); assert.equal(p.language, 'C++23 (GCC 15.2.0)'); assert.equal(p.languageFallback, true);
        assert.equal((await f.run('verify')).verified, true); assert.equal(f.calls.length, 0);
        const result = await f.run('submit'); assert.equal(result.ok, true); assert.equal(result.runId, '11'); assert.equal(f.posts(), 1);
        assert.deepEqual(f.calls.map(c => c.method), ['GET', 'POST', 'GET']);
        assert.equal(f.calls[1].body.get('sourceCode'), f.state.code);
    }
    for (const [labels, selected, fallback] of [
        [['C++23 (GCC 15)', 'C++20 (GCC 12)', 'C++17 (GCC 9)'], 'C++20 (GCC 12)', false],
        [['C++14 (GCC 5)', 'C++17 (GCC 9)'], 'C++17 (GCC 9)', true],
        [['C++98 (GCC 4)', 'C++11 (GCC 5.3.0)'], 'C++11 (GCC 5.3.0)', true],
        [['C++2a (GCC 12)', 'C++2b (GCC 14)'], 'C++2a (GCC 12)', false],
        [['C++1y (GCC 5)', 'C++1z (GCC 9)'], 'C++1z (GCC 9)', true],
    ]) {
        const f = fixture(); f.lang.options = f.options(labels); const result = await f.run('prepare');
        assert.equal(result.language, selected); assert.equal(result.languageFallback, fallback);
    }
    const disabled = fixture(); disabled.lang.options.push({ value: '4', textContent: 'C++20 (GCC 12)', parentElement: { disabled: true } });
    assert.equal((await disabled.run('prepare')).language, 'C++23 (GCC 15.2.0)');
    for (const label of ['C++ IOI-Style(GNU++20) (GCC 14.2.0)', 'C++23 (Clang 21)', 'C (GCC 15)', 'Python 3', 'C++98 (GCC 4)', 'C++03 (GCC 4)', 'C++ 11.2.0 (GCC)']) {
        const f = fixture(); f.lang.options = f.options([label]); assert.match((await f.run('prepare')).error, /没有可用/); assert.equal(f.posts(), 0);
    }
    for (const change of [f => { f.task.value = 'abc477_b'; }, f => { f.lang.value = '1'; }, f => f.editor.setValue('changed'), f => { f.form.action = 'https://evil.test/submit'; }, f => { f.form.isConnected = false; }, f => { f.context.location.search = '?taskScreenName=abc477_b'; }, f => { f.captcha.value = ''; }, f => { f.button.disabled = true; }, f => { f.state.csrf = ''; }, f => { f.state.cancelled = true; }, f => { f.state.onBefore = () => f.editor.setValue('changed'); }, f => { f.state.onClick = () => { f.lang.value = '1'; }; }]) {
        const f = fixture(); await f.run('prepare'); change(f);
        assert.ok((await f.run('submit')).error); assert.equal(f.posts(), 0);
    }
    for (const change of [f => { f.state.status = 403; }, f => { f.state.responseUrl = 'https://atcoder.jp/login'; }, f => { f.state.afterIds = ['10']; }, f => { f.state.afterIds = ['11', '12']; }, f => { f.state.code = 'other code'; }, f => { f.state.detailTask = false; }]) {
        const f = fixture(); await f.run('prepare'); change(f); assert.ok((await f.run('submit')).error); assert.equal(f.posts(), 1);
    }
    const probe = fixture(); probe.job.mode = 'check'; await probe.run('prepare');
    assert.equal((await probe.run('verify')).verified, true); assert.match((await probe.run('submit')).error, /不能发送/); assert.equal(probe.calls.length, 0);
    const disconnected = fixture(); await disconnected.run('prepare'); const normalFetch = disconnected.context.fetch;
    disconnected.context.fetch = async (address, init) => { if (init.method === 'POST') throw Error('network lost'); return normalFetch(address, init); };
    assert.match((await disconnected.run('submit')).error, /请求已发出.*不会自动重试/);
}

async function worker() {
    for (const mode of ['check', 'submit']) {
        const stages = [], session = await createSession({ url, mode, code: 'int main(){}' }, () => {}, { name: 'AtCoder', onFallback: () => stages.push('fallback') });
        try {
            let listener;
            const chrome = { runtime: { onMessage: { addListener: fn => { listener = fn; } } }, tabs: { update: async (_, arg) => { assert.equal(arg.url, 'https://atcoder.jp/contests/abc477/submissions/11'); stages.push('navigate'); } }, scripting: { executeScript: async ({ func, args: [job, stage] }) => {
                assert.equal(func, zoiAtcoderPage); assert.equal(job.url, url); stages.push(stage);
                return [{ result: stage === 'prepare' ? { prepared: true, language: 'C++23 (GCC 15.2.0)', languageFallback: true } : stage === 'verify' ? { verified: true } : { ok: true, runId: '11', resultUrl: 'https://atcoder.jp/contests/abc477/submissions/11' } }];
            } } };
            vm.runInNewContext(fs.readFileSync(path.join(extension, 'background.js'), 'utf8'), { chrome, importScripts() {}, zoiAtcoderPage, URL, fetch, AbortSignal });
            const result = await new Promise(resolve => listener({ type: 'zoi-atcoder-submit', port: session.port, token: session.token, url }, { tab: { id: 1 }, frameId: 0, url }, resolve));
            assert.equal(result.error, undefined);
            assert.deepEqual(stages, mode === 'check' ? ['prepare', 'verify'] : ['prepare', 'fallback', 'submit', 'navigate']);
            assert.equal((await session.done).ok, mode === 'submit');
            assert.ok((await new Promise(resolve => listener({ type: 'zoi-atcoder-submit', port: session.port, token: session.token, url }, { tab: { id: 1 }, frameId: 0, url: url.replace('abc477_a', 'abc477_b') }, resolve))).error);
        } finally { session.cancel(); }
    }
    let message;
    const location = new URL(url + '#zoi-submit=23456.' + 'a'.repeat(64));
    vm.runInNewContext(fs.readFileSync(path.join(extension, 'handoff.js'), 'utf8'), { location, history: { state: {}, replaceState(_, __, value) { assert.equal(value, url); } }, chrome: { runtime: { sendMessage: async data => { message = data; return {}; } } } });
    assert.equal(message.type, 'zoi-atcoder-submit'); assert.equal(message.url, url);
    const manifest = JSON.parse(fs.readFileSync(path.join(extension, 'manifest.json')));
    assert.ok(manifest.host_permissions.includes('https://atcoder.jp/*'));
    assert.ok(manifest.content_scripts[0].matches.includes('https://atcoder.jp/*'));
}

async function sizeChecks() {
    const doc = { version: 1, getText: () => 'source' };
    const vscode = { workspace: { isTrusted: true, openTextDocument: async () => doc }, Uri: { file: p => p } };
    const problem = { url: taskUrl, srcPath: 'size-check.cpp' };
    for (const code of ['', 'x'.repeat(512 * 1024), '汉'.repeat(180000)]) await assert.rejects(submit(vscode, problem, async () => code), /为空或.*512 KiB/);
    await assert.rejects(submit(vscode, { ...problem, srcPath: 'a.py' }, async () => 'pass'), /只支持 .cpp/);
}

(async () => {
    assert.equal(problemUrl(taskUrl + '/?lang=en#x'), taskUrl);
    for (const invalid of ['http://atcoder.jp/contests/a/tasks/b', 'https://evil@atcoder.jp/contests/a/tasks/b', 'https://atcoder.jp.evil.test/contests/a/tasks/b', 'https://atcoder.jp:99/contests/a/tasks/b', 'https://atcoder.jp/contests/a', 'https://atcoder.jp/contests/a/submit']) assert.equal(isAtcoder(invalid), false);
    await pages(); await worker(); await sizeChecks();
    console.log('PASS: AtCoder URL routing, GCC language selection, Ace/plain editors, preflight guards, native form serialization, single POST, exact submission/code confirmation, worker and check-only mode');
})().catch(error => { console.error(error); process.exitCode = 1; });
