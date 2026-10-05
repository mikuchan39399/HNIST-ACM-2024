'use strict';

function zoiYouknowwhoLink(href, title, source, numbers) {
    let url;
    try { url = new URL(href); } catch { return null; }
    if (!['https:', 'http:'].includes(url.protocol) || url.username || url.password || url.port) return null;
    const host = url.hostname.replace(/^www\./, ''), path = url.pathname;
    if (['vjudge.net', 'vjudge.net.cn'].includes(host) || /^vjudge$/i.test(source.trim())) return null;
    const original = label => ({ label, href: 'https://' + url.host + path, title: '打开 ' + label + ' 原题' });
    // Problem tables also contain EDU exercises and old Gym URLs; preserve their own route.
    const cf = ['codeforces.com', 'm1.codeforces.com', 'm2.codeforces.com'].includes(host);
    if (cf && /^\/(?:problemset\/(?:problem|gymProblem)\/\d+\/[A-Za-z0-9]+|problemsets\/acmsguru\/problem\/\d+\/\d+|(?:contest|gym)\/\d+\/problem\/[A-Za-z0-9]+|group\/[A-Za-z0-9]+\/contest\/\d+\/problem\/[A-Za-z0-9]+|edu\/course\/\d+\/lesson\/\d+\/\d+\/practice\/contest\/\d+\/problem\/[A-Za-z0-9]+)\/?$/.test(path)) return original('CF');
    if (host === 'atcoder.jp' && /^\/contests\/[\w-]+\/tasks\/[\w-]+\/?$/.test(path)) return original('AtCoder');
    const legacyAtcoder = host.match(/^([\w-]+)\.contest\.atcoder\.jp$/);
    if (legacyAtcoder && /^\/tasks\/[\w-]+\/?$/.test(path)) return { label: 'AtCoder', href: 'https://atcoder.jp/contests/' + legacyAtcoder[1] + path, title: '打开 AtCoder 原题' };

    const vj = (oj, id) => /^[A-Za-z0-9_-]+$/.test(id || '') ? { label: 'VJudge', href: 'https://vjudge.net/problem/' + encodeURIComponent(oj + '-' + id), title: '打开 ' + oj + ' ' + id + ' 的 VJudge 题页' } : null;
    let match;
    const rules = [
        ['cses.fi', /^\/(?:problemset\/)?task\/(\d+)\/?$/, 'CSES'],
        ['spoj.com', /^\/(?:[\w-]+\/)?problems\/([A-Za-z0-9_]+)(?:\/[a-z]{2})?\/?$/, 'SPOJ'],
        ['codechef.com', /^\/(?:[\w-]+\/)?problems(?:-old)?\/([A-Za-z0-9_]+)\/?$/, 'CodeChef'],
        ['judge.yosupo.jp', /^\/problem\/([\w-]+)\/?$/, 'Yosupo'],
        ['toph.co', /^\/p\/([\w-]+)\/?$/, 'Toph'],
        ['oj.uz', /^\/problem\/view\/([\w-]+)\/?$/, 'OJUZ'],
        ['dmoj.ca', /^\/problem\/([\w-]+)\/?$/, 'DMOJ'],
        ['csacademy.com', /^\/contest\/[\w-]+\/task\/([\w-]+)(?:\/statement)?\/?$/, 'CSAcademy'],
        ['acmicpc.net', /^\/problem\/(\d+)\/?$/, 'Baekjoon'],
        ['loj.ac', /^\/(?:p|problem)\/(\d+)\/?$/, 'LibreOJ'],
        ['uoj.ac', /^\/(?:contest\/\d+\/)?problem\/(\d+)\/?$/, 'UniversalOJ'],
        ['luogu.com.cn', /^\/problem\/([A-Za-z0-9_]+)\/?$/, '洛谷'],
        ['qoj.ac', /^\/(?:contest\/\d+\/)?problem\/(\d+)\/?$/, 'QOJ'],
        ['eolymp.com', /^\/(?:[a-z]{2}\/)?problems\/(\d+)\/?$/, 'EOlymp'],
        ['basecamp.eolymp.com', /^\/(?:[a-z]{2}\/)?problems\/(\d+)\/?$/, 'EOlymp'],
        ['oj.vnoi.info', /^\/problem\/([\w-]+)\/?$/, 'VNOJ'],
        ['yukicoder.me', /^\/problems\/no\/(\d+)\/?$/, 'yukicoder'],
        ['hackerrank.com', /^\/(?:contests\/[\w-]+\/)?challenges\/([\w-]+)(?:\/problem)?\/?$/, 'HackerRank'],
    ];
    for (const [domain, pattern, oj] of rules) if (host === domain && (match = path.match(pattern))) return vj(oj, match[1]);
    if ((host === 'kattis.com' || host.endsWith('.kattis.com')) && (match = path.match(/\/problems\/([\w-]+)\/?$/))) return vj('Kattis', match[1]);
    const numeric = (oj, key) => /^\d+$/.test(url.searchParams.get(key) || '') ? vj(oj, url.searchParams.get(key)) : null;
    if (host === 'usaco.org' && /^\/(?:current\/)?index\.php$/.test(path) && url.searchParams.get('page') === 'viewproblem2') return numeric('USACO', 'cpid');
    if (host === 'acm.timus.ru' && path === '/problem.aspx' && (!url.searchParams.has('space') || url.searchParams.get('space') === '1')) return numeric('URAL', 'num');
    if (host === 'poj.org' && path === '/problem') return numeric('POJ', 'id');
    if (host === 'acm.hdu.edu.cn' && path === '/showproblem.php') return numeric('HDU', 'pid');
    if (host === 'acmp.ru' && path === '/index.asp' && url.searchParams.get('main') === 'task') return numeric('ACMP', 'id_task');
    if (['community.topcoder.com', 'arena.topcoder.com', 'archive.topcoder.com'].includes(host) && /^\/(?:stat|ProblemStatement\/pm\/\d+)\/?$/.test(path)) {
        const id = path.match(/\/pm\/(\d+)/)?.[1] || url.searchParams.get('pm');
        if (/^\d+$/.test(id || '')) return vj('TopCoder', id);
    }
    if (['onlinejudge.org', 'uva.onlinejudge.org'].includes(host)) {
        if (path === '/index.php' && url.searchParams.get('page') === 'show_problem') {
            const key = url.searchParams.get('problem');
            const id = Object.hasOwn(numbers.uva, key) ? numbers.uva[key] : null;
            if (id) return vj('UVA', String(id));
        }
        if ((match = path.match(/^\/external\/\d+\/(\d+)\.pdf$/))) return vj('UVA', match[1]);
    }
    if (host === 'lightoj.com') {
        if ((match = path.match(/^\/problem\/([\w-]+)\/?$/))) {
            const id = /^\d+$/.test(match[1]) ? match[1] : Object.hasOwn(numbers.lightoj, match[1]) ? numbers.lightoj[match[1]] : null;
            if (id) return vj('LightOJ', String(id));
        }
        if (path === '/volume_showproblem.php') return numeric('LightOJ', 'problem');
    }

    const cleanTitle = title.replace(/^\s*\d+\s*[.、]\s*/, '').trim();
    if (!cleanTitle) return null;
    // VJudge stores list filters in the fragment; clear every filter to avoid inheriting a previous search.
    const oj = ['onlinejudge.org', 'uva.onlinejudge.org'].includes(host) ? 'UVA' : host === 'lightoj.com' ? 'LightOJ' : cf && /^\/gym\//.test(path) ? 'Gym' : 'All';
    return { label: 'VJudge 搜索', href: 'https://vjudge.net/problem#OJId=' + oj + '&probNum=&title=' + encodeURIComponent(cleanTitle) + '&source=&category=all', title: '尚无可靠题号映射，按题名在 VJudge 搜索（可能未收录）' };
}

function zoiYouknowwhoInstall(document, numbers, Observer, schedule) {
    const marker = 'data-zoi-ykw-link';
    const selector = '[' + marker + ']';
    let pending = false;
    const observer = new Observer(() => {
        if (!pending) { pending = true; schedule(refresh); }
    });
    function refresh() {
        pending = false;
        observer.disconnect();
        try {
            const stale = new Set(document.querySelectorAll(selector));
            for (const table of document.querySelectorAll('table')) {
                const headers = Array.from(table.tHead?.rows[0]?.cells || [], cell => cell.textContent.trim().toLowerCase());
                const problemColumn = headers.indexOf('problem'), sourceColumn = headers.indexOf('source');
                if (problemColumn < 0 || sourceColumn < 0) continue;
                for (const body of table.tBodies) for (const row of body.rows) {
                    const cell = row.cells[problemColumn], source = row.cells[sourceColumn]?.textContent || '';
                    const anchor = cell?.querySelector('a[href]:not(' + selector + ')');
                    if (!anchor) continue;
                    const link = zoiYouknowwhoLink(anchor.href, anchor.textContent, source, numbers);
                    if (!link || ['CF', 'AtCoder'].includes(link.label)) continue;
                    let badge = cell.querySelector(selector);
                    if (!badge) {
                        badge = document.createElement('a');
                        badge.setAttribute(marker, '');
                        badge.target = '_blank';
                        badge.rel = 'noopener noreferrer';
                        badge.style.cssText = 'display:inline-block;flex-shrink:0;margin-inline-start:0.5em;padding:0.15em 0.6em;border:1px solid rgba(255,255,255,0.3);border-radius:0.4em;font-size:0.75em;font-weight:700;line-height:1.6;white-space:nowrap;text-decoration:none;color:#fff;box-shadow:0 1px 2px rgba(0,0,0,0.18)';
                        anchor.after(badge);
                    }
                    if (badge.href !== link.href) badge.href = link.href;
                    if (badge.textContent !== link.label) badge.textContent = link.label;
                    if (badge.title !== link.title) { badge.title = link.title; badge.setAttribute('aria-label', link.title); }
                    if (badge.zoiLabel !== link.label) { badge.style.backgroundColor = { VJudge: '#047857', 'VJudge 搜索': '#475569' }[link.label]; badge.zoiLabel = link.label; }
                    stale.delete(badge);
                }
            }
            for (const badge of stale) badge.remove();
        } finally {
            observer.observe(document.body, { childList: true, subtree: true, characterData: true, attributes: true, attributeFilter: ['href'] });
        }
    }
    refresh();
    return { refresh, disconnect: () => observer.disconnect() };
}

function zoiYouknowwhoImport(href, title, source, numbers) {
    let target;
    try {
        const url = new URL(href);
        if (['vjudge.net', 'vjudge.net.cn'].includes(url.hostname) && ['https:', 'http:'].includes(url.protocol) && !url.port && !url.username && !url.password) {
            if (/^\/problem\/(?:[A-Za-z0-9_]+|洛谷)-[A-Za-z0-9_.-]+\/?$/.test(decodeURIComponent(url.pathname))) { url.protocol = 'https:'; url.search = ''; url.hash = ''; target = url.href; }
            else if (/^\/contest\/\d+$/.test(url.pathname) && /^#problem\/[A-Z][A-Z0-9]*$/.test(url.hash)) { url.protocol = 'https:'; url.search = ''; target = url.href; }
        }
    } catch { return null; }
    if (!target) {
        const link = zoiYouknowwhoLink(href, title, source, numbers);
        if (!link) return null;
        target = link.href;
    }
    return 'vscode://zoi-local.zoi-statement/import?' + new URLSearchParams({ url: target, title: title.trim().slice(0, 160) });
}
function zoiYouknowwhoImportInstall(document, numbers, Observer, schedule) {
    const marker = 'data-zoi-ykw-import'; let pending = false;
    const observer = new Observer(() => { if (!pending) { pending = true; schedule(refresh); } });
    function refresh() {
        pending = false; observer.disconnect();
        try {
            const stale = new Set(document.querySelectorAll('[' + marker + ']'));
            for (const table of document.querySelectorAll('table')) {
                const heads = Array.from(table.tHead?.rows[0]?.cells || [], c => c.textContent.trim().toLowerCase());
                const pi = heads.indexOf('problem'), si = heads.indexOf('source'); if (pi < 0 || si < 0) continue;
                for (const body of table.tBodies) for (const row of body.rows) {
                    const cell = row.cells[pi], source = row.cells[si]?.textContent || '';
                    const anchor = cell?.querySelector('a[href]:not([data-zoi-ykw-link]):not([' + marker + '])'); if (!anchor) continue;
                    const href = zoiYouknowwhoImport(anchor.href, anchor.textContent, source, numbers); if (!href) continue;
                    let button = cell.querySelector('[' + marker + ']');
                    if (!button) { button = document.createElement('a'); button.setAttribute(marker, ''); button.textContent = 'VS Code'; button.style.cssText = 'display:inline-block;margin-inline-start:.5em;padding:.15em .6em;border-radius:.4em;background:#145caa;color:#fff;font-size:.75em;font-weight:700;text-decoration:none;white-space:nowrap'; anchor.after(button); }
                    if (button.href !== href) button.href = href;
                    if (!button.title) { button.title = '导入题目和样例到 VS Code，并在右侧打开题面'; button.setAttribute('aria-label', button.title); }
                    stale.delete(button);
                }
            }
            for (const item of stale) item.remove();
        } finally { observer.observe(document.body, { childList: true, subtree: true, characterData: true, attributes: true, attributeFilter: ['href'] }); }
    }
    refresh(); return { refresh, disconnect: () => observer.disconnect() };
}
if (typeof module === 'object' && module.exports) module.exports = { zoiYouknowwhoLink, zoiYouknowwhoInstall, zoiYouknowwhoImport, zoiYouknowwhoImportInstall };
else {
    zoiYouknowwhoInstall(document, zoiYouknowwhoNumbers, MutationObserver, callback => setTimeout(callback, 50));
    zoiYouknowwhoImportInstall(document, zoiYouknowwhoNumbers, MutationObserver, callback => setTimeout(callback, 50));
}
