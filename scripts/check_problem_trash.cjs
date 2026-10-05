'use strict';
const assert = require('node:assert/strict');
const fs = require('node:fs');
const path = require('node:path');
const crypto = require('node:crypto');
const trash = require('./statement-extension/trash.cjs');
const core = require('./statement-extension/core.cjs');

async function main() {
    const base = path.resolve(__dirname, '../.zoi-checks/codex-work');
    fs.mkdirSync(base, { recursive: true });
    const root = fs.mkdtempSync(path.join(base, 'problem-trash-'));
    const file = name => path.join(root, name);
    const put = (name, data) => fs.writeFileSync(file(name), data);
    const read = name => fs.readFileSync(file(name), 'utf8');
    try {
        fs.mkdirSync(file('.cph')); fs.mkdirSync(file('subfolder'));
        put('题目 & [A]!.cpp', '\ufeff// my solution\r\n'); put('B.cpp', '// keep'); put('test.h', '// header'); put('subfolder/C.cpp', '// nested');
        put('题目 & [A]!.zoi.state.json', '{"saved":"state"}'); put('题目 & [A]!.exe', 'binary'); put('B.zoi.cpp', '// generated');
        const source = file('题目 & [A]!.cpp');
        const data = { name: 'A', srcPath: source, url: 'https://codeforces.com/contest/4/problem/A', tests: [{ input: '8\n', output: 'YES\n' }] };
        const original = '\ufeff' + JSON.stringify(data, null, 2) + '\r\n';
        put('.cph/.题目 & [A]!.cpp_old.prob', original);
        assert.deepEqual(new Set(trash.list(root).map(p => path.basename(p))), new Set(['B.cpp', '题目 & [A]!.cpp']));
        const items = trash.plan(root, [source]);
        assert.equal(items.length, 4);
        trash.execute(items);
        assert.ok(!fs.existsSync(source)); assert.equal(read('trash heap/题目 & [A]!.cpp'), '\ufeff// my solution\r\n');
        assert.equal(read('B.cpp'), '// keep'); assert.equal(read('test.h'), '// header'); assert.equal(read('subfolder/C.cpp'), '// nested');
        const target = file('trash heap/题目 & [A]!.cpp');
        const hash = crypto.createHash('md5').update(target).digest('hex');
        const moved = JSON.parse(read('trash heap/.cph/.题目 & [A]!.cpp_' + hash + '.prob'));
        assert.equal(moved.srcPath, target); assert.deepEqual(moved.tests, data.tests); assert.equal(moved.url, data.url);
        assert.deepEqual(core.readProblem(target).tests, data.tests);
        trash.execute(trash.reverse(items));
        assert.equal(read('.cph/.题目 & [A]!.cpp_old.prob'), original);
        assert.equal(read('题目 & [A]!.exe'), 'binary');

        put('trash heap/B.cpp', '// existing');
        assert.throws(() => trash.plan(root, [file('B.cpp')]), /同名/); assert.equal(read('trash heap/B.cpp'), '// existing');
        assert.throws(() => trash.plan(root, [source, file('B.cpp')]), /同名/); assert.ok(fs.existsSync(source));
        assert.throws(() => trash.plan(root, [file('subfolder/C.cpp')]), /根目录/);
        assert.throws(() => trash.list(file('trash heap')), /垃圾堆/);
        put('题目 & [A]!.zoi.pending.json', '{}');
        assert.throws(() => trash.plan(root, [source]), /未恢复/); fs.unlinkSync(file('题目 & [A]!.zoi.pending.json'));
        const stale = trash.plan(root, [source]); put('题目 & [A]!.cpp', '// changed');
        assert.throws(() => trash.execute(stale), /已变化/); assert.equal(read('题目 & [A]!.cpp'), '// changed');
        assert.ok(!fs.existsSync(target));

        const lateCollision = trash.plan(root, [source]);
        put('trash heap/题目 & [A]!.cpp', '// another writer');
        assert.throws(() => trash.execute(lateCollision), /目标已存在/);
        assert.equal(read('trash heap/题目 & [A]!.cpp'), '// another writer');
        fs.unlinkSync(target);

        const failed = trash.plan(root, [source]), unlink = fs.unlinkSync;
        let failures = 0;
        fs.unlinkSync = p => { if (p === file('题目 & [A]!.exe') && failures++ === 0) throw Error('injected sharing violation'); return unlink(p); };
        try { assert.throws(() => trash.execute(failed), /injected sharing violation.*\n本次操作已撤回/); }
        finally { fs.unlinkSync = unlink; }
        for (const i of failed) { assert.deepEqual(fs.readFileSync(i.from), i.before); assert.ok(!fs.existsSync(i.to)); }

        put('Local.cpp', '// local');
        put('.cph/.Local.cpp_local.prob', JSON.stringify({ name: 'Local', local: true, srcPath: file('Local.cpp'), url: file('Local.cpp'), tests: [] }));
        const localPlan = trash.plan(root, [file('Local.cpp')]); trash.execute(localPlan);
        const localData = JSON.parse(fs.readFileSync(localPlan.find(i => i.to.endsWith('.prob')).to, 'utf8'));
        assert.equal(localData.url, file('trash heap/Local.cpp')); assert.equal(localData.srcPath, localData.url);
        put('trash heap/Local.cpp', '// edited after moving');
        assert.throws(() => trash.execute(trash.reverse(localPlan)), /已变化/);
        assert.equal(read('trash heap/Local.cpp'), '// edited after moving'); assert.ok(!fs.existsSync(file('Local.cpp')));

        const link = file('linked');
        fs.symlinkSync(file('subfolder'), link, process.platform === 'win32' ? 'junction' : 'dir');
        try { assert.throws(() => trash.list(link), /联接/); } finally { fs.unlinkSync(link); }
        put('.cph/.题目 & [A]!.cpp_old.prob', '{bad json');
        assert.throws(() => trash.plan(root, [source]), /CPH 数据/); put('.cph/.题目 & [A]!.cpp_old.prob', original);

        // The command must preserve dirty buffers and cancel without moving anything.
        let run, selection, dirty = true, notices = [], errors = [], closed = 0;
        const uri = p => ({ scheme: 'file', fsPath: p });
        const folder = { name: 'test', uri: uri(root) };
        const document = { uri: uri(source), get isDirty() { return dirty; } };
        const vscode = {
            Uri: { file: uri },
            commands: { registerCommand: (_, fn) => { run = fn; return { dispose() {} }; } },
            workspace: { isTrusted: true, workspaceFolders: [folder], getWorkspaceFolder: () => folder, getConfiguration: () => ({ get: () => '' }), textDocuments: [document] },
            window: {
                activeTextEditor: { document }, showQuickPick: async () => selection,
                showErrorMessage: async e => errors.push(e), showInformationMessage: async s => { notices.push(s); },
                tabGroups: { all: [{ tabs: [{ input: { uri: uri(source) } }] }], close: async tabs => { closed += tabs.length; } },
            },
        };
        trash.register({ subscriptions: [] }, vscode, () => {});
        await run(); assert.ok(fs.existsSync(source));
        selection = [{ file: source }]; await run(); assert.match(errors.pop(), /保存/); assert.ok(fs.existsSync(source));
        dirty = false;
        vscode.window.showInformationMessage = async s => { notices.push(s); return s.startsWith('已将') ? '撤回本次' : undefined; };
        await run(); assert.equal(closed, 1); assert.equal(errors.length, 0); assert.ok(fs.existsSync(source));
        assert.equal(read('.cph/.题目 & [A]!.cpp_old.prob'), original);
        assert.ok(notices.some(s => s.includes('已撤回本次')));
        console.log('[PASS] problem trash: selection/cancel, dirty buffers, CPH path+hash, exact undo, collisions, rollback, linked paths, pending writes and unrelated files');
    } finally {
        assert.equal(path.dirname(root), base);
        assert.ok(!fs.lstatSync(root).isSymbolicLink());
        fs.rmSync(root, { recursive: true, force: true });
    }
}
main().catch(e => { console.error(e); process.exitCode = 1; });
