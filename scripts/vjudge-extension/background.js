'use strict';
importScripts('page.js');

chrome.runtime.onMessage.addListener((message, sender, reply) => {
    if (message?.type !== 'zoi-vjudge-submit') return;
    (async () => {
        if (!sender.tab || sender.frameId !== 0 || !Number.isInteger(message.port) || message.port < 1024 || message.port > 65535 || !/^[a-f0-9]{64}$/.test(message.token)) throw Error('无效的提交请求。');
        const url = new URL(message.url), source = new URL(sender.url);
        if (url.origin !== source.origin || url.pathname !== source.pathname || url.protocol !== 'https:' || !['vjudge.net', 'vjudge.net.cn'].includes(url.host)) throw Error('提交页面不匹配。');
        const request = async (route, data = {}) => {
            const response = await fetch(`http://127.0.0.1:${message.port}/${route}`, { method: 'POST', headers: { Authorization: `Bearer ${message.token}`, 'Content-Type': 'application/json' }, body: JSON.stringify({ url: message.url, ...data }), signal: AbortSignal.timeout(10000) });
            const result = await response.json();
            if (!response.ok) throw Error(result.error || '本地提交连接失败。');
            return result;
        };
        const job = await request('claim');
        try {
            const run = async stage => {
                const [result] = await chrome.scripting.executeScript({ target: { tabId: sender.tab.id }, world: 'MAIN', func: zoiVjudgePage, args: [job, stage] });
                if (!result?.result) throw Error('页面离开或提交脚本未完成，请检查 VJudge 记录。');
                if (result.result.error) throw Error(result.result.error);
                return result.result;
            };
            const prepared = await run('prepare');
            const selection = { language: prepared.language, languageFallback: prepared.languageFallback === true };
            if (job.mode === 'check') await run('verify');
            if (job.mode !== 'check') await request('permit', selection);
            const result = job.mode === 'check' ? { prepared: true, ...selection, message: `Chrome 已连接，${prepared.language} 表单和 O2 代码快照已通过提交前校验；没有点击提交。` + (selection.languageFallback ? '未提供 GNU C++20，已自动回退。' : '') + (prepared.warning || '') } : await run('submit');
            await request('report', result);
            return result;
        } catch (error) {
            await request('report', { ok: false, message: error.message }).catch(() => {});
            throw error;
        }
    })().then(reply, error => reply({ error: error.message }));
    return true;
});
