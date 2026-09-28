'use strict';
importScripts('page.js', 'atcoder.js', 'statement.js');

chrome.runtime.onMessage.addListener((message, sender, reply) => {
    if (message?.type === 'zoi-statement') return zoiHandleStatement(message, sender, reply);
    if (!['zoi-vjudge-submit', 'zoi-atcoder-submit'].includes(message?.type)) return;
    (async () => {
        if (!sender.tab || sender.frameId !== 0 || !Number.isInteger(message.port) || message.port < 1024 || message.port > 65535 || !/^[a-f0-9]{64}$/.test(message.token)) throw Error('无效的提交请求。');
        const url = new URL(message.url), source = new URL(sender.url);
        const atcoder = message.type === 'zoi-atcoder-submit';
        const name = atcoder ? 'AtCoder' : 'VJudge';
        if (url.origin !== source.origin || url.pathname !== source.pathname || url.search !== source.search || url.protocol !== 'https:' || !(atcoder ? ['atcoder.jp'] : ['vjudge.net', 'vjudge.net.cn']).includes(url.host)) throw Error('提交页面不匹配。');
        const request = async (route, data = {}) => {
            const response = await fetch(`http://127.0.0.1:${message.port}/${route}`, { method: 'POST', headers: { Authorization: `Bearer ${message.token}`, 'Content-Type': 'application/json' }, body: JSON.stringify({ url: message.url, ...data }), signal: AbortSignal.timeout(10000) });
            const result = await response.json();
            if (!response.ok) throw Error(result.error || '本地提交连接失败。');
            return result;
        };
        const job = await request('claim');
        try {
            const run = async stage => {
                const [result] = await chrome.scripting.executeScript({ target: { tabId: sender.tab.id }, world: 'MAIN', func: atcoder ? zoiAtcoderPage : zoiVjudgePage, args: [job, stage] });
                if (!result?.result) throw Error(`页面离开或提交脚本未完成，请检查 ${name} 记录。`);
                if (result.result.error) throw Error(result.result.error);
                return result.result;
            };
            const prepared = await run('prepare');
            const selection = { language: prepared.language, languageFallback: prepared.languageFallback === true, languageUnspecified: prepared.languageUnspecified === true };
            if (job.mode === 'check') await run('verify');
            if (job.mode !== 'check') await request('permit', selection);
            const result = job.mode === 'check' ? { prepared: true, ...selection, message: `Chrome 已连接，${prepared.language} 表单和 O2 代码快照已通过提交前校验；没有点击提交。` + (selection.languageFallback ? '未提供可用的 C++20，已自动回退。' : '') + (prepared.warning || '') } : await run('submit');
            await request('report', result);
            if (atcoder && result.ok && result.resultUrl) {
                const target = new URL(result.resultUrl);
                if (target.origin === url.origin && /^\/contests\/[\w-]+\/submissions\/\d+$/.test(target.pathname)) await chrome.tabs.update(sender.tab.id, { url: target.href }).catch(() => {});
            }
            return result;
        } catch (error) {
            await request('report', { ok: false, message: error.message }).catch(() => {});
            throw error;
        }
    })().then(reply, error => reply({ error: error.message }));
    return true;
});

function zoiHandleStatement(message, sender, reply) {
    if (message?.type !== 'zoi-statement') return;
    (async () => {
        if (!sender.tab || sender.frameId !== 0 || !Number.isInteger(message.port) || message.port < 1024 || message.port > 65535 || !/^[a-f0-9]{64}$/.test(message.token)) throw Error('无效的题面读取请求。');
        const source = new URL(sender.url), url = new URL(message.url);
        if (url.protocol !== 'https:' || !['codeforces.com', 'www.codeforces.com', 'm1.codeforces.com', 'm2.codeforces.com', 'atcoder.jp', 'vjudge.net', 'vjudge.net.cn'].includes(url.host) || source.origin !== url.origin || source.pathname !== url.pathname || source.search !== url.search) throw Error('题面页面不匹配。');
        const request = async (route, data = {}) => {
            const response = await fetch(`http://127.0.0.1:${message.port}/${route}`, { method: 'POST', headers: { Authorization: `Bearer ${message.token}`, 'Content-Type': 'application/json' }, body: JSON.stringify({ url: message.url, ...data }), signal: AbortSignal.timeout(15000) });
            const result = await response.json(); if (!response.ok) throw Error(result.error || '本地题面连接失败。'); return result;
        };
        const job = await request('claim');
        try {
            const candidates = await chrome.tabs.query({ url: url.origin + '/*' });
            const existing = candidates.find(t => {
                if (t.id === sender.tab.id || !t.url) return false;
                const candidate = new URL(t.url);
                return candidate.pathname === url.pathname && (!job.site.startsWith('vj') || !url.hash || candidate.hash === url.hash);
            });
            const [result] = await chrome.scripting.executeScript({ target: { tabId: existing?.id || sender.tab.id }, func: zoiStatementPage, args: [job] });
            if (!result?.result) throw Error('题面读取没有完成。');
            await request('report', result.result);
            // Only close the tab opened by our one-time handoff; never close a reused user tab.
            const tab = await chrome.tabs.get(sender.tab.id).catch(() => null);
            if (tab && tab.url === message.url) await chrome.tabs.remove(sender.tab.id).catch(() => {});
            return { ok: true };
        } catch (e) { await request('report', { error: e.message }).catch(() => {}); throw e; }
    })().then(reply, error => reply({ error: error.message }));
    return true;
}
