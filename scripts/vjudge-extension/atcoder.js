'use strict';

// Serialized into MAIN world. Selectors/editor sync follow AtCoder's public contest.js.
async function zoiAtcoderPage(job, stage) {
    let sent = false;
    const normalize = text => text.replace(/\r\n?/g, '\n');
    const wait = async (get, message) => {
        const end = Date.now() + 15000;
        do { const value = get(); if (value) return value; await new Promise(resolve => setTimeout(resolve, 100)); } while (Date.now() < end);
        throw Error(message);
    };
    try {
        const expected = new URL(job.url), contest = expected.pathname.match(/^\/contests\/([\w-]+)\/submit$/)?.[1];
        const task = expected.searchParams.get('taskScreenName');
        if (expected.origin !== 'https://atcoder.jp' || !contest || !task || !/^[\w-]+$/.test(task)) throw Error('AtCoder 提交地址无效。');
        const verifyUrl = () => { if (location.origin !== expected.origin || location.pathname !== expected.pathname || new URL(location.href).searchParams.get('taskScreenName') !== task) throw Error('页面题目发生变化，停止提交。'); };
        verifyUrl();
        const form = await wait(() => document.querySelector('form.form-code-submit'), '未找到 AtCoder 提交表单，请先登录并检查参赛权限。');
        const action = () => new URL(form.action, location.href);
        const verifyAction = () => { if (action().origin !== expected.origin || action().pathname !== expected.pathname || form.method.toLowerCase() !== 'post') throw Error('AtCoder 表单提交地址发生变化。'); };
        verifyAction();
        const selectTask = form.querySelector('[name="data.TaskScreenName"]');
        if (!selectTask || ![...selectTask.options].some(o => o.value === task && !o.disabled && !o.parentElement?.disabled)) throw Error('AtCoder 表单中没有当前题目。');
        const change = e => { if (window.jQuery) window.jQuery(e).trigger('change'); else e.dispatchEvent(new Event('change', { bubbles: true })); };
        if (stage === 'prepare') { selectTask.value = task; change(selectTask); }
        const language = await wait(() => { const e = form.querySelector('select[name="data.LanguageId"]'); return e?.options.length && e; }, 'AtCoder 未加载编译器列表。');
        const choices = [...language.options].flatMap(option => {
            const label = option.textContent.trim(), version = label.match(/^(?:GNU\s*)?(?:C\+\+|G\+\+)\s*(0x|11|1y|14|1z|17|2a|20|2b|23|2c|26)(?![\w.])/i)?.[1].toLowerCase();
            const aliases = { '0x': 2011, '1y': 2014, '1z': 2017, '2a': 2020, '2b': 2023, '2c': 2026 };
            const standard = version ? (aliases[version] || 2000 + Number(version)) : 0;
            if (!option.value || option.disabled || option.parentElement?.disabled || !standard || !/gcc|g\+\+|gnu/i.test(label) || /clang|msvc|visual|ioi/i.test(label)) return [];
            return [{ option, label, standard }];
        });
        choices.sort((a, b) => Number(b.standard === 2020) - Number(a.standard === 2020) || b.standard - a.standard);
        const choice = choices[0];
        if (!choice) throw Error('AtCoder 没有可用的普通 GNU C++ 编译器，未提交。');
        const textarea = form.querySelector('textarea[name="sourceCode"]');
        if (!textarea) throw Error('AtCoder 代码输入框发生变化。');
        const editor = window.jQuery?.(textarea).data('editor');
        const plainVisible = () => textarea.getClientRects().length > 0;
        if (!plainVisible() && !editor) throw Error('AtCoder 编辑器尚未就绪，请刷新后重试。');
        const readCode = () => plainVisible() ? textarea.value : editor.getValue();
        if (stage === 'prepare') {
            language.value = choice.option.value; change(language);
            editor?.setValue(normalize(job.code), -1); textarea.value = normalize(job.code);
            textarea.dispatchEvent(new Event('input', { bubbles: true }));
            return { prepared: true, language: choice.label, languageFallback: choice.standard !== 2020 };
        }
        const preflight = () => {
            verifyUrl(); verifyAction();
            if (!form.isConnected || selectTask.value !== task) throw Error('AtCoder 题号发生变化，停止提交。');
            if (language.value !== choice.option.value) throw Error('提交编译器发生变化，停止提交。');
            if (normalize(readCode()) !== normalize(job.code)) throw Error('网页中的代码内容发生变化，停止提交。');
        };
        preflight();
        if (stage === 'verify') return { verified: true };
        if (stage !== 'submit' || job.mode === 'check') throw Error('连接检查不能发送提交。');
        const button = form.querySelector('#submit');
        if (!button || button.disabled) throw Error('AtCoder 提交按钮尚不可用。');
        const humanCheck = form.querySelector('[name="cf-turnstile-response"]');
        if (humanCheck && !humanCheck.value) throw Error('AtCoder 要求网页验证，请在网页完成验证后手动提交。');
        const listUrl = `${expected.origin}/contests/${contest}/submissions/me`;
        const readHtml = async url => {
            const response = await fetch(url, { credentials: 'same-origin', cache: 'no-store', signal: AbortSignal.timeout(20000) });
            if (!response.ok || response.url !== url) throw Error('无法读取 AtCoder 提交记录，请检查登录与网页验证。');
            return new DOMParser().parseFromString(await response.text(), 'text/html');
        };
        const ids = doc => [...doc.querySelectorAll('a[href]')].map(a => new URL(a.getAttribute('href'), expected.origin)).filter(u => u.origin === expected.origin && new RegExp(`^/contests/${contest}/submissions/\\d+$`).test(u.pathname)).map(u => u.pathname.split('/').pop());
        const before = new Set(ids(await readHtml(listUrl)));
        preflight(); // The user may edit the form while the GET is in flight.
        // Let native validation/editor handlers run; send that form once with its own CSRF/CAPTCHA.
        const response = await new Promise((resolve, reject) => {
            const timer = setTimeout(() => { form.removeEventListener('submit', capture); reject(Error('原生表单未提交，请检查网页提示。')); }, 2000);
            function capture(event) {
                clearTimeout(timer); form.removeEventListener('submit', capture);
                if (event.defaultPrevented) { reject(Error('AtCoder 阻止了本次提交，请检查网页提示。')); return; }
                event.preventDefault();
                try {
                    preflight();
                    const body = new URLSearchParams(new FormData(form));
                    if (body.get('data.TaskScreenName') !== task || body.get('data.LanguageId') !== choice.option.value || normalize(body.get('sourceCode') || '') !== normalize(job.code) || !body.get('csrf_token')) throw Error('AtCoder 表单数据不完整或发生变化，未提交。');
                    if (body.has('cf-turnstile-response') && !body.get('cf-turnstile-response')) throw Error('请先完成 AtCoder 网页验证。');
                    sent = true;
                    resolve(fetch(action().href, { method: 'POST', body, credentials: 'same-origin', signal: AbortSignal.timeout(20000) }));
                } catch (error) { reject(error); }
            }
            form.addEventListener('submit', capture);
            try { button.click(); } catch (error) { clearTimeout(timer); form.removeEventListener('submit', capture); reject(error); }
        });
        if (!response.ok || response.url !== listUrl) throw Error('AtCoder 未确认接收，请检查网页提交记录；工具不会自动重试。');
        const resultDoc = new DOMParser().parseFromString(await response.text(), 'text/html');
        const fresh = [...new Set(ids(resultDoc))].filter(id => !before.has(id));
        if (fresh.length !== 1) throw Error('无法唯一确认 AtCoder 提交编号，请先检查提交记录；工具不会自动重试。');
        const resultUrl = `${expected.origin}/contests/${contest}/submissions/${fresh[0]}`;
        const detail = await readHtml(resultUrl);
        const source = detail.querySelector('#submission-code');
        if (!source || normalize(source.textContent) !== normalize(job.code) || !detail.querySelector(`a[href="/contests/${contest}/tasks/${task}"]`)) throw Error('AtCoder 提交记录与本次代码或题号不一致，请检查网页；工具不会自动重试。');
        return { ok: true, runId: fresh[0], resultUrl };
    } catch (error) { return { error: error.message + (sent ? ' 提交请求已发出，请先检查 AtCoder 记录，工具不会自动重试。' : '') }; }
}
if (typeof module !== 'undefined') module.exports = { zoiAtcoderPage };
