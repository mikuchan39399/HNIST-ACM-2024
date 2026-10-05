'use strict';
(() => {
    const match = location.hash.match(/(?:^#|&)zoi-statement=(\d{1,5})\.([a-f0-9]{64})(?:&zoi-problem=([^&]+))?$/);
    if (!match) return;
    const clean = location.href.slice(0, -match[0].length);
    const url = match[3] ? decodeURIComponent(match[3]) : clean;
    history.replaceState(history.state, '', clean);
    chrome.runtime.sendMessage({ type: 'zoi-statement', port: Number(match[1]), token: match[2], url }).catch(() => {});
})();
