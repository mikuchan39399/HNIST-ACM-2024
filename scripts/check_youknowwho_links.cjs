'use strict';
const assert = require('node:assert/strict');
const fs = require('node:fs');
const path = require('node:path');
const vm = require('node:vm');
const { zoiYouknowwhoLink: resolve, zoiYouknowwhoInstall: install } = require('./vjudge-extension/youknowwho.js');
const numbers = require('./vjudge-extension/youknowwho-data.js');
const extension = path.join(__dirname, 'vjudge-extension');

const direct = [
    ['https://cses.fi/problemset/task/1753', 'CSES-1753'],
    ['https://www.spoj.com/problems/NAJPF/', 'SPOJ-NAJPF'],
    ['https://www.codechef.com/JUNE221A/problems/ADDBYPERM', 'CodeChef-ADDBYPERM'],
    ['https://judge.yosupo.jp/problem/addition_of_big_integers', 'Yosupo-addition_of_big_integers'],
    ['https://toph.co/p/abcdefgh', 'Toph-abcdefgh'],
    ['https://oj.uz/problem/view/APIO14_sequence', 'OJUZ-APIO14_sequence'],
    ['https://dmoj.ca/problem/apio10p1', 'DMOJ-apio10p1'],
    ['https://csacademy.com/contest/archive/task/bfs-dfs/statement/', 'CSAcademy-bfs-dfs'],
    ['https://www.acmicpc.net/problem/10446', 'Baekjoon-10446'],
    ['https://loj.ac/p/138', 'LibreOJ-138'],
    ['https://uoj.ac/problem/164', 'UniversalOJ-164'],
    ['https://www.luogu.com.cn/problem/P4824', '洛谷-P4824'],
    ['https://qoj.ac/contest/1917/problem/10097', 'QOJ-10097'],
    ['https://basecamp.eolymp.com/en/problems/11100', 'EOlymp-11100'],
    ['https://www.eolymp.com/en/problems/1453', 'EOlymp-1453'],
    ['https://oj.vnoi.info/problem/icpc23_national_i', 'VNOJ-icpc23_national_i'],
    ['https://yukicoder.me/problems/no/1891', 'yukicoder-1891'],
    ['https://www.hackerrank.com/contests/5-days-of-game-theory/challenges/a-chessboard-game/problem', 'HackerRank-a-chessboard-game'],
    ['https://open.kattis.com/problems/allpairspath', 'Kattis-allpairspath'],
    ['https://nus.kattis.com/courses/CS3233/CS3233_S2_AY2122/assignments/qs926n/problems/socialdistancing', 'Kattis-socialdistancing'],
    ['https://usaco.org/index.php?page=viewproblem2&cpid=992', 'USACO-992'],
    ['https://acm.timus.ru/problem.aspx?space=1&num=1002', 'URAL-1002'],
    ['http://poj.org/problem?id=1195', 'POJ-1195'],
    ['http://acm.hdu.edu.cn/showproblem.php?pid=1695', 'HDU-1695'],
    ['https://acmp.ru/index.asp?main=task&id_task=829&locale=en', 'ACMP-829'],
    ['https://community.topcoder.com/stat?c=problem_statement&pm=12614', 'TopCoder-12614'],
    ['https://onlinejudge.org/index.php?option=onlinejudge&page=show_problem&problem=396', 'UVA-455'],
    ['https://onlinejudge.org/index.php?page=show_problem&problem=1239', 'UVA-10298'],
    ['https://uva.onlinejudge.org/external/4/455.pdf', 'UVA-455'],
    ['https://lightoj.com/problem/unlucky-strings', 'LightOJ-1268'],
    ['http://www.lightoj.com/volume_showproblem.php?problem=1268', 'LightOJ-1268'],
    ['https://www.spoj.com/problems/ORDERSET/en/', 'SPOJ-ORDERSET'],
    ['https://www.spoj.com/PT07/problems/PT07A/', 'SPOJ-PT07A'],
    ['https://www.codechef.com/LTIME87A/problems-old/CNTGRP', 'CodeChef-CNTGRP'],
    ['https://loj.ac/problem/6053', 'LibreOJ-6053'],
    ['https://uoj.ac/contest/51/problem/514', 'UniversalOJ-514'],
    ['https://usaco.org/current/index.php?page=viewproblem2&cpid=576', 'USACO-576'],
];
for (const [href, target] of direct) {
    const link = resolve(href, '1. Example', '', numbers);
    assert.equal(link.label, 'VJudge', href);
    assert.equal(decodeURIComponent(link.href), 'https://vjudge.net/problem/' + target, href);
}
for (const href of [
    'https://codeforces.com/problemset/problem/471/D',
    'https://codeforces.com/contest/2010/problem/C2',
    'https://codeforces.com/gym/100212/problem/C',
    'https://codeforces.com/group/Rilx5irOux/contest/528296/problem/N',
    'https://codeforces.com/problemsets/acmsguru/problem/99999/106',
    'https://codeforces.com/edu/course/2/lesson/2/3/practice/contest/269118/problem/A',
    'https://codeforces.com/problemset/gymProblem/100162/G',
    'https://codeforces.com/gym/101021/problem/1',
    'https://atcoder.jp/contests/abc051/tasks/abc051_b',
]) {
    const link = resolve(href + '?lang=en#test', '', '', numbers);
    assert.equal(link.href, href);
    assert.equal(link.label, href.includes('atcoder') ? 'AtCoder' : 'CF');
}
assert.equal(resolve('http://arc070.contest.atcoder.jp/tasks/arc070_c', '', '', numbers).href, 'https://atcoder.jp/contests/arc070/tasks/arc070_c');
for (const href of ['https://vjudge.net/problem/HDU-7084', 'http://www.vjudge.net.cn/problem/UVA-455']) assert.equal(resolve(href, 'P', '', numbers), null);
assert.equal(resolve('https://cses.fi/problemset/task/1753', 'P', 'VJudge', numbers), null);
for (const href of ['javascript:alert(1)', 'data:text/html,evil', 'not a url', 'https://name:secret@cses.fi/problemset/task/1', 'https://cses.fi:8888/problemset/task/1']) assert.equal(resolve(href, 'P', '', numbers), null);
for (const href of ['https://cses.fi.evil.test/problemset/task/1753', 'https://leetcode.com/problems/example/', 'https://example.org/statement.pdf', 'https://onlinejudge.org/index.php?page=show_problem&problem=999999999', 'https://lightoj.com/problem/new-problem', 'https://lightoj.com/problem/constructor']) {
    const link = resolve(href, '12. A & B # 中文', '', numbers);
    assert.equal(link.label, 'VJudge 搜索');
    const params = new URLSearchParams(new URL(link.href).hash.slice(1));
    assert.equal(params.get('title'), 'A & B # 中文');
    assert.equal(params.get('category'), 'all');
    assert.equal(params.get('source'), '');
    assert.equal(params.get('probNum'), '');
}
assert.ok(Object.keys(numbers.uva).length > 4900);
assert.ok(Object.keys(numbers.lightoj).length > 400);
for (const values of Object.values(numbers)) for (const [id, number] of Object.entries(values)) {
    assert.match(id, /^[\w-]+$/);
    assert.ok(Number.isSafeInteger(number) && number > 0);
}

// DOM contract from the live topic list: header names distinguish problem rows from resources.
function fixture() {
    const marker = '[data-zoi-ykw-link]', allBadges = new Set(), callbacks = [];
    let observing = false, notify;
    const cell = (text = '') => ({ textContent: text, badges: [], querySelector(selector) {
        return selector === marker ? this.badges[0] || null : this.anchor || null;
    } });
    const row = (href, title, source) => {
        const problem = cell();
        problem.anchor = { href, textContent: title, after(badge) { problem.badges.push(badge); allBadges.add(badge); badge.parent = problem; assert.equal(observing, false); } };
        return { cells: [cell('#'), problem, cell(source)] };
    };
    const table = (headers, rows) => ({ tHead: { rows: [{ cells: headers.map(cell) }] }, tBodies: [{ rows }] });
    const resource = table(['Tag', 'Title', 'Source/Credit'], [row(direct[0][0], 'Resource', 'CSES')]);
    const problems = table(['#', 'Problem', 'Source'], [row(direct[0][0], '1. String Matching', 'CSES'), row('https://vjudge.net/problem/HDU-7084', '2. Pty', 'VJudge')]);
    const doc = { body: {}, tables: [resource, problems], querySelectorAll(selector) { return selector === 'table' ? this.tables : [...allBadges]; }, createElement(tag) {
        assert.equal(tag, 'a');
        return { style: {}, attributes: {}, setAttribute(k, v) { this.attributes[k] = v; }, remove() { allBadges.delete(this); this.parent.badges.splice(this.parent.badges.indexOf(this), 1); } };
    } };
    const Observer = class { constructor(fn) { notify = fn; } disconnect() { observing = false; } observe(_, config) { assert.deepEqual(config.attributeFilter, ['href']); observing = true; } };
    const api = install(doc, numbers, Observer, fn => callbacks.push(fn));
    return { api, doc, problems, resource, row, allBadges, callbacks, notify: () => notify() };
}
const f = fixture();
assert.equal(f.allBadges.size, 1);
assert.equal(f.resource.tBodies[0].rows[0].cells[1].badges.length, 0);
const badge = [...f.allBadges][0];
assert.equal(badge.href, 'https://vjudge.net/problem/CSES-1753');
assert.equal(badge.target, '_blank');
assert.equal(badge.rel, 'noopener noreferrer');
f.api.refresh(); assert.equal(f.allBadges.size, 1); assert.equal([...f.allBadges][0], badge);
f.problems.tBodies[0].rows[0].cells[1].anchor.href = 'https://codeforces.com/contest/625/problem/B';
f.notify(); f.notify(); assert.equal(f.callbacks.length, 1); f.callbacks.shift()();
assert.equal(f.allBadges.size, 0); // CF already has its original link.
f.problems.tBodies[0].rows.push(f.row('https://atcoder.jp/contests/abc477/tasks/abc477_a', 'New', 'AtCoder'));
f.notify(); f.callbacks.shift()(); assert.equal(f.allBadges.size, 0);
f.problems.tBodies[0].rows[0].cells[1].anchor.href = 'https://vjudge.net/problem/UVA-455';
f.notify(); f.callbacks.shift()(); assert.equal(f.allBadges.size, 0);
f.doc.tables = [f.resource]; f.api.refresh(); assert.equal(f.allBadges.size, 0);

const manifest = JSON.parse(fs.readFileSync(path.join(extension, 'manifest.json'), 'utf8'));
const config = manifest.content_scripts.find(s => s.matches.includes('https://youkn0wwho.academy/topic-list*'));
assert.deepEqual(config.js, ['youknowwho-data.js', 'youknowwho.js']);
assert.equal(config.run_at, 'document_idle');
const browser = { document: f.doc, MutationObserver: class { disconnect() {} observe() {} }, setTimeout, URL, URLSearchParams };
for (const file of config.js) vm.runInNewContext(fs.readFileSync(path.join(extension, file), 'utf8'), browser, { filename: file });
assert.ok(!manifest.host_permissions.some(host => host.includes('lightoj') || host.includes('onlinejudge')));
console.log('PASS YOUKNOWWHO links: ' + direct.length + ' judge routes, CF/AtCoder, UVA/LightOJ IDs, search fallback, DOM refresh/deduplication, manifest and browser scripts');
