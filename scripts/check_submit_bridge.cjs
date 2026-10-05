'use strict';

const fs = require('node:fs');
const path = require('node:path');
const vm = require('node:vm');
const assert = require('node:assert/strict');
const { execFileSync } = require('node:child_process');
const { install, transform, specifications, cphVjudge, cphFrontend, cphBrowser, cphBrowserFrontend, luogu418 } = require('./install_submit_bridge.cjs');
const bridge = require('./submit_bridge.cjs');
const root = path.resolve(__dirname, '..');
const parent = path.join(root, '.zoi-checks/codex-work');
fs.mkdirSync(parent, { recursive: true });
const dir = fs.mkdtempSync(path.join(parent, 'submit-check-'));
const file = path.join(dir, '中文 空格.cpp');
const original = '#include "local.h"\nint main() { return value()==7 ? 0 : 1; }\n';
fs.writeFileSync(file, original);
fs.writeFileSync(path.join(dir, 'local.h'), '#pragma once\n// 中文注释\ninline int value(){return 7;}\n');
let text = original;
const document = { fileName: file, uri: { scheme: 'file' }, version: 1, getText: () => text };
const errors = [];
const vscode = {
    workspace: { isTrusted: true, openTextDocument: async () => document },
    Uri: { file: x => x },
    window: { activeTextEditor: { document }, showErrorMessage: e => errors.push(e) },
};

// Minimal bundle fixtures use the exact installed call shapes, with network endpoints mocked.
const cph = 't.storeSubmitProblem=e=>{const t=e.srcPath,n=(0,v.getProblemName)(e.url),r=(0,d.readFileSync)(t).toString(),i=(0,p.getLanguageId)(e.srcPath);S={sourceCode:r,problemName:n,languageId:i}},t.submitToCodeForces=async()=>{var e;const r=problem,i=new URL(r.url);(0,f.isCodeforcesUrl)(i)?((0,c.storeSubmitProblem)(r),messages.push("waiting")):null},t.getProblemName=e=>e;';
const luogu = 't.submit=async()=>{const e={},n={id:1,O2:true},t={document};return (0,a.q6)(e,t.document.getText(),n.id,n.O2)};';

async function utilsCompatibilityChecks() {
    const header = JSON.stringify(path.join(root, 'algorithms/杂项/utils/utils.cpp').replace(/\\/g, '/'));
    const input = `#include ${header}
#include ${header}
int main() {
    fast_io();
    int hi = 3, lo = 3;
    assert(cmax(hi, 7) && hi == 7);
    assert(cmin(lo, 1) && lo == 1);
    assert(!cmax(hi, 7) && !cmin(lo, lo));
    LL big = LLONG_MIN;
    assert(cmax(big, LLONG_MAX) && big == LLONG_MAX);
    assert(cmin(big, 0LL) && big == 0);
    PII point(1, 9);
    assert(cmax(point, PII(2, 0)) && point == PII(2, 0));
    for (int n : {200000, 0, 1, 257, 200000}) {
        VI a(n + 20, 7); VLL b(n + 1, 9); vector<bool> bits(n + 11, true);
        z_fill_n(n, 0, a, b, bits);
        for (size_t i = 0; i < a.size(); ++i) assert(a[i] == (i < (size_t)n + 10 ? 0 : 7));
        for (LL x : b) assert(x == 0);
        for (size_t i = 0; i < bits.size(); ++i) assert(bits[i] == (i >= (size_t)n + 10));
    }
    vector<string> words(12, "old"); z_fill_n(1, string("new"), words);
    assert(words[10] == "new" && words[11] == "old");
    VI empty; z_fill_n(0, 0, empty);
    assert(dx4[2] == -1 && dy4[0] == 1 && dx8[0] == -1 && dy8[7] == -1);
    dx4[0] = 3; assert(dx4[0] == 3); dx4[0] = 0;
    assert(floor_div(-7, 3) == -3 && ceil_div(-7, 3) == -2);
    assert(floor_isqrt(LLONG_MAX) == 3037000499LL && ceil_isqrt(LLONG_MAX) == 3037000500LL);
#if __cplusplus >= 201703L
    assert(string_view("hello").size() == 5);
#endif
#if __cplusplus >= 202002L
    static_assert(integral<LL>); assert(popcount(7u) == 3);
#endif
    cout << "utils compatible";
}
`;
    const src = path.join(dir, 'utils-compat.cpp'); fs.writeFileSync(src, input);
    const snapshot = await bridge.prepareDocument(vscode, { ...document, fileName: src, getText: () => input });
    const submission = path.join(dir, 'utils-submission.cpp'); fs.writeFileSync(submission, snapshot);
    // 编译实际展开的完整 KMP 对拍, 同时验证旧标准语法、依赖展开和算法结果。
    const kmpSource = path.join(root, 'algorithms/字符串/KMP/对拍/kmp_check.cpp');
    const kmpSnapshot = await bridge.prepareDocument(vscode, { ...document, fileName: kmpSource, getText: () => fs.readFileSync(kmpSource, 'utf8') });
    const kmpSubmission = path.join(dir, 'kmp-submission.cpp'); fs.writeFileSync(kmpSubmission, kmpSnapshot);
    const manacherSource = path.join(root, 'algorithms/字符串/Manacher/对拍/manacher_check.cpp');
    const manacherSnapshot = await bridge.prepareDocument(vscode, { ...document, fileName: manacherSource, getText: () => fs.readFileSync(manacherSource, 'utf8') });
    const manacherSubmission = path.join(dir, 'manacher-submission.cpp'); fs.writeFileSync(manacherSubmission, manacherSnapshot);
    const manacherTemplate = path.join(root, 'algorithms/字符串/Manacher/manacher.cpp');
    const manacherUsage = fs.readFileSync(manacherTemplate, 'utf8').match(/\/\*\s*Usage\b([\s\S]*?)\*\//)[1];
    const usageSnapshot = await bridge.prepareDocument(vscode, { ...document, fileName: manacherTemplate, getText: () => manacherUsage });
    const usageSubmission = path.join(dir, 'manacher-usage.cpp'); fs.writeFileSync(usageSubmission, usageSnapshot);
    const zSource = path.join(root, 'algorithms/字符串/Z函数/对拍/z_function_check.cpp');
    const zSnapshot = await bridge.prepareDocument(vscode, { ...document, fileName: zSource, getText: () => fs.readFileSync(zSource, 'utf8') });
    const zSubmission = path.join(dir, 'z-submission.cpp'); fs.writeFileSync(zSubmission, zSnapshot);
    const zTemplate = path.join(root, 'algorithms/字符串/Z函数/z_function.cpp');
    const zUsage = fs.readFileSync(zTemplate, 'utf8').match(/\/\*\s*Usage\b([\s\S]*?)\*\//)[1];
    const zUsageSnapshot = await bridge.prepareDocument(vscode, { ...document, fileName: zTemplate, getText: () => zUsage });
    const zUsageSubmission = path.join(dir, 'z-usage.cpp'); fs.writeFileSync(zUsageSubmission, zUsageSnapshot);
    const acSource = path.join(root, 'algorithms/字符串/AC自动机/对拍/acam_check.cpp');
    const acSnapshot = await bridge.prepareDocument(vscode, { ...document, fileName: acSource, getText: () => fs.readFileSync(acSource, 'utf8') });
    const acSubmission = path.join(dir, 'acam-submission.cpp'); fs.writeFileSync(acSubmission, acSnapshot);
    const acTemplate = path.join(root, 'algorithms/字符串/AC自动机/acam.cpp');
    const acUsage = fs.readFileSync(acTemplate, 'utf8').match(/\/\*\s*Usage\b([\s\S]*?)\*\//)[1];
    const acUsageSnapshot = await bridge.prepareDocument(vscode, { ...document, fileName: acTemplate, getText: () => acUsage });
    const acUsageSubmission = path.join(dir, 'acam-usage.cpp'); fs.writeFileSync(acUsageSubmission, acUsageSnapshot);
    for (const std of ['c++11', 'c++14', 'c++17', 'c++20', 'c++23']) {
        const poison = path.join(dir, std); fs.mkdirSync(poison);
        // 拦截展开快照直接引入新标准头; 新版 libstdc++ 自己的内部依赖仍交回系统头
        for (const name of ['c++11', 'c++14'].includes(std) ? ['string_view', 'bit', 'concepts', 'ranges', 'span'] : std === 'c++17' ? ['bit', 'concepts', 'ranges', 'span'] : []) {
            fs.writeFileSync(path.join(poison, name), `#pragma GCC system_header\n#if __INCLUDE_LEVEL__ == 1\n#error unavailable standard header\n#endif\n#include_next <${name}>\n`);
        }
        const exe = path.join(dir, process.platform === 'win32' ? 'utils-compat.exe' : 'utils-compat');
        execFileSync('g++', ['-std=' + std, '-pedantic-errors', '-Wall', '-Wextra', '-Werror', '-O2', '-I', poison, submission, '-o', exe], { cwd: dir, env: { ...process.env, TEMP: dir, TMP: dir, TMPDIR: dir } });
        assert.equal(execFileSync(exe, [], { cwd: dir, encoding: 'utf8' }), 'utils compatible');
        execFileSync('g++', ['-std=' + std, '-pedantic-errors', '-Wall', '-Wextra', '-Werror', '-O2', '-I', poison, path.join(root, 'algorithms/杂项/对拍/utils_local_check.cpp'), '-o', exe], { cwd: dir, env: { ...process.env, TEMP: dir, TMP: dir, TMPDIR: dir } });
        assert.match(execFileSync(exe, [], { cwd: dir, encoding: 'utf8' }), /utils_local_check passed/);
        execFileSync('g++', ['-std=' + std, '-pedantic-errors', '-Wall', '-Wextra', '-Werror', '-O2', '-I', poison, kmpSubmission, '-o', exe], { cwd: dir, env: { ...process.env, TEMP: dir, TMP: dir, TMPDIR: dir } });
        const result = execFileSync(exe, [], { cwd: dir, encoding: 'utf8' });
        assert.match(result, /kmp: PASS \(64897 exhaustive pairs, 2000 random cases, byte boundaries, million-length rebuilds\)/);
        assert.match(result, /sequence kmp: PASS/);
        if (['c++20', 'c++23'].includes(std)) assert.match(result, /span kmp: PASS/);
        execFileSync('g++', ['-std=' + std, '-pedantic-errors', '-Wall', '-Wextra', '-Werror', '-O2', '-I', poison, manacherSubmission, '-o', exe], { cwd: dir, env: { ...process.env, TEMP: dir, TMP: dir, TMPDIR: dir } });
        const manacherResult = execFileSync(exe, [], { cwd: dir, encoding: 'utf8' });
        assert.match(manacherResult, /manacher: PASS/);
        assert.match(manacherResult, /sequence manacher: PASS/);
        if (['c++20', 'c++23'].includes(std)) assert.match(manacherResult, /span manacher: PASS/);
        execFileSync('g++', ['-std=' + std, '-pedantic-errors', '-Wall', '-Wextra', '-Werror', '-O2', '-I', poison, usageSubmission, '-o', exe], { cwd: dir, env: { ...process.env, TEMP: dir, TMP: dir, TMPDIR: dir } });
        const expectedUsage = '1 7\nabacaba\n1 0\n7 12\n4 6\n0 0 0\n1 4' + (['c++20', 'c++23'].includes(std) ? '\n1 3' : '');
        assert.equal(execFileSync(exe, [], { cwd: dir, encoding: 'utf8' }).trim().replace(/\r/g, ''), expectedUsage);
        execFileSync('g++', ['-std=' + std, '-pedantic-errors', '-Wall', '-Wextra', '-Werror', '-O2', '-I', poison, zSubmission, '-o', exe], { cwd: dir, env: { ...process.env, TEMP: dir, TMP: dir, TMPDIR: dir } });
        const zResult = execFileSync(exe, [], { cwd: dir, encoding: 'utf8' });
        assert.match(zResult, /z_function: PASS/);
        assert.match(zResult, /extend: PASS/);
        if (['c++20', 'c++23'].includes(std)) assert.match(zResult, /span z: PASS/);
        execFileSync('g++', ['-std=' + std, '-pedantic-errors', '-Wall', '-Wextra', '-Werror', '-O2', '-I', poison, zUsageSubmission, '-o', exe], { cwd: dir, env: { ...process.env, TEMP: dir, TMP: dir, TMPDIR: dir } });
        const expectedZUsage = '7 0 1 0 3 0 1 4 6 7 3 0 1 2 3 1 0 2 1 0' + (['c++20', 'c++23'].includes(std) ? ' 3 1' : '');
        assert.equal(execFileSync(exe, [], { cwd: dir, encoding: 'utf8' }).trim().split(/\s+/).join(' '), expectedZUsage);
        execFileSync('g++', ['-std=' + std, '-pedantic-errors', '-Wall', '-Wextra', '-Werror', '-O2', '-I', poison, acSubmission, '-o', exe], { cwd: dir, env: { ...process.env, TEMP: dir, TMP: dir, TMPDIR: dir } });
        assert.match(execFileSync(exe, [], { cwd: dir, encoding: 'utf8' }), /ACAM PASS:/);
        execFileSync('g++', ['-std=' + std, '-pedantic-errors', '-Wall', '-Wextra', '-Werror', '-O2', '-I', poison, acUsageSubmission, '-o', exe], { cwd: dir, env: { ...process.env, TEMP: dir, TMP: dir, TMPDIR: dir } });
        assert.equal(execFileSync(exe, [], { cwd: dir, encoding: 'utf8' }).trim().split(/\s+/).join(' '), '3 2 3 4 0 2');
        console.log(`PASS: ${std} utils + LOCAL + exported KMP/Manacher/Z/ACAM full regression and Manacher/Z/ACAM Usage`);
    }
}

async function main() {
    text = original.replace('==7', '==7 /* 未保存 */');
    const expanded = await bridge.prepareDocument(vscode, document);
    assert.match(expanded, /inline int value/); assert.match(expanded, /中文注释/);
    assert.match(expanded, /未保存/); assert.doesNotMatch(expanded, /#include "|zoi:begin/);
    assert.equal(fs.readFileSync(file, 'utf8'), original);
    fs.writeFileSync(path.join(dir, 'submission.cpp'), expanded);
    const executable = path.join(dir, process.platform === 'win32' ? 'submission.exe' : 'submission');
    execFileSync('g++', ['-std=c++20', path.join(dir, 'submission.cpp'), '-o', executable], { cwd: dir, env: { ...process.env, TEMP: dir, TMP: dir, TMPDIR: dir } });
    execFileSync(executable, [], { cwd: dir });
    await utilsCompatibilityChecks();

    const exports = {}, messages = [];
    const context = { t: exports, c: exports, S: {}, P: { empty: true }, l: vscode, u: vscode,
        problem: { srcPath: file, url: 'https://codeforces.com/contest/1/problem/A' }, messages,
        v: { getProblemName: () => '1A' }, p: { getLanguageId: () => 54 },
        URL, f: { isCodeforcesUrl: u => u.hostname === 'codeforces.com' },
        require: () => bridge };
    vm.runInNewContext(transform(transform(cph, specifications[0].replacements), cphBrowser), context);
    await exports.submitToCodeForces();
    assert.equal(context.S.sourceCode, expanded); assert.equal(messages.length, 1);
    text = '#include "missing.h"\n';
    await exports.submitToCodeForces();
    assert.equal(context.S.empty, true); assert.equal(messages.length, 1); assert.equal(errors.length, 1);
    let vjCalls = 0;
    context.require = () => ({ ...bridge, submitVjudge: async (_, problem) => { assert.equal(problem.url, 'https://vjudge.net/contest/123#problem/A'); vjCalls++; } });
    context.problem.url = 'https://vjudge.net/contest/123#problem/A';
    await exports.submitToCodeForces(); assert.equal(vjCalls, 1); assert.equal(messages.length, 1); assert.equal(context.S.empty, true);
    let acCalls = 0;
    context.require = () => ({ ...bridge, submitAtcoder: async (_, problem) => { assert.equal(problem.url, 'https://atcoder.jp/contests/abc477/tasks/abc477_a'); acCalls++; } });
    context.problem.url = 'https://atcoder.jp/contests/abc477/tasks/abc477_a';
    await exports.submitToCodeForces(); assert.equal(acCalls, 1); assert.equal(messages.length, 1);
    const frontend = transform('result=e=>' + cphBrowserFrontend[0][0] + '"cph":"kattis":"other";', cphBrowserFrontend);
    const frontendContext = {}; vm.runInNewContext(frontend, frontendContext);
    for (const host of ['atcoder.jp', 'vjudge.net', 'vjudge.net.cn', 'codeforces.com']) assert.equal(frontendContext.result({ hostname: host }), 'cph');
    assert.equal(frontendContext.result({ hostname: 'atcoder.jp.evil.test' }), 'other');

    let submitted = 0, body;
    const lg = { t: {}, o: vscode, document, require: () => bridge,
        a: { q6: async (_, code) => { submitted++; body = code; } } };
    vm.runInNewContext(transform(luogu, specifications[1].replacements), lg);
    await assert.rejects(lg.t.submit(), /自动展开失败/); assert.equal(submitted, 0);
    text = original; await lg.t.submit(); assert.match(body, /inline int value/); assert.equal(submitted, 1);
    const lgNew = { t: document, s: vscode, e: {}, n: { id: 1, O2: true }, require: () => bridge, a: lg.a };
    vm.runInNewContext(transform('async function submit(){return ' + luogu418[0][0] + '} result=submit;', luogu418), lgNew);
    await lgNew.result(); assert.equal(submitted, 2); assert.match(body, /inline int value/);

    const inflight = bridge.prepareDocument(vscode, document);
    await assert.rejects(bridge.prepareDocument(vscode, document), /重复提交/);
    document.version++; await assert.rejects(inflight, /代码发生变化/);
    vscode.workspace.isTrusted = false;
    await assert.rejects(bridge.prepareDocument(vscode, document), /信任/);
    vscode.workspace.isTrusted = true;
    const py = { fileName: 'a.py', getText: () => 'print(1)' };
    assert.equal(await bridge.prepareDocument(vscode, py), 'print(1)');

    const extensionDir = path.join(dir, 'extensions'); fs.mkdirSync(extensionDir);
    fs.writeFileSync(path.join(extensionDir, 'extensions.json'), JSON.stringify(specifications.map((s, i) => ({ identifier: { id: s.id }, relativeLocation: 'plugin' + i }))));
    for (const [i, source] of [cph, luogu].entries()) {
        const target = path.join(extensionDir, 'plugin' + i); fs.mkdirSync(target);
        fs.writeFileSync(path.join(target, 'package.json'), JSON.stringify({ main: 'extension.js' }));
        fs.writeFileSync(path.join(target, 'extension.js'), source);
        if (!i) fs.writeFileSync(path.join(target, 'frontend.module.js'), 'const x=' + cphFrontend[0][0] + '1:2:3;');
    }
    install(extensionDir); install(extensionDir);
    assert.ok(install(extensionDir, 'check').every(r => r.installed));
    install(extensionDir, 'uninstall');
    assert.equal(fs.readFileSync(path.join(extensionDir, 'plugin0/frontend.module.js'), 'utf8'), 'const x=' + cphFrontend[0][0] + '1:2:3;');
    for (const [i, source] of [cph, luogu].entries()) assert.equal(fs.readFileSync(path.join(extensionDir, 'plugin' + i, 'extension.js'), 'utf8'), source);
    // Upgrade from the already installed CF snapshot patch and Luogu's new document API.
    fs.writeFileSync(path.join(extensionDir, 'plugin0/extension.js'), transform(cph, specifications[0].replacements));
    const newLuogu = 'async function submit(){return ' + luogu418[0][0] + '}';
    fs.writeFileSync(path.join(extensionDir, 'plugin1/extension.js'), newLuogu);
    install(extensionDir); assert.ok(install(extensionDir, 'check').every(r => r.installed)); install(extensionDir, 'uninstall');
    assert.equal(fs.readFileSync(path.join(extensionDir, 'plugin1/extension.js'), 'utf8'), newLuogu);
    // Upgrade the existing VJudge route/button without stacking patches; uninstall preserves unrelated fixes.
    const oldBackend = transform(transform(cph, specifications[0].replacements), cphVjudge);
    const oldFrontend = transform('const x=' + cphFrontend[0][0] + '1:2:3;', cphFrontend);
    fs.writeFileSync(path.join(extensionDir, 'plugin0/extension.js'), oldBackend + '\n// unrelated patch');
    fs.writeFileSync(path.join(extensionDir, 'plugin0/frontend.module.js'), oldFrontend);
    assert.equal(install(extensionDir, 'check')[0].installed, false);
    install(extensionDir); install(extensionDir);
    assert.ok(install(extensionDir, 'check').every(r => r.installed));
    install(extensionDir, 'uninstall');
    assert.equal(fs.readFileSync(path.join(extensionDir, 'plugin0/extension.js'), 'utf8'), cph + '\n// unrelated patch');
    assert.equal(fs.readFileSync(path.join(extensionDir, 'plugin0/frontend.module.js'), 'utf8'), 'const x=' + cphFrontend[0][0] + '1:2:3;');
    const changed = path.join(extensionDir, 'plugin1/extension.js'); fs.writeFileSync(changed, '// unknown version');
    assert.throws(() => install(extensionDir), /不匹配/);
    assert.equal(fs.readFileSync(path.join(extensionDir, 'plugin0/extension.js'), 'utf8'), cph + '\n// unrelated patch');
    assert.equal(fs.readFileSync(file, 'utf8'), original);
    assert.ok(!fs.readdirSync(dir).some(n => /zoi\.(state|pending)|\.lock$/.test(n)));
    console.log('PASS: snapshot export + UTF-8 + utils/KMP/Manacher/Z C++11/14/17/20/23 compilation/runtime/old-header checks + CF/VJudge/AtCoder routing + Luogu 4.16/4.18 adapters + fail-closed + concurrent edit + install/idempotence/uninstall/version mismatch');
}

main().then(() => {
    const resolved = fs.realpathSync(dir);
    assert.ok(resolved.startsWith(fs.realpathSync(parent) + path.sep));
    fs.rmSync(resolved, { recursive: true });
}).catch(error => { console.error(error); console.error('Failure fixture retained: ' + dir); process.exitCode = 1; });
