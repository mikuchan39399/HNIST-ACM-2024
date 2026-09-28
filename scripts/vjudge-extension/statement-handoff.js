'use strict';
(() => {
    const match = location.hash.match(/(?:^#|&)zoi-statement=(\d{1,5})\.([a-f0-9]{64})$/);
    if (!match) return;
    const url = location.href.slice(0, -match[0].length);
    history.replaceState(history.state, '', url);
    chrome.runtime.sendMessage({ type: 'zoi-statement', port: Number(match[1]), token: match[2], url }).catch(() => {});
})();
