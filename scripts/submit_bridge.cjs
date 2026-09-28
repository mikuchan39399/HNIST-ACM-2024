'use strict';

const path = require('node:path');
const { execFile } = require('node:child_process');
const root = path.resolve(__dirname, '..');
const pending = new Set();

// Return submission text from a document snapshot. Never replace editor/source content.
async function prepareDocument(vscode, document) {
    if (!/\.cpp$/i.test(document.fileName)) return document.getText();
    if (!vscode.workspace.isTrusted) throw new Error('请先信任当前工作区，再使用自动展开提交。');
    if (document.isUntitled || document.uri.scheme !== 'file') throw new Error('请先将代码保存为 .cpp 文件。');
    const file = document.fileName;
    if (pending.has(file)) throw new Error('这份代码正在展开，请勿重复提交。');
    pending.add(file);
    try {
        const version = document.version;
        const input = document.getText();
        const executable = process.platform === 'win32'
            ? path.join(process.env.SystemRoot || 'C:\\Windows', 'System32/WindowsPowerShell/v1.0/powershell.exe')
            : 'pwsh';
        const code = await new Promise((resolve, reject) => {
            const child = execFile(executable, [
                '-NoProfile', '-NonInteractive', '-ExecutionPolicy', 'Bypass',
                '-File', path.join(__dirname, 'zoi.ps1'), 'export', file, '-FromStdin',
            ], { cwd: root, windowsHide: true, encoding: 'utf8', timeout: 60000, maxBuffer: 32 * 1024 * 1024 },
            (error, stdout, stderr) => {
                if (error) reject(new Error('自动展开失败，已停止提交：\n' + (stderr || stdout || error.message).trim().slice(0, 2000)));
                else resolve(stdout.replace(/^\uFEFF/, ''));
            });
            child.stdin.on('error', () => {}); // execFile reports process failure through its callback.
            child.stdin.end(input, 'utf8');
        });
        if (document.isClosed || document.version !== version || document.getText() !== input) {
            throw new Error('展开期间代码发生变化，已停止提交；请再次点击提交。');
        }
        return code;
    } finally {
        pending.delete(file);
    }
}

async function prepareFile(vscode, file) {
    const document = await vscode.workspace.openTextDocument(vscode.Uri.file(file));
    return prepareDocument(vscode, document);
}

const vjudge = require('./vjudge_bridge.cjs');
const atcoder = require('./atcoder_bridge.cjs');
module.exports = { prepareDocument, prepareFile, isVjudge: vjudge.isVjudge,
    isAtcoder: atcoder.isAtcoder, submitAtcoder: (vscode, problem) => atcoder.submit(vscode, problem, prepareDocument),
    submitVjudge: (vscode, problem) => vjudge.submit(vscode, problem, prepareDocument) };
