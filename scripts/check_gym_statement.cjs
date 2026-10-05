'use strict';
const assert = require('node:assert/strict');
const fs = require('node:fs');
const path = require('node:path');
const vm = require('node:vm');
const { JSDOM } = require('./statement-extension/node_modules/jsdom');
const core = require('./statement-extension/core.cjs');
const render = require('./statement-extension/renderer.js');
const { zoiStatementPage, zoiStatementLocation } = require('./vjudge-extension/statement.js');
const links = require('./vjudge-extension/youknowwho.js');
const numbers = require('./vjudge-extension/youknowwho-data.js');

async function checkGym(work, checkHost) {
    const url = 'https://codeforces.com/gym/100212/problem/C';
    const attachment = 'https://codeforces.com/gym/100212/attachments';
    const pdf = attachment + '/download/1727/contest.pdf';
    const html = `<table><tr><td>Statements</td><td><a href="${pdf}">Download</a></td></tr></table>`;
    assert.equal(core.gymProblem(url).vjudge, 'https://vjudge.net/problem/Gym-100212C');
    assert.equal(core.gymProblem('https://codeforces.com/problemset/gymProblem/100212/C').index, 'C');
    assert.equal(core.gymProblem('https://codeforces.com/contest/100212/problem/C'), null);
    assert.deepEqual(core.gymPdfLinks(html, url), [pdf]);
    assert.deepEqual(core.gymPdfLinks(html + '<div class="problem-statement">HTML wins</div>', url), []);
    assert.deepEqual(core.gymPdfLinks(html.replace('/gym/100212/attachments', '/gym/100213/attachments'), url), []);
    assert.deepEqual(core.gymPdfLinks(html.replace('codeforces.com', 'codeforces.com.evil.test'), url), []);
    assert.equal(core.pdfUrl(pdf), pdf);
    assert.throws(() => core.pdfUrl('https://codeforces.com/profile/user'));
    assert.equal(zoiStatementLocation(url, attachment), true);
    for (const bad of [attachment.replace('100212', '100213'), attachment.replace('codeforces.com', 'evil.test'), attachment + '?other=1', 'https://codeforces.com/gym/100212/problem/A']) assert.equal(zoiStatementLocation(url, bad), false);
    const dom = new JSDOM(html, { url: attachment });
    const snapshot = await vm.runInNewContext('(' + zoiStatementPage + ')', { document: dom.window.document, location: dom.window.location, URL, Date, setTimeout })({ url, site: 'cf' });
    assert.deepEqual(core.gymPdfLinks(snapshot.html, url), [pdf]);
    const wrong = new JSDOM(html, { url: attachment.replace('100212', '100213') });
    await assert.rejects(vm.runInNewContext('(' + zoiStatementPage + ')', { document: wrong.window.document, location: wrong.window.location, URL, Date, setTimeout })({ url, site: 'cf' }), /切换/);
    wrong.window.close();
    const parsed = render.zoiParsePage(dom.window.document, '', 'gym-pdf', url, null, [pdf]);
    assert.deepEqual(parsed.pdfs, [pdf]); assert.match(parsed.variants[0].html, /data-pdf="0"/);
    const search = new URL(new URL(links.zoiYouknowwhoImport(pdf, 'Order-Preserving Codes', 'Codeforces', numbers)).searchParams.get('url'));
    assert.equal(search.pathname, '/problem'); assert.match(search.hash, /OJId=Gym/);
    assert.equal(new URL(links.zoiYouknowwhoImport(url, 'Order-Preserving Codes', 'Codeforces', numbers)).searchParams.get('url'), url);

    const session = await core.createSession(url, { timeout: 3000 });
    try {
        let handoff;
        const redirected = new JSDOM('', { url: attachment + new URL(session.url).hash });
        vm.runInNewContext(fs.readFileSync(path.join(__dirname, 'vjudge-extension/statement-handoff.js'), 'utf8'), { location: redirected.window.location, history: redirected.window.history, chrome: { runtime: { sendMessage: message => { handoff = message; return Promise.resolve(); } } } });
        assert.equal(handoff.url, url); assert.equal(redirected.window.location.href, attachment);
        const post = (route, body = {}) => fetch(`http://127.0.0.1:${session.port}/${route}`, { method: 'POST', headers: { Authorization: 'Bearer ' + session.token }, body: JSON.stringify({ url: handoff.url, ...body }) });
        assert.equal((await post('claim')).status, 200);
        assert.equal((await post('report', { html: snapshot.html })).status, 200);
        assert.deepEqual(core.gymPdfLinks((await session.done).html, url), [pdf]);
        redirected.window.close();
    } finally { session.cancel(); }

    const workerSession = await core.createSession(url, { timeout: 3000 });
    try {
        let listener;
        const closed = [];
        const chrome = {
            runtime: { onMessage: { addListener: fn => { listener = fn; } } },
            tabs: { query: async () => [{ id: 2, url: attachment }], get: async () => ({ url: attachment }), remove: async id => closed.push(id) },
            scripting: { executeScript: async ({ target, args: [job] }) => {
                assert.equal(target.tabId, 2); assert.equal(job.url, url);
                return [{ result: snapshot }];
            } },
        };
        vm.runInNewContext(fs.readFileSync(path.join(__dirname, 'vjudge-extension/background.js'), 'utf8'), { chrome, importScripts() {}, zoiStatementPage, zoiStatementLocation, URL, fetch, AbortSignal });
        const result = await new Promise(resolve => listener({ type: 'zoi-statement', url, port: workerSession.port, token: workerSession.token }, { tab: { id: 1 }, frameId: 0, url: attachment }, resolve));
        assert.equal(result.ok, true); assert.deepEqual(closed, [1]);
        assert.deepEqual(core.gymPdfLinks((await workerSession.done).html, url), [pdf]);
    } finally { workerSession.cancel(); }

    // Run the real webview script: changing PDF variants must reject stale download results.
    const ui = new JSDOM('<header><h1 id="title"></h1><div id="limits"></div><select id="versions"></select><button id="refresh"></button><button id="chrome"></button><button id="gymPdf" hidden></button><button id="original"></button><button id="code"></button><p id="status"></p></header><main id="statement"></main><section id="samples"><div id="sample-list"></div></section>');
    const messages = [], cleaner = require('./statement-extension/vendor/dompurify/dist/purify.min.js')(ui.window);
    const view = vm.createContext({ window: ui.window, document: ui.window.document, URL, DOMPurify: cleaner, markdownit: () => ({ render: s => s }), renderMathInElement() {}, acquireVsCodeApi: () => ({ postMessage: m => messages.push(m) }) });
    vm.runInContext(fs.readFileSync(path.join(__dirname, 'statement-extension/renderer.js'), 'utf8'), view);
    const message = data => ui.window.dispatchEvent(new ui.window.MessageEvent('message', { data }));
    message({ type: 'init', problem: { url, name: 'C', tests: [] } });
    message({ type: 'page', site: 'gym-pdf', html: '', base: url, gymPdfs: [pdf, pdf + '?en'], generation: 1 });
    assert.ok(messages.at(-1).pendingPdf);
    const first = messages.find(m => m.type === 'pdf');
    ui.window.document.getElementById('versions').value = '1'; ui.window.document.getElementById('versions').onchange();
    message({ ...first, type: 'pdfError', text: 'stale' });
    assert.doesNotMatch(ui.window.document.getElementById('status').textContent, /失败/);
    const latest = messages.filter(m => m.type === 'pdf').at(-1);
    message({ ...latest, type: 'pdfError', text: 'HTTP 403' });
    assert.match(ui.window.document.getElementById('status').textContent, /失败/);
    assert.equal(ui.window.document.querySelector('#statement a').href, pdf + '?en');
    assert.ok(messages.filter(m => m.type === 'parsed').every(m => m.pendingPdf));
    ui.window.close();

    const { pages } = require('./fixtures/gym-100212-pdf.json');
    const ranges = { A: [0, 1], B: [1, 3], C: [3, 4], D: [4, 5], E: [5, 6], F: [6, 7], G: [7, 8], H: [8, 9], I: [9, 10], J: [10, 11], K: [11, 13] };
    for (const [index, [start, end]] of Object.entries(ranges)) assert.deepEqual(render.zoiGymPdfSelection(pages, index), { start, end });
    assert.deepEqual(render.zoiGymPdfSamples(pages.slice(3, 4)), [{ input: '5\n1 8 2 3 1\n', output: '00\n01\n10\n110\n111\n' }]);
    assert.equal(render.zoiGymPdfSamples(pages.slice(1, 3))[0].output, '3940\n');
    for (const i of [4, 5, 8, 10]) assert.deepEqual(render.zoiGymPdfSamples([pages[i]]), [], 'Never concatenate independent table samples');
    assert.equal(render.zoiGymPdfSelection(pages, 'Z'), null);
    assert.equal(render.zoiGymPdfSelection([pages[0].concat(pages[3])], 'A'), null);
    assert.equal(render.zoiGymPdfSelection([pages[0], pages[3], pages[0]], 'A'), null);
    assert.equal(render.zoiGymPdfSelection([[]], 'A'), null);

    for (const failed of [false, true]) {
        const directory = path.join(work, 'gym-' + failed); fs.mkdirSync(path.join(directory, '.cph'), { recursive: true });
        let reads = 0;
        await checkHost(directory, html, { url, site: failed ? 'gym-pdf' : 'vj', core: {
            createSession: async source => { assert.equal(source, core.gymProblem(url).vjudge); reads++; return { url: source, done: Promise.resolve(failed ? { error: 'Not available' } : { choices: [{ url: 'https://vjudge.net/problem/description/1', label: 'System', selected: true }] }), cancel() {} }; },
            openChrome: async () => {},
            download: async source => source.includes('/description/') ? '<textarea class="data-json-container">{"sections":[{"value":{"format":"HTML","content":"text"}}]}</textarea>' : html,
        } });
        assert.equal(reads, 1);
    }
    dom.window.close();
    console.log('PASS Gym: redirect handoff, exact contest identity, PDF-only fallback, original CPH URL, A–K ranges, C samples, ambiguous tables and stale imports.');
}
module.exports = { checkGym };
