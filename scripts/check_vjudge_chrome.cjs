'use strict';
// Interactive smoke check: use the real extension and page, but never submit to an OJ.
const { createSession, openChrome, problemUrl } = require('./vjudge_bridge.cjs');
(async () => {
    const args = process.argv.slice(2), manual = args.includes('--manual');
    const url = problemUrl(args.find(value => value !== '--manual') || 'https://vjudge.net/problem/CodeForces-1A');
    const session = await createSession({ url, mode: 'check', code: '#pragma GCC optimize("O2")\r\n// ZOI connection check only. Do not submit.\r\nint main() { return 0; }\r\n' }, () => {});
    try {
        if (manual) console.log('OPEN: ' + session.url);
        else await openChrome(session.url);
        const result = await session.done;
        console.log((result.prepared ? 'PASS: ' : 'FAIL: ') + result.message);
        if (!result.prepared) process.exitCode = 1;
    } finally { session.cancel(); }
})().catch(error => { console.error(error.message); process.exitCode = 1; });
