'use strict';
// Remove the handoff fragment before VJudge's contest router reads it.
(() => {
    const notify = message => {
        const show = () => {
            const notice = document.createElement('div'), close = document.createElement('button');
            notice.setAttribute('role', 'alert');
            notice.style.cssText = 'position:fixed;top:12px;right:12px;max-width:520px;padding:16px;background:#fff;color:#8b2020;border:1px solid #8b2020;border-radius:8px;box-shadow:0 2px 12px #0003;z-index:2147483647';
            notice.textContent = message + ' ';
            close.type = 'button'; close.textContent = '关闭'; close.onclick = () => notice.remove();
            notice.append(close); document.body.append(notice);
        };
        if (document.body) show();
        else document.addEventListener('DOMContentLoaded', show, { once: true });
    };
    const match = location.hash.match(/(?:^#|&)zoi-submit=(\d{1,5})\.([a-f0-9]{64})$/);
    if (!match) return;
    const url = location.href.slice(0, location.href.length - match[0].length);
    history.replaceState(history.state, '', url);
    chrome.runtime.sendMessage({ type: 'zoi-vjudge-submit', port: Number(match[1]), token: match[2], url }).then(result => {
        if (result?.error) notify('ZOI VJudge：' + result.error);
    }).catch(() => notify('ZOI VJudge 扩展连接中断，请先检查网页提交记录再重试。'));
})();
