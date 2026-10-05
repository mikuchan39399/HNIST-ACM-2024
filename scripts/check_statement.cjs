'use strict';
const assert = require('node:assert/strict');
const fs = require('node:fs');
const path = require('node:path');
const http = require('node:http');
const vm = require('node:vm');
const { JSDOM } = require('./statement-extension/node_modules/jsdom');
const purify = require('./statement-extension/vendor/dompurify/dist/purify.min.js');
const markdown = require('./statement-extension/vendor/markdown-it/dist/browser/markdown-it.umd.min.js')({ html: true });
const core = require('./statement-extension/core.cjs');
const render = require('./statement-extension/renderer.js');
const links = require('./vjudge-extension/youknowwho.js');
const numbers = require('./vjudge-extension/youknowwho-data.js');
const { zoiStatementPage } = require('./vjudge-extension/statement.js');

async function checkHost(work, html, options = {}) {
    const commands = {}, panels = [], sent = [], opened = [], errors = [];
    let handler;
    const uri = value => { const u = new URL(value); return { scheme: u.protocol.slice(0, -1), path: u.pathname, query: u.search.slice(1), toString: () => value }; };
    const fileUri = file => ({ scheme: 'file', fsPath: file, toString: () => 'file:///' + file.replace(/\\/g, '/') });
    const vscode = {
        Uri: { parse: uri, file: fileUri, joinPath: (base, name) => fileUri(path.join(base.fsPath, name)) }, ViewColumn: { One: 1, Two: 2, Beside: -2 },
        workspace: { isTrusted: true, workspaceFolders: [{ uri: fileUri(work) }] },
        commands: { registerCommand: (id, fn) => { commands[id] = fn; return { dispose() {} }; }, executeCommand: id => commands[id]() },
        extensions: { getExtension: () => ({ activate: async () => {} }) },
        window: {
            registerUriHandler: x => { handler = x; return { dispose() {} }; },
            showTextDocument: async u => { opened.push(u.fsPath); vscode.window.activeTextEditor = { document: { uri: u } }; },
            showErrorMessage: s => errors.push(s), showWarningMessage: s => errors.push(s),
            createWebviewPanel: () => {
                const p = { messages: [], reveal() {}, dispose() { this.closed?.(); }, onDidDispose(fn) { this.closed = fn; } };
                p.webview = { cspSource: 'https://local.invalid', asWebviewUri: u => u, postMessage: m => p.messages.push(m), onDidReceiveMessage: fn => { p.receive = fn; } };
                panels.push(p); return p;
            },
        },
    };
    const modifiedCore = { ...core, ...options.core, download: options.core?.download || (async () => html), sendToCph: async payload => {
        sent.push(payload); const file = path.join(work, 'imported.cpp');
        fs.writeFileSync(file, 'CPH template');
        fs.writeFileSync(path.join(work, '.cph/.imported.cpp_test.prob'), JSON.stringify({ ...payload, srcPath: file }));
    } };
    const box = { module: { exports: {} }, require: name => name === 'vscode' ? vscode : name === './core.cjs' ? modifiedCore : name === './trash.cjs' ? require('./statement-extension/trash.cjs') : require(name), AbortController, URL, URLSearchParams, setTimeout };
    vm.runInNewContext(fs.readFileSync(path.join(__dirname, 'statement-extension/extension.cjs'), 'utf8'), box);
    const context = { subscriptions: [], extensionUri: fileUri(path.join(__dirname, 'statement-extension')), globalStorageUri: fileUri(path.join(work, 'storage')) };
    box.module.exports.activate(context);
    const url = options.url || 'https://codeforces.com/contest/9/problem/A';
    const link = uri('vscode://zoi-local.zoi-statement/import?' + new URLSearchParams({ url, title: 'Test' }));
    await handler.handleUri(link);
    const panel = panels[0]; await panel.receive({ type: 'ready' });
    const page = panel.messages.find(m => m.type === 'page'); assert.ok(page);
    if (options.site) assert.equal(page.site, options.site);
    if (options.site === 'gym-pdf') assert.equal(page.gymPdfs.length, 1);
    const message = { type: 'parsed', generation: page.generation, tests: [{ input: '2\n', output: '3\n' }] };
    await panel.receive({ ...message, generation: page.generation - 1 }); assert.equal(sent.length, 0);
    await panel.receive({ ...message, pendingPdf: true }); assert.equal(sent.length, 0);
    await Promise.all([panel.receive(message), panel.receive(message)]);
    assert.equal(sent.length, 1); assert.deepEqual(sent[0].tests, message.tests);
    assert.equal(sent[0].url, url);
    if (options.site === 'vj') {
        await panel.receive({ type: 'gymPdf' });
        const last = panel.messages.filter(m => m.type === 'page').at(-1);
        assert.equal(last.site, 'gym-pdf');
        await panel.receive(message); assert.equal(sent.length, 1);
    }
    const file = path.join(work, 'imported.cpp'); assert.ok(opened.includes(file));
    fs.writeFileSync(file, 'user solution');
    await handler.handleUri(link); assert.equal(sent.length, 1); assert.equal(fs.readFileSync(file, 'utf8'), 'user solution');
    assert.deepEqual(errors, []);
    for (const entry of context.subscriptions) entry.dispose?.();
}

async function main() {
    const work = fs.mkdtempSync(path.join(fs.mkdirSync(path.join(__dirname, '../.zoi-checks'), { recursive: true }) || path.join(__dirname, '../.zoi-checks'), 'statement-check-'));
    try {
        const target = path.join(work, '题目.cpp'); fs.writeFileSync(target, 'user code'); fs.mkdirSync(path.join(work, '.cph'));
        const p = { name: 'Watermelon', srcPath: target, url: 'https://codeforces.com/contest/4/problem/A', tests: [{ input: '8\n', output: 'YES\n' }] };
        fs.writeFileSync(path.join(work, '.cph/.题目.cpp_a.prob'), JSON.stringify(p));
        assert.equal(core.readProblem(target).site, 'cf');
        assert.equal(core.findImported(work, p.url), target);
        assert.equal(core.findImported(work, 'https://m1.codeforces.com/problemset/problem/4/A'), target);
        assert.equal(core.findImported(work, 'https://codeforces.com/contest/4/problem/B'), null);
        assert.throws(() => core.readProblem(path.join(work, 'unknown.cpp')), /CPH/);
        for (const url of ['file:///C:/secret', 'https://codeforces.com.evil.test/contest/4/problem/A', 'https://atcoder.jp/login', 'https://vjudge.net/contest/12', 'https://a:b@vjudge.net/problem/UVA-455', 'https://vjudge.net:9999/problem/UVA-455']) assert.throws(() => core.problemUrl(url), url);
        assert.equal(core.problemUrl('https://vjudge.net/contest/12#problem/B').site, 'vj');
        assert.equal(core.problemUrl('https://atcoder.jp/contests/abc001/tasks/abc001_1').url, 'https://atcoder.jp/contests/abc001/tasks/abc001_1?lang=en');
        assert.throws(() => core.descriptionUrl('https://vjudge.net/problem/description/1', 'https://vjudge.net.cn/problem/UVA-455'));
        assert.throws(() => core.pdfUrl('https://localhost/a.pdf'));
        const dom = new JSDOM('<!doctype html><html><body></body></html>', { url: 'https://test.invalid/' });
        const doc = dom.window.document, cleaner = purify(dom.window);
        const cf = '<div class="problem-statement"><p>A <script type="math/tex">x^2</script></p><div class="sample-test"><div class="input"><pre><div class="test-example-line">2</div><div class="test-example-line">3 4</div></pre></div><div class="output"><pre>7<br>8</pre></div></div></div>';
        let parsed = render.zoiParsePage(doc, cf, 'cf', p.url, markdown);
        let root = render.zoiPrepareContent(doc, parsed.variants[0].html, 'cf', p.url, cleaner);
        assert.match(root.textContent, /\\\(x\^2\\\)/);
        assert.deepEqual(render.zoiHtmlSamples(root, 'cf'), [{ input: '2\n3 4\n', output: '7\n8\n' }]);
        const atc = '<div id="task-statement"><span class="lang-en"><section><h3>Sample Input 1</h3><pre>1 2\n</pre></section><section><h3>Sample Output 1</h3><pre>3\n</pre></section><p><var>x_i</var></p></span><span class="lang-ja">日本語</span></div>';
        parsed = render.zoiParsePage(doc, atc, 'atc', 'https://atcoder.jp/contests/abc001/tasks/abc001_1', markdown);
        assert.equal(parsed.variants.length, 2);
        root = render.zoiPrepareContent(doc, parsed.variants[0].html, 'atc', 'https://atcoder.jp/', cleaner);
        assert.deepEqual(render.zoiHtmlSamples(root, 'atc'), [{ input: '1 2\n', output: '3\n' }]);
        assert.match(root.textContent, /\\\(x_i\\\)/);
        const highlighted = '<h3>Sample Input 1</h3><pre class="prettyprint"><ol class="linenums"><li><span>15</span></li><li><span>10</span></li></ol></pre><pre class="source-code-for-copy">15\n10\n</pre><h3>Sample Output 1</h3><pre>5\n</pre>';
        root = render.zoiPrepareContent(doc, highlighted, 'atc', 'https://atcoder.jp/', cleaner);
        assert.deepEqual(render.zoiHtmlSamples(root, 'atc'), [{ input: '15\n10\n', output: '5\n' }]);
        assert.equal(root.querySelector('.source-code-for-copy'), null);
        const vjData = { sections: [{ title: '题面', value: { format: 'MD', content: '**bold** $x$' } }, { title: '样例', value: { format: 'HTML', content: '<table class="vjudge_sample"><tbody><tr><td><pre>1</pre></td><td><pre>2</pre></td></tr></tbody></table>' } }, { title: '', value: { format: 'HTML', content: '[pdf:CDN_BASE_URL/abcd?15]' } }] };
        parsed = render.zoiParsePage(doc, '<textarea class="data-json-container">' + JSON.stringify(vjData).replace(/</g, '&lt;') + '</textarea>', 'vj', 'https://vjudge.net/problem/description/1', markdown);
        assert.deepEqual(parsed.pdfs, ['https://cdn.vjudge.net.cn/abcd?15']);
        root = render.zoiPrepareContent(doc, parsed.variants[0].html, 'vj', 'https://vjudge.net/problem/description/1', cleaner);
        assert.deepEqual(render.zoiHtmlSamples(root, 'vj'), [{ input: '1\n', output: '2\n' }]);
        root = render.zoiPrepareContent(doc, '<script>alert(1)</script><img src="/image.png" onerror="alert(1)"><a href="command:evil">bad</a><iframe src="https://evil.test"></iframe><form><input></form><a href="../x">good</a>', 'cf', 'https://codeforces.com/a/b', cleaner);
        assert.equal(root.querySelector('script, iframe, form, input, [onerror], [href^="command:"]'), null);
        assert.equal(root.querySelector('img').src, 'https://codeforces.com/image.png');
        assert.equal(root.querySelectorAll('a')[1].href, 'https://codeforces.com/x');
        const item = (str, y, x = 20) => ({ str, transform: [1, 0, 0, 1, x, y], width: str.length * 5 });
        assert.deepEqual(render.zoiPdfSamples([[item('Sample Input', 90), item('6', 80), item('Sample Output', 60), item('5', 50)]]), [{ input: '6\n', output: '5\n' }]);
        assert.deepEqual(render.zoiPdfSamples([[item('Sample Input', 90), item('Sample Output', 90, 200), item('1', 80)]]), []);
        const payload = core.companionPayload({ ...core.problemUrl(p.url), name: 'A / B' }, [{ input: '8\n', output: 'YES\n' }]);
        assert.equal(payload.url, p.url); assert.equal(payload.tests.length, 1); assert.equal(payload.batch.size, 1);
        let received;
        const server = http.createServer((req, res) => { const chunks = []; req.on('data', c => chunks.push(c)); req.on('end', () => { received = JSON.parse(Buffer.concat(chunks)); res.end('{}'); }); });
        await new Promise(r => server.listen(0, '127.0.0.1', r));
        try { await core.sendToCph(payload, server.address().port); assert.deepEqual(received, payload); } finally { server.close(); }
        const session = await core.createSession(p.url, { timeout: 3000 });
        const post = (route, body = {}, token = session.token) => fetch(`http://127.0.0.1:${session.port}/${route}`, { method: 'POST', headers: { Authorization: 'Bearer ' + token }, body: JSON.stringify({ url: p.url, ...body }) });
        try {
            assert.equal((await post('claim', {}, 'wrong')).status, 403);
            assert.equal((await post('claim', { url: 'https://codeforces.com/contest/5/problem/A' })).status, 409);
            assert.equal((await post('claim')).status, 200);
            assert.equal((await post('claim')).status, 409);
            assert.equal((await post('permit')).status, 409);
            assert.equal((await post('report', { html: cf })).status, 200);
            assert.equal((await session.done).html, cf);
        } finally { session.cancel(); }
        const timeout = await core.createSession(p.url, { timeout: 10 }); assert.match((await timeout.done).error, /超时/);
        const fakeFetch = async () => new Response('Not PDF'); await assert.rejects(core.download('https://cdn.vjudge.net.cn/p', { kind: 'pdf', fetcher: fakeFetch }), /PDF/);
        await assert.rejects(core.download(p.url, { fetcher: async () => new Response('', { status: 302, headers: { location: 'https://evil.test' } }) }), /其他站点/);
        const readDom = new JSDOM(cf, { url: p.url });
        const snapshot = await vm.runInNewContext('(' + zoiStatementPage + ')', { document: readDom.window.document, location: readDom.window.location, URL, Date, setTimeout })({ url: p.url, site: 'cf' }); assert.match(snapshot.html, /problem-statement/);

        const linkDom = new JSDOM('<table><thead><tr><th>Problem</th><th>Source</th></tr></thead><tbody>' + [
            ['https://codeforces.com/contest/4/problem/A', 'CF'], ['https://atcoder.jp/contests/abc001/tasks/abc001_1', 'AtCoder'], ['https://lightoj.com/problem/unlucky-strings', 'LightOJ'], ['https://vjudge.net/problem/UVA-455', 'VJudge']
        ].map(([url, src]) => `<tr><td><a href="${url}">A &amp; 中文</a></td><td>${src}</td></tr>`).join('') + '</tbody></table>', { url: 'https://youkn0wwho.academy/topic-list/kmp' });
        let count = 0;
        const schedule = fn => { assert.ok(++count < 20, 'Mutation observers must settle, not ping-pong'); setTimeout(fn, 0); };
        const a = links.zoiYouknowwhoInstall(linkDom.window.document, numbers, linkDom.window.MutationObserver, schedule);
        const b = links.zoiYouknowwhoImportInstall(linkDom.window.document, numbers, linkDom.window.MutationObserver, schedule);
        await new Promise(r => setTimeout(r, 50));
        assert.equal(linkDom.window.document.querySelectorAll('[data-zoi-ykw-link]').length, 1);
        const buttons = [...linkDom.window.document.querySelectorAll('[data-zoi-ykw-import]')]; assert.equal(buttons.length, 4);
        assert.equal(new URL(buttons[2].href).searchParams.get('url'), 'https://vjudge.net/problem/LightOJ-1268');
        assert.equal(new URL(buttons[0].href).searchParams.get('title'), 'A & 中文');
        a.refresh(); b.refresh(); await new Promise(r => setTimeout(r, 50)); assert.equal(buttons.length, 4); a.disconnect(); b.disconnect();
        assert.equal(new URL(links.zoiYouknowwhoImport('https://unknown.example/p', 'X', '', numbers)).searchParams.get('url'), 'https://vjudge.net/problem#OJId=All&probNum=&title=X&source=&category=all');
        dom.window.close(); readDom.window.close(); linkDom.window.close();
        assert.equal(fs.readFileSync(target, 'utf8'), 'user code');
        await checkHost(work, cf);
        await require('./check_gym_statement.cjs').checkGym(work, checkHost);
        console.log('PASS statements: CPH association, URL validation, all three parsers, bilingual/MD/PDF, samples, HTML sanitization, one-time read session, CPH transport, YOUKNOWWHO routing and observer stability.');
    } finally {
        if (!path.resolve(work).startsWith(path.resolve(__dirname, '../.zoi-checks') + path.sep)) throw Error('Unexpected test cleanup directory');
        fs.rmSync(work, { recursive: true, force: true });
    }
}
main().catch(e => { console.error(e); process.exitCode = 1; });
