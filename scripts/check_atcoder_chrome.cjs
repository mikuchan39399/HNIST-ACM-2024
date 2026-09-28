'use strict';
// Real form preflight only: the loopback session refuses all submission permits.
const { createSession, openChrome } = require('./vjudge_bridge.cjs');
const { submissionUrl } = require('./atcoder_bridge.cjs');
(async () => {
    const args = process.argv.slice(2), target = args.find(arg => arg !== '--manual');
    if (!target) throw Error('用法：node scripts/check_atcoder_chrome.cjs <AtCoder 题目 URL> [--manual]');
    const session = await createSession({ url: submissionUrl(target), mode: 'check', code: '#pragma GCC optimize("O2")\r\n// ZOI connection check only. Do not submit.\r\nint main() { return 0; }\r\n' }, () => {}, { name: 'AtCoder' });
    try {
        if (args.includes('--manual')) console.log('OPEN: ' + session.url);
        else await openChrome(session.url);
        const result = await session.done;
        console.log((result.prepared ? 'PASS: ' : 'FAIL: ') + result.message);
        if (!result.prepared) process.exitCode = 1;
    } finally { session.cancel(); }
})().catch(error => { console.error(error.message); process.exitCode = 1; });
