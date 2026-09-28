'use strict';
const fs = require('node:fs');
const path = require('node:path');
const http = require('node:http');
const crypto = require('node:crypto');
const { spawn } = require('node:child_process');
const MAX_HTML = 8 * 1024 * 1024;
const CF = ['codeforces.com', 'www.codeforces.com', 'm1.codeforces.com', 'm2.codeforces.com'];
const VJ = ['vjudge.net', 'vjudge.net.cn'];

function problemUrl(value) {
    const u = new URL(value);
    if (!['https:', 'http:'].includes(u.protocol) || u.username || u.password || u.port) throw Error('题目链接必须是受支持的 OJ 地址。');
    u.protocol = 'https:';
    if (CF.includes(u.hostname) && /^\/(?:problemset\/(?:problem|gymProblem)\/\d+\/[A-Za-z0-9]+|problemsets\/acmsguru\/problem\/\d+\/\d+|(?:contest|gym)\/\d+\/problem\/[A-Za-z0-9]+|group\/[A-Za-z0-9]+\/contest\/\d+\/problem\/[A-Za-z0-9]+|edu\/course\/\d+\/lesson\/\d+\/\d+\/practice\/contest\/\d+\/problem\/[A-Za-z0-9]+)\/?$/.test(u.pathname)) {
        u.hash = ''; u.search = ''; return { site: 'cf', url: u.href };
    }
    if (u.hostname === 'atcoder.jp' && /^\/contests\/[\w-]+\/tasks\/[\w-]+\/?$/.test(u.pathname)) {
        u.hash = ''; u.search = '?lang=en'; return { site: 'atc', url: u.href };
    }
    if (VJ.includes(u.hostname)) {
        if (/^\/problem\/(?:[A-Za-z0-9_]+|洛谷)-[A-Za-z0-9_.-]+\/?$/.test(decodeURIComponent(u.pathname))) u.hash = '';
        else if (!/^\/contest\/\d+\/?$/.test(u.pathname) || !/^#problem\/[A-Z][A-Z0-9]*$/.test(u.hash)) throw Error('VJudge 比赛链接需要包含具体题号。');
        u.search = ''; return { site: 'vj', url: u.href };
    }
    throw Error('目前支持 Codeforces、AtCoder 和 VJudge 的具体题目链接。');
}
function searchUrl(value) {
    const u = new URL(value), params = new URLSearchParams(u.hash.slice(1));
    if (u.protocol !== 'https:' || !VJ.includes(u.host) || u.username || u.password || u.pathname !== '/problem' || u.search || !params.get('title')?.trim() || params.get('title').length > 180 || [...params.keys()].some(k => !['OJId', 'probNum', 'title', 'source', 'category'].includes(k))) throw Error('无效的 VJudge 题名搜索。');
    return { site: 'vj-search', url: u.href };
}

function problemKey(value) {
    const target = problemUrl(value), u = new URL(target.url);
    let route = u.pathname.replace(/\/$/, '');
    if (target.site === 'cf') route = route.replace(/^\/problemset\/problem\/(\d+)\/([^/]+)$/, '/contest/$1/problem/$2').replace(/^\/problemset\/gymProblem\/(\d+)\/([^/]+)$/, '/gym/$1/problem/$2');
    return target.site + ':' + route + u.hash;
}

function readProblem(file) {
    const dir = path.join(path.dirname(file), '.cph');
    const same = (a, b) => process.platform === 'win32' ? path.resolve(a).toLowerCase() === path.resolve(b).toLowerCase() : path.resolve(a) === path.resolve(b);
    if (!fs.existsSync(dir)) throw Error('没有找到当前文件的 CPH 题目信息，请先用 Companion 导入这道题。');
    const matches = [];
    for (const name of fs.readdirSync(dir)) {
        if (!name.endsWith('.prob') || !name.startsWith('.' + path.basename(file) + '_')) continue;
        const target = path.join(dir, name);
        if (fs.statSync(target).size > MAX_HTML) continue;
        let p; try { p = JSON.parse(fs.readFileSync(target, 'utf8').replace(/^\uFEFF/, '')); } catch { continue; }
        if (typeof p.srcPath !== 'string' || !same(p.srcPath, file)) continue;
        matches.push(p);
    }
    if (!matches.length) throw Error('当前代码没有对应的 CPH 链接；请先用 Companion 导入，或在原来的 CPH 文件中打开代码。');
    const urls = new Set(matches.map(p => problemKey(p.url)));
    if (urls.size !== 1) throw Error('发现多个不同题目的 CPH 关联，请先在 CPH 中核对当前文件。');
    const p = matches[0], target = problemUrl(p.url);
    return { ...target, file, name: String(p.name || path.basename(file)), timeLimit: p.timeLimit, memoryLimit: p.memoryLimit,
        tests: Array.isArray(p.tests) ? p.tests.filter(t => typeof t.input === 'string' && typeof t.output === 'string').map(t => ({ input: t.input, output: t.output })) : [] };
}

function descriptionUrl(value, base) {
    const u = new URL(value, base), b = new URL(base);
    if (u.origin !== b.origin || !VJ.includes(u.hostname) || !/^\/problem\/description\/\d+$/.test(u.pathname) || u.username || u.password || u.hash) throw Error('不是本题的 VJudge 题面版本地址。');
    return u.href;
}
function pdfUrl(value) {
    const u = new URL(value);
    if (u.protocol !== 'https:' || u.port || u.username || u.password || !['cdn.vjudge.net', 'cdn.vjudge.net.cn', 'onlinejudge.org', 'uva.onlinejudge.org'].includes(u.hostname)) throw Error('此 PDF 地址不在支持的题面站点中，请使用“原网页”查看。');
    return u.href;
}

async function download(url, { kind = 'html', signal, fetcher = fetch } = {}) {
    let current = new URL(url);
    const initial = current.origin;
    for (let hop = 0; hop < 4; hop++) {
        const r = await fetcher(current.href, { redirect: 'manual', signal: signal ? AbortSignal.any([signal, AbortSignal.timeout(25000)]) : AbortSignal.timeout(25000), headers: { Accept: kind === 'pdf' ? 'application/pdf' : 'text/html' } });
        if ([301, 302, 303, 307, 308].includes(r.status)) {
            await r.body?.cancel();
            const next = new URL(r.headers.get('location'), current);
            if (next.origin !== initial || next.username || next.password) throw Error('题面跳转到了其他站点，请用浏览器读取。');
            current = next; continue;
        }
        if (!r.ok) { await r.body?.cancel(); throw Error(`题面服务器返回 HTTP ${r.status}，可使用“从 Chrome 读取”。`); }
        const chunks = []; let size = 0;
        const limit = kind === 'pdf' ? 32 * 1024 * 1024 : MAX_HTML;
        for await (const chunk of r.body) {
            size += chunk.length;
            if (size > limit) throw Error('题面文件过大，请在原网页查看。');
            chunks.push(chunk);
        }
        const data = Buffer.concat(chunks);
        if (kind === 'pdf') {
            if (!data.subarray(0, 1024).includes(Buffer.from('%PDF-'))) throw Error('服务器没有返回 PDF 文件。');
            return data.toString('base64');
        }
        return data.toString('utf8');
    }
    throw Error('题面重定向次数过多。');
}

// Only the one-time browser job can supply its own statement. No code or credentials cross this channel.
async function createSession(url, { timeout = 120000 } = {}) {
    let job; try { job = problemUrl(url); } catch { job = searchUrl(url); }
    const token = crypto.randomBytes(32).toString('hex');
    let state = 'new', ended = false, timer, settle;
    const done = new Promise(resolve => { settle = resolve; });
    const finish = value => { if (ended) return; ended = true; clearTimeout(timer); server.close(); server.closeIdleConnections?.(); settle(value); };
    const server = http.createServer(async (req, res) => {
        res.setHeader('Cache-Control', 'no-store');
        const reply = (status, data) => { res.writeHead(status, { 'Content-Type': 'application/json' }); res.end(JSON.stringify(data)); };
        if (req.method !== 'POST' || req.headers.host !== `127.0.0.1:${server.address()?.port}` || req.headers.authorization !== `Bearer ${token}` || (req.headers.origin && !/^chrome-extension:\/\/[a-p]{32}$/.test(req.headers.origin))) return reply(403, { error: 'Unauthorized' });
        try {
            const chunks = []; let size = 0;
            for await (const chunk of req) { size += chunk.length; if (size > MAX_HTML * 2) { reply(413, { error: '题面过大。' }); req.destroy(); return; } chunks.push(chunk); }
            const body = JSON.parse(Buffer.concat(chunks).toString('utf8'));
            if (ended || body.url !== job.url) return reply(409, { error: '题目链接不匹配。' });
            if (req.url === '/claim' && state === 'new') { state = 'claimed'; return reply(200, job); }
            if (req.url !== '/report' || state !== 'claimed') return reply(409, { error: '题面请求已领取或已结束。' });
            let result;
            if (body.error) result = { error: String(body.error).slice(0, 800) };
            else if (job.site === 'vj-search') {
                const results = (Array.isArray(body.results) ? body.results : []).slice(0, 50).map(c => ({ url: problemUrl(c.url).url, label: String(c.label).slice(0, 200) }));
                if (!results.length || results.some(c => !VJ.includes(new URL(c.url).host))) throw Error('VJudge 搜索没有找到题目，请在原网页核对题名或收录情况。');
                result = { results };
            } else if (job.site === 'vj') {
                const choices = (Array.isArray(body.choices) ? body.choices : []).slice(0, 100).map(c => ({ url: descriptionUrl(c.url, job.url), label: String(c.label).slice(0, 160), selected: c.selected === true }));
                if (!choices.length) throw Error('网页没有提供可用的题面版本。');
                result = { choices };
            } else {
                if (typeof body.html !== 'string' || !body.html.trim() || Buffer.byteLength(body.html) > MAX_HTML) throw Error('没有读到有效题面。');
                result = { html: body.html, base: job.url };
            }
            state = 'done'; reply(200, { received: true }); finish(result);
        } catch (e) { reply(400, { error: e.message }); }
    });
    server.requestTimeout = 15000;
    await new Promise((resolve, reject) => { server.once('error', reject); server.listen(0, '127.0.0.1', resolve); });
    timer = setTimeout(() => finish({ error: 'Chrome 题面读取超时。请确认 ZOI Submit 已重载为 1.4.0，网页已完成登录或验证，再点击“从 Chrome 读取”。' }), timeout);
    const target = new URL(job.url); target.hash += `${target.hash ? '&' : ''}zoi-statement=${server.address().port}.${token}`;
    return { url: target.href, port: server.address().port, token, done, cancel: () => finish({ error: '已取消题面读取。' }) };
}
function openChrome(url) {
    const exe = [process.env.PROGRAMFILES, process.env['PROGRAMFILES(X86)'], process.env.LOCALAPPDATA].filter(Boolean).map(p => path.join(p, 'Google/Chrome/Application/chrome.exe')).find(p => fs.existsSync(p));
    if (!exe) throw Error('未找到 Chrome，请先安装并加载 ZOI Submit 扩展。');
    return new Promise((resolve, reject) => {
        const child = spawn(exe, ['--new-tab', url], { detached: true, stdio: 'ignore', windowsHide: true });
        child.once('error', reject); child.once('spawn', () => { child.unref(); resolve(); });
    });
}
function findImported(directory, url) {
    const dir = path.join(directory, '.cph');
    if (!fs.existsSync(dir)) return null;
    const key = problemKey(url);
    for (const name of fs.readdirSync(dir).filter(n => n.endsWith('.prob'))) {
        try {
            const file = path.join(dir, name); if (fs.statSync(file).size > MAX_HTML) continue;
            const p = JSON.parse(fs.readFileSync(file, 'utf8'));
            if (problemKey(p.url) === key && typeof p.srcPath === 'string' && path.dirname(path.resolve(p.srcPath)).toLowerCase() === path.resolve(directory).toLowerCase() && fs.existsSync(p.srcPath)) return p.srcPath;
        } catch {}
    }
    return null;
}
function companionPayload(problem, tests) {
    const url = new URL(problem.url), id = problem.site === 'vj' ? url.pathname.split('/').pop() + url.hash.replace('#problem/', '_') : problem.site === 'atc' ? url.pathname.split('/').pop() : 'CF_' + url.pathname.split('/').filter(p => /^\d+$|^[A-Z]\d?$/i.test(p)).join('_');
    return { name: id.replace(/[^\w-]/g, '_') + '_' + String(problem.name).replace(/[^\p{L}\p{N}_ ]/gu, '_').slice(0, 100), group: ({ cf: 'Codeforces', atc: 'AtCoder', vj: 'Virtual Judge' })[problem.site], url: problem.url, interactive: false,
        memoryLimit: Number(problem.memoryLimit) || 256, timeLimit: Number(problem.timeLimit) || 2000, tests, testType: 'single', input: { type: 'stdin' }, output: { type: 'stdout' }, languages: { java: { mainClass: 'Main', taskClass: 'Main' } }, batch: { id: crypto.randomUUID(), size: 1 } };
}
async function sendToCph(payload, port = 27121) {
    const data = Buffer.from(JSON.stringify(payload));
    await new Promise((resolve, reject) => {
        const req = http.request({ hostname: '127.0.0.1', port, method: 'POST', path: '/', headers: { 'Content-Type': 'application/json', 'Content-Length': data.length }, timeout: 10000 }, res => {
            res.resume(); res.once('end', () => res.statusCode === 200 ? resolve() : reject(Error('CPH 返回 HTTP ' + res.statusCode)));
        });
        req.once('timeout', () => req.destroy(Error('连接 CPH 超时。'))); req.once('error', reject); req.end(data);
    });
}
module.exports = { MAX_HTML, problemUrl, problemKey, searchUrl, readProblem, descriptionUrl, pdfUrl, download, createSession, openChrome, findImported, companionPayload, sendToCph };
