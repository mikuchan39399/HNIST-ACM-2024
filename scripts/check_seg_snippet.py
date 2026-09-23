"""Check the shipped snippet, isolated installation, and real segment-tree use."""
import argparse
import json
import os
from pathlib import Path
import shutil
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parent.parent


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--shell', default='pwsh')
    parser.add_argument('--compiler', default='g++')
    parser.add_argument('--install-only', action='store_true')
    args = parser.parse_args()
    parent = ROOT / '.zoi-checks' / 'codex-work'
    parent.mkdir(parents=True, exist_ok=True)
    work = Path(tempfile.mkdtemp(prefix='seg-snippet-', dir=parent)).resolve()
    env = os.environ.copy()
    env.update({key: str(work) for key in ('TEMP', 'TMP', 'TMPDIR')})
    source = ROOT / 'scripts/snippets/seg-add-sum.json'
    definitions = json.loads(source.read_text(encoding='utf-8'))
    snippet, = definitions.values()
    assert snippet['scope'] == 'cpp' and snippet['prefix'] == 'zoisegaddsum'
    assert snippet['body'][-1] == '$0'
    profile = work / 'profile with spaces' / 'snippets'
    profile.mkdir(parents=True)
    unrelated = profile / 'cpp.json'
    unrelated.write_text('{"keep": {"prefix": "keep", "body": ["hello"]}}', encoding='utf-8')
    saved = unrelated.read_bytes()
    target = profile / 'zoi-seg-add-sum.code-snippets'

    def run(command, ok=True):
        result = subprocess.run(command, cwd=ROOT, env=env, capture_output=True,
                                text=True, encoding='utf-8', errors='replace', timeout=120)
        if (result.returncode == 0) != ok:
            raise RuntimeError(result.stdout + result.stderr)
        return result

    install = [args.shell, '-NoProfile', '-ExecutionPolicy', 'Bypass', '-File',
               str(ROOT / 'scripts/install-seg-snippet.ps1'), '-SnippetsDir', str(profile)]
    run(install)
    assert json.loads(target.read_text(encoding='utf-8')) == definitions
    first = target.read_bytes()
    run(install)
    assert target.read_bytes() == first and unrelated.read_bytes() == saved
    target.write_text('{"custom": true}', encoding='utf-8')
    run(install, ok=False)
    run(install + ['-Uninstall'], ok=False)
    assert target.read_text(encoding='utf-8') == '{"custom": true}'
    target.write_bytes(first)
    run(install + ['-Uninstall'])
    run(install + ['-Uninstall'])
    assert not target.exists() and unrelated.read_bytes() == saved

    if not args.install_only:
        # Compile the actual snippet body, not a separately maintained copy.
        body = '\n'.join(snippet['body'][:-1])
        assert '$' not in body
        code = '#include "seg.h"\n#include "dynamicSeg.h"\n' + body + r'''
int main()
{
    assert((Info{} + Info(7)).sum == 7 && (Info(7) + Info{}).len == 1);
    Tag t{3}; t.apply({-5}); assert(t.add == -2); t.clear(); assert(!t.has_tag());
    mt19937 rng(42);
    SegTree<Info, Tag> seg(200000);
    DySegTree<Info, Tag> dyn(64, 500000);
    for (int tc = 0; tc < 300; tc++)
    {
        int n = 1 + int(rng() % 64);
        VLL a(n + 1);
        vector<Info> init(n + 1);
        for (int i = 1; i <= n; i++)
        {
            a[i] = tc % 2 ? 0 : int(rng() % 201) - 100;
            init[i] = Info(a[i]);
        }
        seg.build(init);
        if (tc % 2) dyn.init(n); else dyn.build(init);
        for (int op = 0; op < 300; op++)
        {
            int l = 1 + int(rng() % n), r = 1 + int(rng() % n);
            if (l > r) swap(l, r);
            if (op % 2)
            {
                LL expected = accumulate(a.begin() + l, a.begin() + r + 1, 0LL);
                assert(seg.query(l, r).sum == expected && dyn.query(l, r).sum == expected);
            }
            else
            {
                LL delta = int(rng() % 201) - 100;
                if (op == 0) delta = 1000000000LL;
                seg.modify(l, r, {delta}); dyn.modify(l, r, {delta});
                for (int i = l; i <= r; i++) a[i] += delta;
            }
        }
    }
    int n = 200000;
    vector<Info> init(n + 1, Info(0)); init[0] = Info{};
    seg.build(init); dyn.build(init);
    LL value = 0;
    for (int i = 1; i <= n; i++)
    {
        LL delta = i % 2 ? 1000000000LL : -1000000000LL;
        value += delta;
        seg.modify(1, n, {delta}); dyn.modify(1, n, {delta});
        assert(seg.query(i, n).sum == value * (n - i + 1));
        assert(dyn.query(i, n).sum == value * (n - i + 1));
    }
}
'''
        cpp = work / 'snippet_check.cpp'
        exe = work / ('snippet_check.exe' if os.name == 'nt' else 'snippet_check')
        cpp.write_text(code, encoding='utf-8')
        run([args.compiler, '-std=c++20', '-O2', '-Wall', '-Wextra', '-Werror', '-UNDEBUG',
             '-I', str(ROOT / 'zoi'), str(cpp), '-o', str(exe)])
        run([str(exe)])
    # Only remove this invocation's verified workspace; keep failures for diagnosis.
    if work.parent != parent.resolve() or not work.name.startswith('seg-snippet-'):
        raise RuntimeError('Unexpected cleanup path')
    shutil.rmtree(work)
    print('[PASS] snippet install/reinstall/conflict/uninstall; ' +
          ('installation only' if args.install_only else '300 random cases and n=200000 with both trees'))


if __name__ == '__main__':
    main()
