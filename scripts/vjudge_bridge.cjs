'use strict';

const http = require('node:http');
const crypto = require('node:crypto');
const fs = require('node:fs');
const path = require('node:path');
const { spawn } = require('node:child_process');
const active = new Set();

function problemUrl(value) {
    const u = new URL(value);
    if (u.protocol !== 'https:' || !['vjudge.net', 'vjudge.net.cn'].includes(u.hostname) || u.port || u.username || u.password) throw Error('不是受支持的 VJudge 题目地址。');
    if (/^\/problem\/[A-Za-z0-9_]+-[A-Za-z0-9_.-]+$/.test(u.pathname)) u.hash = '';
    else if (!/^\/contest\/\d+$/.test(u.pathname) || !/^#problem\/[A-Z][A-Z0-9]*$/.test(u.hash)) throw Error('请用 Competitive Companion 从具体题目页面拉题（比赛页需选中一道题）。');
    u.search = '';
    return u.href;
}
function isVjudge(value) { try { problemUrl(String(value)); return true; } catch { return false; } }

// A submission exists in memory only. Claim and permit are each single-use.
async function createSession(job, validate, { timeout = 120000, onFallback = () => {} } = {}) {
    const token = crypto.randomBytes(32).toString('hex');
    let state = 'new', settle, timer;
    const done = new Promise(resolve => { settle = resolve; });
    const finish = result => { clearTimeout(timer); job = null; server.close(); server.closeIdleConnections?.(); settle(result); };
    const server = http.createServer(async (req, res) => {
        res.setHeader('Cache-Control', 'no-store');
        const reply = (status, data) => { res.writeHead(status, { 'Content-Type': 'application/json' }); res.end(JSON.stringify(data)); };
        if (req.headers.host !== `127.0.0.1:${server.address()?.port}` || req.method !== 'POST' || req.headers.authorization !== `Bearer ${token}` || (req.headers.origin && !/^chrome-extension:\/\/[a-p]{32}$/.test(req.headers.origin))) return reply(403, { error: 'Unauthorized' });
        let raw = '';
        try {
            for await (const chunk of req) { raw += chunk; if (raw.length > 8192) { reply(413, { error: 'Too large' }); req.destroy(); return; } }
            const body = JSON.parse(raw);
            if (!job || body.url !== job.url) return reply(409, { error: '题目地址不匹配或任务已结束。' });
            if (req.url === '/claim' && state === 'new') {
                validate(); state = 'claimed'; return reply(200, job);
            }
            if (req.url === '/permit' && state === 'claimed') {
                if (job.mode === 'check') return reply(409, { error: '连接检查不允许提交，请重新加载 Chrome 扩展后再检查。' });
                validate();
                if (body.languageFallback === true) {
                    const language = String(body.language || '').trim().slice(0, 200);
                    if (!language) throw Error('回退编译器信息缺失，停止提交。');
                    onFallback(language);
                }
                state = 'permitted'; return reply(200, { allowed: true });
            }
            if (req.url === '/report' && ['claimed', 'permitted'].includes(state)) {
                const result = { ok: state === 'permitted' && body.ok === true && /^\d+$/.test(String(body.runId)), prepared: state === 'claimed' && job.mode === 'check' && body.prepared === true, runId: String(body.runId || ''), message: String(body.message || '').slice(0, 1000) };
                state = 'done'; reply(200, { received: true }); finish(result); return;
            }
            reply(409, { error: '任务已领取或已提交；请先检查网页记录，不要重复提交。' });
        } catch (e) { reply(400, { error: e.message }); }
    });
    server.requestTimeout = 10000;
    await new Promise((resolve, reject) => { server.once('error', reject); server.listen(0, '127.0.0.1', resolve); });
    timer = setTimeout(() => finish({ ok: false, message: state === 'new' ? 'Chrome 提交扩展未连接。请加载 scripts/vjudge-extension 后重试。' : '等待提交结果超时，请先检查 VJudge 提交记录；工具不会自动重试。' }), timeout);
    const target = new URL(job.url);
    target.hash += `${target.hash ? '&' : ''}zoi-submit=${server.address().port}.${token}`;
    return { url: target.href, port: server.address().port, token, done, cancel: () => finish({ ok: false, message: '已取消提交。' }) };
}

function openChrome(url) {
    const candidates = [process.env.PROGRAMFILES, process.env['PROGRAMFILES(X86)'], process.env.LOCALAPPDATA].filter(Boolean).map(p => path.join(p, 'Google/Chrome/Application/chrome.exe'));
    const executable = candidates.find(p => fs.existsSync(p));
    if (!executable) throw Error('未找到 Chrome，请安装 Chrome 后重试。');
    return new Promise((resolve, reject) => {
        const child = spawn(executable, ['--new-tab', url], { detached: true, stdio: 'ignore', windowsHide: true });
        child.once('error', reject); child.once('spawn', () => { child.unref(); resolve(); });
    });
}

async function submit(vscode, problem, prepareDocument) {
    const url = problemUrl(problem.url), file = problem.srcPath;
    if (!/\.cpp$/i.test(file)) throw Error('VJudge 自动提交目前只支持 .cpp（优先 GNU C++20，O2）。');
    if (active.has(file)) throw Error('该文件已有 VJudge 提交在处理中，请先查看 Chrome 中的结果。');
    active.add(file);
    let session;
    try {
        const document = await vscode.workspace.openTextDocument(vscode.Uri.file(file));
        const version = document.version, original = document.getText();
        const code = await prepareDocument(vscode, document);
        const validate = () => { if (!vscode.workspace.isTrusted || document.isClosed || version !== document.version || original !== document.getText()) throw Error('准备提交期间代码发生变化，本次提交已停止，请重新提交。'); };
        validate();
        if (!code.trim() || Buffer.byteLength(code) > 4 * 1024 * 1024) throw Error('提交代码为空或超过 4 MiB。');
        session = await createSession({ url, code: '#pragma GCC optimize("O2")\n' + code }, validate, {
            onFallback: language => { void vscode.window.showWarningMessage(`VJudge：此题没有 GNU C++20，已自动改用 ${language}（O2）。代码需兼容该版本。`); },
        });
        await openChrome(session.url);
        vscode.window.showInformationMessage('正在连接 Chrome 提交 VJudge（优先 GNU C++20，O2）。');
        const result = await session.done;
        if (result.ok) vscode.window.showInformationMessage(`VJudge 已接收提交 #${result.runId}，请在网页查看评测结果。`);
        else vscode.window.showErrorMessage('VJudge：' + (result.message || '未确认提交成功，请检查网页。'));
    } finally { session?.cancel(); active.delete(file); }
}

module.exports = { problemUrl, isVjudge, createSession, submit, openChrome };
