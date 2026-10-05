'use strict';
const fs = require('node:fs');
const path = require('node:path');
const crypto = require('node:crypto');

const same = (a, b) => process.platform === 'win32' ? path.resolve(a).toLowerCase() === path.resolve(b).toLowerCase() : path.resolve(a) === path.resolve(b);
const isSource = name => /\.(?:cpp|cc|cxx|c)$/i.test(name) && !/\.zoi\.cpp$/i.test(name);
function stat(file) { try { return fs.lstatSync(file); } catch (e) { if (e.code === 'ENOENT') return null; throw e; } }
function checkPath(file) {
    for (let p = path.resolve(file); ; p = path.dirname(p)) {
        if (stat(p)?.isSymbolicLink()) throw Error('路径经过符号链接或目录联接，请使用真实目录：' + p);
        if (path.dirname(p) === p) break;
    }
}
function read(file) {
    checkPath(file);
    const info = stat(file);
    if (!info?.isFile()) throw Error('文件不存在或不是普通文件：' + file);
    return fs.readFileSync(file);
}
function list(root) {
    checkPath(root);
    if (path.basename(root).toLowerCase() === 'trash heap') throw Error('请在原刷题文件夹中运行，不能再次清理垃圾堆。');
    return fs.readdirSync(root, { withFileTypes: true }).filter(e => e.isFile() && isSource(e.name)).map(e => path.join(root, e.name)).sort((a, b) => a.localeCompare(b));
}
function plan(root, files, normalize = p => p) {
    root = path.resolve(root);
    const available = list(root), items = [], destinations = new Set();
    const add = (from, to, after) => {
        checkPath(to);
        if (stat(to)) throw Error('垃圾堆已有同名文件，请先整理：' + to);
        const key = process.platform === 'win32' ? to.toLowerCase() : to;
        if (destinations.has(key)) throw Error('多个关联文件指向同一位置，请先核对 CPH 数据：' + to);
        destinations.add(key);
        const before = read(from);
        items.push({ from, to, before, after: after || before });
    };
    for (const file of files) {
        if (!available.some(p => same(p, file))) throw Error('只允许选择当前工作区根目录的 C/C++ 文件：' + file);
        const name = path.basename(file), stem = name.slice(0, -path.extname(name).length);
        const target = normalize(path.join(root, 'trash heap', name));
        for (const pending of [stem + '.zoi.pending.json', name + '.zoi.lock', name + '.zoi-write.tmp', stem + '.zoi.state.json.zoi-write.tmp']) {
            if (stat(path.join(root, pending))) throw Error('题目存在正在运行或未恢复的 ZOI 操作，请先完成恢复：' + name);
        }
        add(file, target);
        for (const suffix of ['.exe', '.out', '.o', '.zoi.cpp', '.zoi.sha', '.zoi.state.json']) {
            const extra = path.join(root, stem + suffix);
            if (stat(extra)) add(extra, path.join(root, 'trash heap', stem + suffix));
        }
        const cph = path.join(root, '.cph');
        checkPath(cph);
        if (!stat(cph)) continue;
        for (const entry of fs.readdirSync(cph)) {
            if (!entry.startsWith('.' + name + '_') || !entry.endsWith('.prob')) continue;
            const from = path.join(cph, entry);
            let data;
            try { data = JSON.parse(read(from).toString('utf8').replace(/^\uFEFF/, '')); }
            catch (e) { throw Error('CPH 数据无法读取，保留原题目：' + from + '；' + e.message); }
            if (typeof data.srcPath !== 'string' || !same(data.srcPath, file)) continue;
            data.srcPath = target;
            if (data.local && typeof data.url === 'string' && same(data.url, file)) data.url = target;
            const hash = crypto.createHash('md5').update(target).digest('hex');
            add(from, path.join(root, 'trash heap', '.cph', '.' + name + '_' + hash + '.prob'), Buffer.from(JSON.stringify(data)));
        }
    }
    return items;
}
function unchanged(file, data) { try { return read(file).equals(data); } catch { return false; } }
function execute(items) {
    const created = [], removed = [], dirs = [];
    const mkdir = dir => {
        checkPath(dir);
        if (stat(dir)) { if (!stat(dir).isDirectory()) throw Error('目标不是目录：' + dir); return; }
        mkdir(path.dirname(dir)); fs.mkdirSync(dir); dirs.push(dir);
    };
    try {
        for (const item of items) {
            checkPath(item.to);
            if (!unchanged(item.from, item.before)) throw Error('文件已变化，请重新选择：' + item.from);
            if (stat(item.to)) throw Error('目标已存在，未覆盖：' + item.to);
        }
        // All destination copies are complete before any source is removed.
        for (const item of items) {
            mkdir(path.dirname(item.to));
            const fd = fs.openSync(item.to, 'wx'); created.push(item);
            try { fs.writeFileSync(fd, item.after); fs.fsyncSync(fd); } finally { fs.closeSync(fd); }
        }
        for (const item of items) {
            if (!unchanged(item.from, item.before) || !unchanged(item.to, item.after)) throw Error('移动期间文件发生变化，停止操作：' + item.from);
        }
        for (const item of items) {
            if (!unchanged(item.from, item.before)) throw Error('移动期间源文件发生变化：' + item.from);
            fs.unlinkSync(item.from); removed.push(item);
        }
    } catch (error) {
        const failures = [];
        for (const item of removed) {
            try { fs.writeFileSync(item.from, item.before, { flag: 'wx' }); }
            catch { failures.push(item.from); }
        }
        for (const item of created) {
            try {
                if (unchanged(item.from, item.before) && unchanged(item.to, item.after)) fs.unlinkSync(item.to);
                else failures.push(item.to);
            } catch { failures.push(item.to); }
        }
        for (const dir of dirs.reverse()) { try { fs.rmdirSync(dir); } catch {} }
        throw Error(error.message + (failures.length ? '\n部分文件保留在以下位置，请核对后处理：\n' + failures.join('\n') : '\n本次操作已撤回。'));
    }
}
function reverse(items) { return items.map(i => ({ from: i.to, to: i.from, before: i.after, after: i.before })); }

function register(context, vscode, onMoved) {
    let busy = false;
    context.subscriptions.push(vscode.commands.registerCommand('zoi.trashProblems', async () => {
        if (busy) return;
        busy = true;
        try {
            if (!vscode.workspace.isTrusted) throw Error('请先信任刷题工作区。');
            const folders = (vscode.workspace.workspaceFolders || []).filter(f => f.uri.scheme === 'file');
            if (!folders.length) throw Error('请先在 VS Code 打开刷题文件夹。');
            const active = vscode.window.activeTextEditor?.document.uri;
            let folder = active && vscode.workspace.getWorkspaceFolder(active);
            if (!folder || folder.uri.scheme !== 'file') folder = folders.length === 1 ? folders[0] : await vscode.window.showQuickPick(folders.map(f => ({ label: f.name, folder: f })), { title: '选择刷题文件夹' }).then(x => x?.folder);
            if (!folder) return;
            if (vscode.workspace.getConfiguration('cph', folder.uri).get('general.saveLocation', '')) throw Error('当前配置了自定义 CPH 数据目录；此入口仅支持题目旁的 .cph，请先恢复默认保存位置。');
            const files = list(folder.uri.fsPath);
            if (!files.length) { await vscode.window.showInformationMessage('当前工作区根目录没有可整理的 C/C++ 题目。'); return; }
            const selected = await vscode.window.showQuickPick(files.map(file => ({ label: path.basename(file), file })), {
                canPickMany: true, title: '把题目移入 trash heap', placeHolder: '输入筛选，勾选复选框，回车移动；Esc 取消（仅当前工作区根目录）',
            });
            if (!selected?.length) return;
            const items = plan(folder.uri.fsPath, selected.map(x => x.file), p => vscode.Uri.file(p).fsPath);
            const dirty = entries => (vscode.workspace.textDocuments || []).find(d => d.isDirty && d.uri.scheme === 'file' && entries.some(i => same(i.from, d.uri.fsPath) || same(i.to, d.uri.fsPath)));
            const unsaved = dirty(items);
            if (unsaved) throw Error('请先保存或撤销未保存的修改，再移动：' + path.basename(unsaved.uri.fsPath));
            execute(items); onMoved(selected.map(x => x.file));
            const tabs = vscode.window.tabGroups.all.flatMap(g => g.tabs).filter(t => t.input?.uri?.scheme === 'file' && items.some(i => same(i.from, t.input.uri.fsPath)));
            try { if (tabs.length) await vscode.window.tabGroups.close(tabs, true); } catch {}
            const answer = await vscode.window.showInformationMessage(`已将 ${selected.length} 道题及关联文件移入 trash heap。`, '撤回本次');
            if (answer === '撤回本次') {
                const undo = reverse(items);
                if (dirty(undo)) throw Error('有未保存的修改，请先保存；垃圾堆中的文件保持原样。');
                execute(undo);
                await vscode.window.showInformationMessage('已撤回本次移动，源码和 CPH 样例已恢复。');
            }
        } catch (e) { await vscode.window.showErrorMessage('ZOI 整理题目：' + e.message); }
        finally { busy = false; }
    }));
}
module.exports = { list, plan, execute, reverse, register };
