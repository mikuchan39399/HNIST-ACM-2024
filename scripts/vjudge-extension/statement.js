'use strict';
function zoiStatementLocation(target, current) {
    const a = new URL(target), b = new URL(current);
    if (a.origin !== b.origin || b.username || b.password) return false;
    if (a.pathname === b.pathname && a.search === b.search) return true;
    const gym = a.pathname.match(/^\/gym\/(\d+)\/problem\/[A-Za-z0-9]+\/?$/) || a.pathname.match(/^\/problemset\/gymProblem\/(\d+)\/[A-Za-z0-9]+\/?$/);
    return !!gym && ['codeforces.com', 'www.codeforces.com', 'm1.codeforces.com', 'm2.codeforces.com'].includes(a.hostname) && b.pathname === '/gym/' + gym[1] + '/attachments' && !b.search;
}
async function zoiStatementPage(job) {
    const target = new URL(job.url);
    const gym = job.site === 'cf' && (target.pathname.match(/^\/gym\/(\d+)\/problem\/[A-Za-z0-9]+\/?$/) || target.pathname.match(/^\/problemset\/gymProblem\/(\d+)\/[A-Za-z0-9]+\/?$/));
    const until = Date.now() + 90000;
    while (Date.now() < until) {
        if (location.origin !== target.origin || (location.pathname !== target.pathname && !(gym && location.pathname === '/gym/' + gym[1] + '/attachments'))) throw Error('题目页面已切换，请重新读取。');
        if (job.site === 'vj-search') {
            if (location.hash !== new URL(job.url).hash) throw Error('VJudge 搜索条件已变化，请重新导入。');
            const results = Array.from(document.querySelectorAll('table tbody a[href]')).filter(a => /^\/problem\/(?:[A-Za-z0-9_]+|洛谷)-[A-Za-z0-9_.-]+$/.test(decodeURIComponent(new URL(a.href).pathname))).map(a => ({ url: a.href, label: (a.closest('tr')?.textContent || a.textContent).trim().replace(/\s+/g, ' ').slice(0, 200) }));
            if (results.length) return { results: [...new Map(results.map(r => [r.url, r])).values()].slice(0, 50) };
            if (document.querySelector('.dataTables_empty') && !document.querySelector('.dataTables_processing:not([style*="none"])')) throw Error('VJudge 没有找到对应题目，请核对题名或收录情况。');
        } else if (job.site === 'vj') {
            if (new URL(job.url).hash.startsWith('#problem/') && location.hash !== new URL(job.url).hash) throw Error('VJudge 比赛题号已切换，请重新读取。');
            const choices = Array.from(document.querySelectorAll('#prob-descs .problem-description-item')).map(item => {
                const a = item.querySelector('a.js-desc-open');
                return { url: a?.href, label: (item.querySelector('.statement-author-row')?.textContent || item.dataset.authorName || '题面').trim().replace(/\s+/g, ' ') + ' · ' + (item.querySelector('time')?.textContent || '').trim(), selected: item.classList.contains('active') };
            }).filter(x => x.url);
            if (choices.length) return { choices };
        } else {
            const root = document.querySelector(job.site === 'cf' ? '.problem-statement' : '#task-statement');
            if (root?.textContent.trim()) return { html: root.outerHTML };
            if (gym) {
                const links = Array.from(document.querySelectorAll('a[href]')).filter(a => {
                    const u = new URL(a.href);
                    return u.origin === target.origin && u.pathname.startsWith('/gym/' + gym[1] + '/attachments/download/') && /\.pdf$/i.test(u.pathname);
                });
                if (links.length) return { html: links.map(a => a.outerHTML).join('\n') };
            }
        }
        await new Promise(resolve => setTimeout(resolve, 300));
    }
    throw Error('没有找到题面。请完成网页登录或验证，再从 VS Code 重试。');
}
if (typeof module === 'object' && module.exports) module.exports = { zoiStatementPage, zoiStatementLocation };
