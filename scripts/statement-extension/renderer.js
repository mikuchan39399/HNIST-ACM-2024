'use strict';
/* Parsing is shared by the webview and DOM fixture checks. Remote scripts are never executed. */
function zoiParsePage(document, html, site, base, markdown) {
    const parsed = new document.defaultView.DOMParser().parseFromString(html, 'text/html');
    const variants = [], pdfs = [];
    if (site === 'cf') {
        const root = parsed.querySelector('.problem-statement');
        if (!root) throw Error('没有找到 Codeforces 题面，请从 Chrome 读取。');
        variants.push({ label: '原题', html: root.outerHTML });
    } else if (site === 'atc') {
        const root = parsed.querySelector('#task-statement');
        if (!root) throw Error('没有找到 AtCoder 题面，请从 Chrome 读取。');
        for (const [selector, label] of [['.lang-en', 'English'], ['.lang-ja', '日本語']]) {
            const part = root.querySelector(selector); if (part?.textContent.trim()) variants.push({ label, html: part.innerHTML });
        }
        if (!variants.length) variants.push({ label: '原题', html: root.innerHTML });
    } else {
        const node = parsed.querySelector('textarea.data-json-container');
        if (!node) throw Error('没有找到 VJudge 题面数据。');
        const data = JSON.parse(node.value || node.textContent);
        if (!Array.isArray(data.sections) || !data.sections.length) throw Error('这个题面版本没有正文。');
        const container = parsed.createElement('div');
        for (const section of data.sections) {
            if (section.title) { const h = parsed.createElement('h2'); h.textContent = section.title; container.append(h); }
            const value = section.value || {}, content = String(value.content || '');
            const pdf = content.trim().match(/^\[pdf:([^\]]+)\]$/i);
            if (pdf) {
                const url = pdf[1].replace(/^CDN_BASE_URL/, 'https://cdn.vjudge.net.cn');
                pdfs.push(new URL(url, base).href);
                const holder = parsed.createElement('div'); holder.className = 'pdf-document'; holder.dataset.pdf = String(pdfs.length - 1); container.append(holder);
            } else {
                const part = parsed.createElement('div');
                if (value.format === 'MD') part.innerHTML = markdown.render(content);
                else if (value.format === 'HTML') part.innerHTML = content;
                else { const pre = parsed.createElement('pre'); pre.textContent = content; part.append(pre); }
                container.append(part);
            }
        }
        variants.push({ label: data.lang || '题面', html: container.innerHTML });
    }
    return { variants, pdfs };
}

function zoiPrepareContent(document, html, site, base, purify) {
    const container = document.createElement('div');
    // DOMPurify removes active content first. MathJax's inert TeX scripts are converted separately.
    const parsed = new document.defaultView.DOMParser().parseFromString(html, 'text/html');
    for (const node of parsed.querySelectorAll('script[type^="math/tex"]')) {
        const display = /mode\s*=\s*display/.test(node.type);
        node.replaceWith(parsed.createTextNode((display ? '\\[' : '\\(') + node.textContent + (display ? '\\]' : '\\)')));
    }
    for (const node of parsed.querySelectorAll('.katex')) {
        const tex = node.querySelector('annotation[encoding="application/x-tex"]');
        if (tex) node.replaceWith(parsed.createTextNode('\\(' + tex.textContent + '\\)'));
    }
    for (const node of parsed.querySelectorAll('.MathJax, .MathJax_Display, .MathJax_Preview, mjx-container')) node.remove();
    if (site === 'atc') {
        for (const node of parsed.querySelectorAll('var')) node.replaceWith(parsed.createTextNode('\\(' + node.textContent + '\\)'));
        for (const node of parsed.querySelectorAll('.source-code-for-copy')) node.remove();
    }
    for (const node of parsed.querySelectorAll('[href], [src]')) for (const attr of ['href', 'src']) if (node.hasAttribute(attr)) {
        try { const u = new URL(node.getAttribute(attr), base); if (!['http:', 'https:'].includes(u.protocol) || u.username || u.password) node.removeAttribute(attr); else node.setAttribute(attr, u.href); }
        catch { node.removeAttribute(attr); }
    }
    container.append(purify.sanitize(parsed.body.innerHTML, { RETURN_DOM_FRAGMENT: true, USE_PROFILES: { html: true, mathMl: true }, FORBID_TAGS: ['style', 'form', 'input', 'button', 'select', 'textarea', 'iframe', 'object', 'embed'], FORBID_ATTR: ['style', 'srcset', 'id', 'name'] }));
    return container;
}

function zoiHtmlSamples(root, site) {
    const value = el => {
        const copy = el.cloneNode(true);
        for (const br of copy.querySelectorAll('br')) br.replaceWith('\n');
        const lines = copy.querySelectorAll('.test-example-line, ol.linenums > li');
        return (lines.length ? Array.from(lines, n => n.textContent).join('\n') : copy.textContent).replace(/\r/g, '').replace(/^\n|\n$/g, '') + '\n';
    };
    if (site === 'cf') {
        const ins = root.querySelectorAll('.sample-test .input pre'), outs = root.querySelectorAll('.sample-test .output pre');
        if (ins.length && ins.length === outs.length) return Array.from(ins, (p, i) => ({ input: value(p), output: value(outs[i]) }));
    }
    const inputHeading = /^(?:Sample\s+Input|Input\s+Example|入力例|样例输入|输入样例)(?:\s*#?\s*\d+)?\s*[:：]?$/i;
    const outputHeading = /^(?:Sample\s+Output|Output\s+Example|出力例|样例输出|输出样例)(?:\s*#?\s*\d+)?\s*[:：]?$/i;
    const ins = [], outs = [];
    for (const heading of root.querySelectorAll('h2, h3, h4, .section-title')) {
        const text = heading.textContent.trim();
        const target = inputHeading.test(text) ? ins : outputHeading.test(text) ? outs : null;
        if (!target) continue;
        let sibling = heading.nextElementSibling, pre;
        while (sibling && !/^H[1-6]$/.test(sibling.tagName)) { pre = sibling.matches('pre') ? sibling : sibling.querySelector('pre'); if (pre) break; sibling = sibling.nextElementSibling; }
        if (pre) target.push(value(pre));
    }
    if (ins.length && ins.length === outs.length) return ins.map((input, i) => ({ input, output: outs[i] }));
    const pairs = [];
    for (const row of root.querySelectorAll('table.vjudge_sample tbody tr')) {
        const cells = row.querySelectorAll('td'); if (cells.length === 2 && cells[0].querySelector('pre') && cells[1].querySelector('pre')) pairs.push({ input: value(cells[0].querySelector('pre')), output: value(cells[1].querySelector('pre')) });
    }
    return pairs;
}

function zoiPdfSamples(pages) {
    const lines = [];
    for (const items of pages) {
        const rows = [];
        for (const item of items.filter(i => i.str?.trim()).sort((a, b) => b.transform[5] - a.transform[5] || a.transform[4] - b.transform[4])) {
            let row = rows.find(r => Math.abs(r.y - item.transform[5]) < 2);
            if (!row) { row = { y: item.transform[5], items: [] }; rows.push(row); } row.items.push(item);
        }
        for (const row of rows) {
            let text = '', right = -Infinity;
            for (const item of row.items.sort((a, b) => a.transform[4] - b.transform[4])) { const gap = item.transform[4] - right; text += (text && gap > 2 ? ' ' : '') + item.str; right = item.transform[4] + item.width; }
            lines.push(text.trimEnd());
        }
    }
    const input = lines.findIndex(l => /^Sample\s*Input\s*:?$/i.test(l.trim()));
    const output = lines.findIndex((l, i) => i > input && /^Sample\s*Output\s*:?$/i.test(l.trim()));
    if (input < 0 || output <= input + 1) return [];
    const stop = lines.findIndex((l, i) => i > output && /^(?:Note|Notes|Hint|Hints|Explanation|样例说明)\s*:?$/i.test(l.trim()));
    const tail = lines.slice(output + 1, stop < 0 ? undefined : stop);
    // A narrow, unambiguous layout only; uncertain PDF layouts must not create bogus tests.
    const a = lines.slice(input + 1, output), b = tail;
    if (!a.length || !b.length || a.concat(b).some(l => /Sample\s*(?:Input|Output)/i.test(l))) return [];
    return [{ input: a.join('\n') + '\n', output: b.join('\n') + '\n' }];
}

if (typeof module === 'object' && module.exports) module.exports = { zoiParsePage, zoiPrepareContent, zoiHtmlSamples, zoiPdfSamples };
else (() => {
    const api = acquireVsCodeApi(), $ = id => document.getElementById(id), md = markdownit({ html: true, breaks: false });
    let config, page, parsed, generation = 0, pendingPdfs = 0, pdfTexts = [], tests = [], pdfJobs = [], renderEpoch = 0;
    const send = message => api.postMessage(message);
    const status = (text, error = false) => { $('status').textContent = text; $('status').className = error ? 'error' : ''; };
    for (const id of ['refresh', 'chrome', 'original', 'code']) $(id).onclick = () => send({ type: id });
    $('versions').onchange = () => { const index = Number($('versions').value); if (page.site === 'vj') send({ type: 'choose', index }); else render(index); };
    document.addEventListener('click', e => { const a = e.target.closest('a[href]'); if (a) { e.preventDefault(); send({ type: 'link', url: a.href }); } });
    function reportSamples() { send({ type: 'parsed', generation, tests, pendingPdf: pendingPdfs > 0 }); }
    function render(index = 0) {
        ++renderEpoch; for (const task of pdfJobs) task.destroy().catch(() => {}); pdfJobs = []; pdfTexts = []; pendingPdfs = 0;
        const root = zoiPrepareContent(document, parsed.variants[index].html, page.site, page.base, DOMPurify);
        tests = zoiHtmlSamples(root, page.site); $('statement').replaceChildren(root);
        renderMathInElement(root, { delimiters: [{ left: '$$$', right: '$$$', display: false }, { left: '$$', right: '$$', display: true }, { left: '\\[', right: '\\]', display: true }, { left: '\\(', right: '\\)', display: false }, { left: '$', right: '$', display: false }], throwOnError: false, trust: false, maxExpand: 1000, maxSize: 20 });
        parsed.pdfs.forEach((url, id) => {
            const holder = root.querySelector('[data-pdf="' + id + '"]'); if (!holder) return;
            pendingPdfs++; holder.textContent = '正在加载 PDF…'; send({ type: 'pdf', url, id, generation });
        });
        status(pendingPdfs ? '正在加载 PDF 题面…' : '题面已载入'); reportSamples();
    }
    async function showPdf(message) {
        const epoch = renderEpoch, holder = $('statement').querySelector('[data-pdf="' + message.id + '"]'); if (!holder) return;
        const pdfjs = await import(config.pdfModule); pdfjs.GlobalWorkerOptions.workerSrc = config.pdfWorker;
        const task = pdfjs.getDocument({ data: Uint8Array.from(atob(message.data), c => c.charCodeAt(0)), isEvalSupported: false, useSystemFonts: true, cMapUrl: config.pdfAssets + 'cmaps/', cMapPacked: true, standardFontDataUrl: config.pdfAssets + 'standard_fonts/', wasmUrl: config.pdfAssets + 'wasm/' });
        pdfJobs.push(task); const pdf = await task.promise; if (epoch !== renderEpoch) return;
        holder.replaceChildren(); const link = document.createElement('a'); link.className = 'pdf-link'; link.href = parsed.pdfs[message.id]; link.textContent = `PDF 原文 · ${pdf.numPages} 页`; holder.append(link);
        if (pdf.numPages > 100) throw Error('PDF 超过 100 页，请打开原文查看。');
        for (let i = 1; i <= pdf.numPages; i++) {
            if (epoch !== renderEpoch) return;
            const p = await pdf.getPage(i), natural = p.getViewport({ scale: 1 });
            const scale = Math.min(1.5, Math.max(0.35, holder.clientWidth / natural.width));
            const viewport = p.getViewport({ scale }), ratio = Math.min(devicePixelRatio || 1, 2);
            const div = document.createElement('div'); div.className = 'pdf-page'; div.style.width = viewport.width + 'px'; div.style.height = viewport.height + 'px';
            div.style.setProperty('--scale-factor', scale); div.style.setProperty('--total-scale-factor', scale); const canvas = document.createElement('canvas'); canvas.width = viewport.width * ratio; canvas.height = viewport.height * ratio; div.append(canvas); holder.append(div);
            await p.render({ canvasContext: canvas.getContext('2d'), viewport, transform: [ratio, 0, 0, ratio, 0, 0] }).promise;
            const text = await p.getTextContent(); pdfTexts.push(text.items);
            const layer = document.createElement('div'); layer.className = 'textLayer'; div.append(layer);
            await new pdfjs.TextLayer({ textContentSource: text, container: layer, viewport }).render();
        }
        if (epoch !== renderEpoch) return;
        pendingPdfs--; if (!pendingPdfs) { if (!tests.length) tests = zoiPdfSamples(pdfTexts); status('题面已载入'); reportSamples(); }
    }
    window.addEventListener('message', async e => {
        const m = e.data;
        try {
            if (m.type === 'init') {
                config = m; $('title').textContent = m.problem.name;
                $('limits').textContent = [m.problem.timeLimit ? `${m.problem.timeLimit} ms` : '', m.problem.memoryLimit ? `${m.problem.memoryLimit} MB` : ''].filter(Boolean).join(' · ');
                for (const [i, t] of m.problem.tests.entries()) { const title = document.createElement('h3'); title.textContent = '样例 ' + (i + 1); const a = document.createElement('pre'), b = document.createElement('pre'); a.textContent = t.input; b.textContent = t.output; $('sample-list').append(title, a, b); } $('samples').hidden = !m.problem.tests.length;
            } else if (m.type === 'status') status(m.text);
            else if (m.type === 'error') status(m.text, true);
            else if (m.type === 'page') {
                page = m; generation = m.generation; parsed = zoiParsePage(document, m.html, m.site, m.base, md);
                const choices = m.site === 'vj' ? m.choices : parsed.variants;
                $('versions').replaceChildren(...choices.map((c, i) => { const option = document.createElement('option'); option.value = i; option.textContent = c.label; return option; }));
                $('versions').value = m.selected || 0; $('versions').hidden = choices.length < 2; render();
            } else if (m.type === 'pdf' && m.generation === generation) await showPdf(m);
        } catch (error) { status(error.message, true); send({ type: 'parseError', generation, text: error.message }); }
    });
    send({ type: 'ready' });
})();
