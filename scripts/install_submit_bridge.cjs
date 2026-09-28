'use strict';

const fs = require('node:fs');
const path = require('node:path');
const os = require('node:os');
const vm = require('node:vm');

// Match the installed bundles explicitly: an unknown upstream version must fail closed.
const specifications = [
    {
        id: 'divyanshuagrawal.competitive-programming-helper',
        replacements: [
            ['t.storeSubmitProblem=e=>{const t=e.srcPath', 't.storeSubmitProblem=async e=>{S=P;const t=e.srcPath'],
            ['r=(0,d.readFileSync)(t).toString(),i=(0,p.getLanguageId)(e.srcPath);S=',
                'r=await require("./zoi-submit-bridge.cjs").prepareFile(l,t),i=(0,p.getLanguageId)(e.srcPath);S='],
            ['(0,c.storeSubmitProblem)(r),', 'await(0,c.storeSubmitProblem)(r),'],
            ['t.submitToCodeForces=async()=>{var e;', 't.submitToCodeForces=async()=>{try{var e;'],
            ['},t.getProblemName=e=>', '}catch(zoiError){u.window.showErrorMessage(zoiError.message)}},t.getProblemName=e=>'],
        ],
    },
    {
        id: 'yltx.vscode-luogu',
        replacements: [
            ['(0,a.q6)(e,t.document.getText(),n.id,n.O2)',
                '(0,a.q6)(e,await require("./zoi-submit-bridge.cjs").prepareDocument(o,t.document),n.id,n.O2)'],
        ],
    },
];

const luogu418 = [[
    '(0,a.q6)(e,t.getText(),n.id,n.O2)',
    '(0,a.q6)(e,await require("./zoi-submit-bridge.cjs").prepareDocument(s,t),n.id,n.O2)',
]];
const cphVjudge = [[
    '(0,f.isCodeforcesUrl)(i)?(await(0,c.storeSubmitProblem)(r),',
    'require("./zoi-submit-bridge.cjs").isVjudge(i)?await require("./zoi-submit-bridge.cjs").submitVjudge(u,r):(0,f.isCodeforcesUrl)(i)?(await(0,c.storeSubmitProblem)(r),',
]];
const cphBrowser = [[cphVjudge[0][0],
    'require("./zoi-submit-bridge.cjs").isAtcoder(i)?await require("./zoi-submit-bridge.cjs").submitAtcoder(u,r):' + cphVjudge[0][1],
]];
const frontendCondition = 'e.hostname.endsWith("codeforces.com")||"open.kattis.com"===e.hostname||e.hostname.endsWith("cses.fi")?e.hostname.endsWith("codeforces.com")?';
const supported = '(e.hostname.endsWith("codeforces.com")||["vjudge.net","vjudge.net.cn"].includes(e.hostname))';
const cphFrontend = [[frontendCondition, supported + '||"open.kattis.com"===e.hostname||e.hostname.endsWith("cses.fi")?' + supported + '?']];
const browserSupported = '(e.hostname.endsWith("codeforces.com")||["vjudge.net","vjudge.net.cn","atcoder.jp"].includes(e.hostname))';
const cphBrowserFrontend = [[frontendCondition, browserSupported + '||"open.kattis.com"===e.hostname||e.hostname.endsWith("cses.fi")?' + browserSupported + '?']];

function removeKnown(source, versions) {
    for (const replacements of versions) if (replacements.every(([, to]) => source.includes(to))) return transform(source, replacements, true);
    return source;
}

function update(source, replacements, mode) {
    const patched = replacements.every(([, to]) => source.includes(to));
    const next = mode === 'uninstall' ? (patched ? transform(source, replacements, true) : source)
        : (patched ? source : transform(source, replacements));
    return { source, next, patched };
}

function transform(source, replacements, undo = false) {
    for (const pair of replacements) {
        const [from, to] = undo ? [...pair].reverse() : pair;
        if (source.split(from).length - 1 !== 1) {
            throw new Error('插件代码与适配版本不匹配；未覆盖该文件。请检查插件更新后的提交入口。');
        }
        source = source.replace(from, to);
    }
    new vm.Script(source);
    return source;
}

function install(directory, mode = 'install') {
    const base = path.resolve(directory);
    const entries = JSON.parse(fs.readFileSync(path.join(base, 'extensions.json'), 'utf8'));
    const shim = '// Managed by ZOI submit bridge.\nmodule.exports = require(' + JSON.stringify(path.join(__dirname, 'submit_bridge.cjs')) + ');\n';
    const plans = specifications.map(spec => {
        const entry = entries.find(e => e.identifier.id.toLowerCase() === spec.id);
        if (!entry?.relativeLocation) throw new Error('没有找到已启用插件：' + spec.id);
        const folder = path.resolve(base, entry.relativeLocation);
        if (!folder.startsWith(base + path.sep)) throw new Error('插件路径超出指定目录。');
        const pkg = JSON.parse(fs.readFileSync(path.join(folder, 'package.json'), 'utf8'));
        const file = path.resolve(folder, pkg.main);
        if (!file.startsWith(folder + path.sep)) throw new Error('插件入口路径超出目录。');
        const source = fs.readFileSync(file, 'utf8');
        const replacements = spec.id === 'yltx.vscode-luogu' && luogu418.some(([from, to]) => source.includes(from) || source.includes(to)) ? luogu418 : spec.replacements;
        const extra = spec.id === specifications[0].id;
        // Remove our routing before restoring the old snapshot patch on uninstall.
        const routed = extra ? removeKnown(source, [cphBrowser, cphVjudge]) : source;
        const basePatch = update(routed, replacements, mode);
        const finalPatch = extra && mode !== 'uninstall' ? update(basePatch.next, cphBrowser, mode) : basePatch;
        const patched = basePatch.patched && (!extra || cphBrowser.every(([, to]) => source.includes(to)));
        const assets = [{ file, source, next: finalPatch.next }];
        let frontendPatched = true;
        if (extra) {
            const frontend = path.join(path.dirname(file), 'frontend.module.js');
            const source = fs.readFileSync(frontend, 'utf8');
            const clean = removeKnown(source, [cphBrowserFrontend, cphFrontend]);
            const next = mode === 'uninstall' ? clean : update(clean, cphBrowserFrontend, mode).next;
            assets.push({ file: frontend, source, next });
            frontendPatched = cphBrowserFrontend.every(([, to]) => source.includes(to));
        }
        const helper = path.join(path.dirname(file), 'zoi-submit-bridge.cjs');
        if (fs.existsSync(helper) && !fs.readFileSync(helper, 'utf8').startsWith('// Managed by ZOI submit bridge.\n')) {
            throw new Error('发现非本工具管理的同名文件：' + helper);
        }
        return { id: spec.id, assets, helper, patched: patched && frontendPatched };
    });
    if (mode === 'check') {
        return plans.map(p => ({ id: p.id, installed: p.patched && fs.existsSync(p.helper) && fs.readFileSync(p.helper, 'utf8') === shim }));
    }
    // Validate both adapters before changing either extension. Backups are for manual recovery.
    for (const p of plans) {
        if (mode === 'install') {
            for (const asset of p.assets) {
                const backup = asset.file + '.zoi-submit-backup';
                if (!fs.existsSync(backup)) fs.copyFileSync(asset.file, backup, fs.constants.COPYFILE_EXCL);
            }
            fs.writeFileSync(p.helper, shim);
        }
        for (const asset of p.assets) if (asset.source !== asset.next) fs.writeFileSync(asset.file, asset.next);
        if (mode === 'uninstall' && fs.existsSync(p.helper)) fs.unlinkSync(p.helper);
    }
    return plans.map(p => ({ id: p.id, action: mode }));
}

if (require.main === module) {
    try {
        const args = process.argv.slice(2);
        const mode = args.includes('--uninstall') ? 'uninstall' : args.includes('--check') ? 'check' : 'install';
        const index = args.indexOf('--extensions-dir');
        if (index >= 0 && !args[index + 1]) throw new Error('--extensions-dir needs a path');
        const result = install(index < 0 ? path.join(os.homedir(), '.vscode/extensions') : args[index + 1], mode);
        console.log(JSON.stringify(result, null, 2));
        if (mode === 'check' && result.some(r => !r.installed)) process.exitCode = 1;
    } catch (error) { console.error(error.message); process.exitCode = 1; }
}

module.exports = { install, transform, specifications, luogu418, cphVjudge, cphFrontend, cphBrowser, cphBrowserFrontend };
