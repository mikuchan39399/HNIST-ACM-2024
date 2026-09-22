'use strict';

// Self-contained because Chrome serializes this function into the page's MAIN world.
async function zoiVjudgePage(job, stage) {
    const wait = async (get, message, timeout = 20000) => {
        const end = Date.now() + timeout;
        do { const value = get(); if (value) return value; await new Promise(resolve => setTimeout(resolve, 100)); } while (Date.now() < end);
        throw Error(message);
    };
    const visible = e => e && e.getClientRects().length > 0;
    const normalizeNewlines = text => text.replace(/\r\n?/g, '\n');
    const expected = new URL(job.url);
    const verifyUrl = () => { if (location.origin !== expected.origin || location.pathname !== expected.pathname || location.hash !== expected.hash) throw Error('页面题目发生变化，停止提交。'); };
    try {
        verifyUrl();
        if (stage === 'prepare' && !document.querySelector('#submit-form')) {
            const trigger = await wait(() => [...document.querySelectorAll('#btn-submit, #problem-submit')].find(e => visible(e) && !e.closest('.modal')), '未找到题目的提交按钮，请登录 VJudge 并打开具体题目。');
            trigger.click();
        }
        const form = await wait(() => document.querySelector('#submit-form'), '未打开提交表单，请检查登录状态或比赛权限。');
        const modal = form.closest('.modal');
        // Bootstrap builds the editor in show.bs.modal, before the backdrop transition shows the modal.
        // Wait only during preparation; a subsequently closed form must still stop submission.
        if (stage === 'prepare') await wait(() => visible(modal), '提交表单未能显示，请检查网页提示后重试。');
        const language = await wait(() => { const e = form.querySelector('#submit-language'); return e?.options.length > 1 && e; }, 'VJudge 未加载编译器列表。');
        const inContest = expected.pathname.startsWith('/contest/');
        const formProblem = inContest ? form.querySelector('#contest-num')?.textContent.trim().split(/\s+-\s+/)[0] : form.querySelector('.problem-origin')?.textContent.replace(/\s+/g, '');
        const expectedProblem = inContest ? expected.hash.split('/')[1] : expected.pathname.split('/')[2];
        if (formProblem !== expectedProblem) throw Error('表单题号与 CPH 题目不一致，停止提交。');
        const candidates = [...language.options].filter(o => o.value && !o.disabled && !o.parentElement?.disabled && /(?:[cg]\+\+|cpp)/i.test(o.textContent) && /gnu|gcc|g\+\+/i.test(o.textContent) && !/clang/i.test(o.textContent)).map(option => {
            const label = option.textContent.trim();
            const version = label.match(/(?:[cg]\+\+|cpp)\s*(98|03|0x|11|1y|14|1z|17|2a|20|2b|23|2c|26)\b/i)?.[1].toLowerCase();
            const aliases = { '98': 1998, '03': 2003, '0x': 2011, '1y': 2014, '1z': 2017, '2a': 2020, '2b': 2023, '2c': 2026 };
            // GCC 版本不是 C++ 标准版本; 未标明标准的 GNU C++ 只作为最后选择
            return { option, label, standard: version ? (aliases[version] || 2000 + Number(version)) : 0, bits64: /\b64(?:\s*-?\s*bit)?\b|x86_64|amd64/i.test(label) };
        });
        if (!candidates.length) throw Error('这道题的原 OJ 没有提供可用的 GNU C++ 编译器，未提交。');
        candidates.sort((a, b) => Number(b.standard === 2020) - Number(a.standard === 2020) || b.standard - a.standard || Number(b.bits64) - Number(a.bits64));
        const choice = candidates[0], selected = choice.option;
        const cm = await wait(() => form.querySelector('.CodeMirror')?.CodeMirror, 'VJudge 代码编辑器发生变化，请更新适配。');
        const defaultLabel = form.querySelector('label[for="submitter-type0"]');
        const personalLabel = form.querySelector('label[for="submitter-type1"]');
        const defaultAvailable = defaultLabel && !defaultLabel.classList.contains('disabled');
        const method = defaultAvailable ? '#submitter-type0' : '#submitter-type1';
        const accountReady = () => {
            if (defaultAvailable) return true;
            const account = form.querySelector('#submit-remote-account');
            return personalLabel && !personalLabel.classList.contains('disabled') && account && !account.disabled && Number(account.value) > 0 && !account.selectedOptions[0]?.disabled;
        };
        if (stage === 'prepare') {
            language.value = selected.value;
            language.dispatchEvent(new Event('change', { bubbles: true }));
            // VJudge may disable shared accounts for an OJ; respect its native restriction.
            const privateCode = form.querySelector('#open0'), submitter = form.querySelector(method);
            if (!privateCode || !submitter || !defaultLabel) throw Error('提交选项发生变化，请更新适配。');
            privateCode.click(); submitter.click();
            cm.setValue(normalizeNewlines(job.code)); cm.save();
            if (job.mode !== 'check') await wait(accountReady, '此题无法使用 VJudge 公共账号。请先在网页“管理账号”绑定并选中可用的原 OJ 个人账号，再重新提交。', 8000);
            return { prepared: true, language: choice.label, languageFallback: choice.standard !== 2020, warning: accountReady() ? '' : '当前题需要绑定可用的原 OJ 个人账号后才能提交。' };
        }
        if (job.mode === 'check' && stage !== 'verify') throw Error('连接检查不能发送提交。');
        verifyUrl();
        if (!visible(modal)) throw Error('提交表单已关闭，停止自动提交。');
        if (language.value !== selected.value) throw Error('提交编译器发生变化，停止自动提交。');
        if (normalizeNewlines(cm.getValue()) !== normalizeNewlines(job.code)) throw Error('网页中的代码内容发生变化，停止自动提交。');
        if (!form.querySelector('#open0')?.checked) throw Error('代码公开选项发生变化，停止自动提交。');
        if (!form.querySelector(method)?.checked || (!accountReady() && job.mode !== 'check')) throw Error('原 OJ 提交账号发生变化或不可用，停止自动提交。');
        if (stage === 'verify') return { verified: true };
        if (document.querySelector('#turnstile-container-submit iframe')) throw Error('VJudge 要求验证码，请在网页完成验证后手动点击提交。');
        const button = modal.querySelector('#btn-submit');
        if (!button || button.disabled) throw Error('提交按钮尚不可用，请检查网页提示。');
        const route = expected.pathname.startsWith('/contest/') ? `/contest/submit/${expected.pathname.split('/')[2]}/${expected.hash.split('/')[1]}` : `/problem/submit/${expected.pathname.split('/')[2]}`;
        // Observe only the native form's one request; credentials and CAPTCHA remain with VJudge.
        return await new Promise(resolve => {
            const proto = XMLHttpRequest.prototype, originalOpen = proto.open;
            let timer, finished = false;
            const finish = result => { if (finished) return; finished = true; clearTimeout(timer); if (proto.open === observe) proto.open = originalOpen; resolve(result); };
            function observe(method, address, ...args) {
                if (String(method).toUpperCase() === 'POST' && new URL(address, location.href).pathname === route) {
                    if (proto.open === observe) proto.open = originalOpen;
                    this.addEventListener('loadend', () => {
                        try {
                            const data = this.responseType === 'json' ? this.response : JSON.parse(this.responseText);
                            if (this.status >= 200 && this.status < 300 && /^\d+$/.test(String(data.runId))) finish({ ok: true, runId: String(data.runId) });
                            else finish({ error: 'VJudge 未接收提交，请查看表单提示；验证码或登录需要你在网页处理。' });
                        } catch { finish({ error: '未能确认提交结果，请先检查 VJudge 记录，不会自动重试。' }); }
                    }, { once: true });
                }
                return originalOpen.call(this, method, address, ...args);
            }
            proto.open = observe;
            timer = setTimeout(() => finish({ error: '提交结果超时，请先检查网页记录，不会自动重试。' }), 20000);
            try { button.click(); } catch (e) { finish({ error: e.message }); }
        });
    } catch (e) { return { error: e.message }; }
}
if (typeof module !== 'undefined') module.exports = { zoiVjudgePage };
