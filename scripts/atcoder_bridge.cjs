'use strict';
const browser = require('./vjudge_bridge.cjs');

function problemUrl(value) {
    const u = new URL(value);
    if (u.protocol !== 'https:' || u.host !== 'atcoder.jp' || u.username || u.password || !/^\/contests\/[a-zA-Z0-9_-]+\/tasks\/[a-zA-Z0-9_-]+\/?$/.test(u.pathname)) throw Error('请用 Competitive Companion 从 AtCoder 的具体题目页面拉题。');
    u.pathname = u.pathname.replace(/\/$/, ''); u.search = ''; u.hash = '';
    return u.href;
}
function isAtcoder(value) { try { problemUrl(String(value)); return true; } catch { return false; } }
function submissionUrl(value) {
    const u = new URL(problemUrl(value)), parts = u.pathname.split('/');
    u.pathname = `/contests/${parts[2]}/submit`; u.searchParams.set('taskScreenName', parts[4]);
    return u.href;
}
module.exports = { problemUrl, isAtcoder, submissionUrl,
    submit: (vscode, problem, prepareDocument) => browser.submit(vscode, problem, prepareDocument, { name: 'AtCoder', url: submissionUrl(problem.url), maxBytes: 512 * 1024 }) };
