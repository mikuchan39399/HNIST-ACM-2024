'use strict';
const fs = require('node:fs');
const path = require('node:path');
const crypto = require('node:crypto');
const core = require('./core.cjs');

function activate(context) {
    const vscode = require('vscode');
    let current;
    const cacheDir = path.join(context.globalStorageUri.fsPath, 'statements');
    function cacheFile(key) { return path.join(cacheDir, crypto.createHash('sha256').update(key).digest('hex') + '.json'); }
    function cached(key) {
        try { const x = JSON.parse(fs.readFileSync(cacheFile(key), 'utf8')); if (x.version === 1 && Date.now() - x.time < 7 * 86400000) return x.value; } catch {}
    }
    function remember(key, value) {
        fs.mkdirSync(cacheDir, { recursive: true });
        fs.writeFileSync(cacheFile(key), JSON.stringify({ version: 1, time: Date.now(), value }));
        const files = fs.readdirSync(cacheDir).filter(n => /^[a-f0-9]{64}\.json$/.test(n)).map(n => ({ name: n, time: fs.statSync(path.join(cacheDir, n)).mtimeMs })).sort((a, b) => b.time - a.time);
        for (const old of files.slice(100)) fs.unlinkSync(path.join(cacheDir, old.name));
    }
    function createPanel(problem) {
        const panel = vscode.window.createWebviewPanel('zoi.statement', '题面 · ' + problem.name, { viewColumn: vscode.ViewColumn.Beside, preserveFocus: true }, {
            enableScripts: true, retainContextWhenHidden: true, localResourceRoots: [context.extensionUri],
        });
        const state = { panel, problem, generation: 0, controller: null, session: null, choices: [], index: 0, busy: false, disposed: false, importing: false };
        const post = data => { if (!state.disposed) void panel.webview.postMessage(data); };
        const asset = p => panel.webview.asWebviewUri(vscode.Uri.joinPath(context.extensionUri, p)).toString();
        const nonce = crypto.randomBytes(20).toString('hex');
        const csp = panel.webview.cspSource;
        panel.webview.html = `<!doctype html><html lang="zh-CN"><head><meta charset="utf-8"><meta name="viewport" content="width=device-width, initial-scale=1"><meta http-equiv="Content-Security-Policy" content="default-src 'none'; img-src ${csp} https: data: blob:; style-src ${csp} 'unsafe-inline'; font-src ${csp}; script-src 'nonce-${nonce}' ${csp}; worker-src ${csp} blob:; connect-src ${csp};"><link rel="stylesheet" href="${asset('style.css')}"><link rel="stylesheet" href="${asset('vendor/katex/dist/katex.min.css')}"><link rel="stylesheet" href="${asset('vendor/pdfjs-dist/web/pdf_viewer.css')}"></head><body>
        <header><div class="eyebrow">ZOI · 题面</div><h1 id="title">正在读取题目信息…</h1><div id="limits"></div><nav><select id="versions" aria-label="题面版本" hidden></select><button id="refresh">刷新</button><button id="chrome">从 Chrome 读取</button><button id="original">原网页</button><button id="code">回到代码</button></nav><p id="status" role="status"></p></header>
        <main id="statement"></main><section id="samples" hidden><h2>CPH 样例</h2><div id="sample-list"></div></section>
        <script nonce="${nonce}" src="${asset('vendor/dompurify/dist/purify.min.js')}"></script><script nonce="${nonce}" src="${asset('vendor/markdown-it/dist/browser/markdown-it.umd.min.js')}"></script><script nonce="${nonce}" src="${asset('vendor/katex/dist/katex.min.js')}"></script><script nonce="${nonce}" src="${asset('vendor/katex/dist/contrib/auto-render.min.js')}"></script><script nonce="${nonce}" src="${asset('renderer.js')}"></script></body></html>`;
        const stop = () => { state.controller?.abort(); state.session?.cancel(); state.session = null; };
        async function importProblem(tests) {
            if (!problem.importDirectory || state.importing || state.disposed) return;
            state.importing = true;
            try {
                if (!vscode.workspace.isTrusted) throw Error('请先信任刷题工作区。');
                const existing = core.findImported(problem.importDirectory, problem.url);
                let file = existing;
                if (!file) {
                    const cph = vscode.extensions.getExtension('DivyanshuAgrawal.competitive-programming-helper');
                    if (!cph) throw Error('请先安装 CPH 扩展。');
                    await cph.activate();
                    await core.sendToCph(core.companionPayload(problem, tests));
                    const deadline = Date.now() + 15000;
                    while (!file && Date.now() < deadline) { await new Promise(r => setTimeout(r, 200)); file = core.findImported(problem.importDirectory, problem.url); }
                    if (!file) throw Error('已发送到 CPH，但未在当前工作区确认题目文件。请查看 CPH 的语言选择或保存提示；多窗口时仅第一个 CPH 接收窗口有效。请勿重复点击导入。');
                }
                problem.file = file; delete problem.importDirectory;
                await vscode.window.showTextDocument(vscode.Uri.file(file), { viewColumn: vscode.ViewColumn.One });
                panel.reveal(vscode.ViewColumn.Two, true);
                post({ type: 'status', text: existing ? '已打开现有题目，代码和样例保持原样。' : `已生成题目文件，导入 ${tests.length} 组样例。` });
                if (!existing && !tests.length) vscode.window.showWarningMessage('ZOI：题目文件已生成，但未能可靠提取样例，请在 CPH 手动补充。');
            } catch (e) { post({ type: 'error', text: e.message }); vscode.window.showErrorMessage('ZOI 导入：' + e.message); }
        }
        async function browserRead() {
            post({ type: 'status', text: '正在从 Chrome 读取题面。若网页要求登录或验证，请在浏览器中完成；不会发送提交。' });
            const session = await core.createSession(problem.url); state.session = session;
            try { await core.openChrome(session.url); const data = await session.done; if (data.error) throw Error(data.error); return data; }
            finally { session.cancel(); if (state.session === session) state.session = null; }
        }
        async function load(force = false, browser = false, choice = null) {
            stop(); const generation = ++state.generation;
            state.controller = new AbortController(); state.busy = true;
            const signal = state.controller.signal;
            post({ type: 'status', text: '正在加载题面…' });
            try {
                const rootKey = problem.url;
                let root = !force && cached(rootKey);
                if (!root) {
                    if (browser || problem.site === 'vj') root = await browserRead();
                    else {
                        try {
                            const html = await core.download(problem.url, { signal });
                            if (!(problem.site === 'cf' ? /class=["'][^"']*problem-statement/ : /id=["']task-statement/).test(html)) throw Error('未读到题面正文。');
                            root = { html, base: problem.url };
                        } catch (error) { if (signal.aborted) throw error; root = await browserRead(); }
                    }
                }
                if (generation !== state.generation || state.disposed) return;
                if (root.choices) {
                    state.choices = root.choices.map(c => ({ ...c, url: core.descriptionUrl(c.url, problem.url) }));
                    state.index = choice === null ? Math.max(0, state.choices.findIndex(c => c.selected)) : choice;
                    const selected = state.choices[state.index]; if (!selected) throw Error('题面版本不存在。');
                    const descKey = selected.url;
                    let detail = !force && cached(descKey);
                    if (!detail) detail = { html: await core.download(descKey, { signal }), base: descKey };
                    if (!/data-json-container/.test(detail.html)) throw Error('该版本未返回题面内容，请点击“原网页”核对。');
                    if (generation !== state.generation) return;
                    remember(descKey, detail);
                    post({ type: 'page', ...detail, site: 'vj', choices: state.choices, selected: state.index, generation });
                } else post({ type: 'page', ...root, site: problem.site, generation });
                remember(rootKey, root);
            } catch (error) { if (generation === state.generation && !state.disposed) post({ type: 'error', text: error.message }); }
            finally { if (generation === state.generation) state.busy = false; }
        }
        panel.webview.onDidReceiveMessage(async message => {
            try {
                if (message.type === 'ready') {
                    post({ type: 'init', problem, pdfModule: asset('vendor/pdfjs-dist/build/pdf.min.mjs'), pdfWorker: asset('vendor/pdfjs-dist/build/pdf.worker.min.mjs'), pdfAssets: asset('vendor/pdfjs-dist') + '/' });
                    await load();
                } else if (message.type === 'refresh') await load(true);
                else if (message.type === 'chrome') await load(true, true);
                else if (message.type === 'choose' && Number.isInteger(message.index) && state.choices[message.index]) await load(false, false, message.index);
                else if (message.type === 'original') await vscode.env.openExternal(vscode.Uri.parse(problem.url));
                else if (message.type === 'code' && problem.file) await vscode.window.showTextDocument(vscode.Uri.file(problem.file), { viewColumn: vscode.ViewColumn.One });
                else if (message.type === 'parsed' && message.generation === state.generation && !message.pendingPdf && Array.isArray(message.tests)) {
                    const tests = message.tests.slice(0, 100).filter(t => typeof t.input === 'string' && typeof t.output === 'string' && t.input.length + t.output.length <= 1024 * 1024);
                    await importProblem(tests);
                }
                else if (message.type === 'parseError' && problem.importDirectory) post({ type: 'error', text: '题面解析未完成，尚未生成题目文件：' + message.text });
                else if (message.type === 'link' && typeof message.url === 'string') {
                    const u = new URL(message.url); if (['https:', 'http:'].includes(u.protocol) && !u.username && !u.password) await vscode.env.openExternal(vscode.Uri.parse(u.href));
                } else if (message.type === 'pdf' && message.generation === state.generation && Number.isInteger(message.id)) {
                    const data = await core.download(core.pdfUrl(message.url), { kind: 'pdf', signal: state.controller.signal });
                    if (message.generation === state.generation) post({ type: 'pdf', data, id: message.id, generation: state.generation });
                }
            } catch (e) { post({ type: 'error', text: e.message }); }
        }, null, context.subscriptions);
        panel.onDidDispose(() => { state.disposed = true; stop(); if (current === state) current = null; }, null, context.subscriptions);
        state.load = load;
        return state;
    }
    context.subscriptions.push(vscode.commands.registerCommand('zoi.showStatement', async () => {
        try {
            if (!vscode.workspace.isTrusted) throw Error('请先信任当前工作区，再读取 CPH 题目。');
            const editor = vscode.window.activeTextEditor;
            if (!editor || editor.document.uri.scheme !== 'file') { if (current) { current.panel.reveal(vscode.ViewColumn.Beside, true); return; } throw Error('请先打开 Companion 导入的代码文件。'); }
            const problem = core.readProblem(editor.document.uri.fsPath);
            if (current?.problem.file === problem.file && current.problem.url === problem.url) { current.panel.reveal(undefined, true); return; }
            current?.panel.dispose(); current = createPanel(problem);
        } catch (e) { vscode.window.showErrorMessage('ZOI 题面：' + e.message); }
    }));
    context.subscriptions.push(vscode.window.registerUriHandler({ async handleUri(uri) {
        try {
            if (uri.path !== '/import') throw Error('不支持的 ZOI 操作。');
            if (!vscode.workspace.isTrusted) throw Error('请先打开并信任刷题文件夹。');
            const directories = (vscode.workspace.workspaceFolders || []).filter(f => f.uri.scheme === 'file');
            if (!directories.length) throw Error('请先在 VS Code 打开刷题文件夹，再点击导入。');
            const query = new URLSearchParams(uri.query);
            let target;
            try { target = core.problemUrl(query.get('url')); }
            catch {
                const search = core.searchUrl(query.get('url'));
                const session = await core.createSession(search.url);
                try {
                    await core.openChrome(session.url);
                    const response = await session.done; if (response.error) throw Error(response.error);
                    const selected = await vscode.window.showQuickPick(response.results.map(r => ({ label: r.label, description: r.url, url: r.url })), { title: '选择要导入的 VJudge 题目', placeHolder: '原链接没有可靠题号映射，请核对题名和来源' });
                    if (!selected) return; target = core.problemUrl(selected.url);
                } finally { session.cancel(); }
            }
            const problem = { ...target, name: String(query.get('title') || '题目').slice(0, 160), tests: [] };
            if (current?.problem.url === problem.url && current.importing) { current.panel.reveal(undefined, true); return; }
            // CPH receives into the first workspace folder; follow the same rule.
            const directory = directories[0].uri.fsPath, existing = core.findImported(directory, problem.url);
            if (existing) {
                await vscode.window.showTextDocument(vscode.Uri.file(existing), { viewColumn: vscode.ViewColumn.One });
                await vscode.commands.executeCommand('zoi.showStatement'); return;
            }
            problem.importDirectory = directory;
            current?.panel.dispose(); current = createPanel(problem);
        } catch (e) { vscode.window.showErrorMessage('ZOI 导入：' + e.message); }
    } }));
    context.subscriptions.push({ dispose: () => current?.panel.dispose() });
}
module.exports = { activate };
