#!/usr/bin/env python3
"""Exercise real layout migration in disposable repositories, without an LLM."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest

from source_lookup import engine, suite

ROOT = Path(__file__).resolve().parent.parent
SHELL = 'pwsh'
COMPILER = 'g++'


class LayoutTests(unittest.TestCase):
    def setUp(self):
        parent = ROOT / '.zoi-checks'
        parent.mkdir(exist_ok=True)
        self.root = Path(tempfile.mkdtemp(prefix='layout-test-', dir=parent)).resolve()
        self.assertEqual(self.root.parent, parent.resolve())
        self.addCleanup(self.cleanup_fixture)
        self.put('scripts/sync_layout.ps1', (ROOT / 'scripts/sync_layout.ps1').read_text())
        self.put('zoi/_catalog.txt', 'core\talgorithms/base/core.cpp\npart\talgorithms/old/part.cpp\n')
        self.put('algorithms/base/core.cpp', '// zoi: core\n#pragma once\ninline int core() { return 7; }\n')
        self.put('algorithms/old/part.cpp', '// zoi: part\n#include "../base/core.cpp"\ninline int part() { return core(); }\n')
        self.put('algorithms/old/probe_check.cpp', '#include "part.cpp"\nint main() { return part() != 7; }\n')
        self.put('rules/verification.json', json.dumps({'schema': 1, 'coverage': [dict(
            template='algorithms/old/part.cpp', suite='algorithms/old/probe_check.cpp',
            api=['part'], oracle='constant', cases=['7'], summary='untouched', limitations='fixture')]}))
        self.run_sync()

    def cleanup_fixture(self):
        # This unique fixture was created by this test; never follow a reparse point.
        self.assertEqual(self.root.parent, (ROOT / '.zoi-checks').resolve())
        self.assertTrue(self.root.name.startswith('layout-test-'))
        for p in self.root.rglob('*'):
            self.assertFalse(p.is_symlink())
        shutil.rmtree(self.root)

    def put(self, name, text):
        p = self.root / name
        p.parent.mkdir(parents=True, exist_ok=True)
        p.write_text(text, encoding='utf-8')

    def read(self, name):
        return (self.root / name).read_text(encoding='utf-8')

    def move(self, old, new):
        src, dst = self.root / old, self.root / new
        self.assertTrue(str(src.resolve()).startswith(str(self.root) + os.sep))
        self.assertTrue(str(dst.resolve()).startswith(str(self.root) + os.sep))
        dst.parent.mkdir(parents=True, exist_ok=True)
        src.rename(dst)

    def hashes(self):
        return {p.relative_to(self.root).as_posix(): hashlib.sha256(p.read_bytes()).hexdigest()
                for p in self.root.rglob('*') if p.is_file()}

    def run_sync(self, *args, success=True):
        run = subprocess.run([SHELL, '-NoProfile', '-ExecutionPolicy', 'Bypass', '-File', str(self.root / 'scripts/sync_layout.ps1'),
                              '-NoRefresh', *args], cwd=self.root, stdout=subprocess.PIPE,
                             stderr=subprocess.STDOUT, timeout=30)
        self.assertEqual(run.returncode == 0, success, run.stdout.decode('utf-8', errors='replace'))
        return run

    def test_relocate_depth_and_registered_suite_compile(self):
        self.move('algorithms/old', 'algorithms/new/deep')
        self.run_sync()
        self.assertIn('#include "../../base/core.cpp"', self.read('algorithms/new/deep/part.cpp'))
        self.assertIn('algorithms/new/deep/probe_check.cpp', self.read('rules/verification.json'))
        self.assertIn('algorithms/new/deep/part.cpp', self.read('zoi/part.h'))
        self.assertEqual(suite(self.root, 'probe_check.cpp'), 'algorithms/new/deep/probe_check.cpp')
        self.assertEqual(engine(self.root, 'part'), 'algorithms/new/deep/part.cpp')
        self.put('probe.cpp', self.read('algorithms/new/deep/probe_check.cpp').replace('"part.cpp"', '"zoi/part.h"'))
        env = dict(os.environ, TEMP=str(self.root), TMP=str(self.root), TMPDIR=str(self.root))
        exe = self.root / 'probe.exe'
        subprocess.run([COMPILER, '-std=c++20', '-Wall', '-Wextra', '-Werror',
                        str(self.root / 'probe.cpp'), '-o', str(exe)], cwd=self.root, env=env, check=True, timeout=60)
        subprocess.run([str(exe)], cwd=self.root, env=env, check=True, timeout=10)
        self.run_sync('-Check')

    def test_incoming_links_comments_and_evidence(self):
        old = 'algorithms/old/part.cpp'
        self.put('README.md', '[inline](<'+old+'#anchor>)\n[ref]: '+old+'\n'
                 '```cpp\n[example]('+old+')\n```\n`[example]('+old+')`\n<!-- [example]('+old+') -->\n')
        self.put('records/verification/runs/evidence.json', '{"path":"'+old+'"}')
        self.put('records/tooling/rule_history.md', '[frozen]('+old+')\n')
        self.put('algorithms/base/comment.cpp', '/*\n#include "../old/part.cpp"\n*/\n'
                 'const char* raw = R"x(\n#include "../old/part.cpp"\n)x";\n')
        before = self.hashes()
        self.move(old, 'algorithms/new space/renamed(2).cpp')
        self.run_sync()
        readme = self.read('README.md')
        self.assertIn('algorithms/new%20space/renamed(2).cpp#anchor', readme)
        self.assertIn('[ref]: algorithms/new%20space/renamed(2).cpp', readme)
        self.assertIn('[example]('+old+')', readme)
        after = self.hashes()
        for name in ['records/verification/runs/evidence.json', 'records/tooling/rule_history.md', 'algorithms/base/comment.cpp']:
            self.assertEqual(before[name], after[name])

    def test_noop_preserves_mtimes_and_check_is_read_only(self):
        times = {p: p.stat().st_mtime_ns for p in self.root.rglob('*') if p.is_file()}
        self.run_sync(); self.run_sync('-Check')
        self.assertEqual(times, {p: p.stat().st_mtime_ns for p in times})
        self.move('algorithms/old/part.cpp', 'algorithms/new/part.cpp')
        before = self.hashes()
        self.run_sync('-Check', success=False)
        self.assertEqual(before, self.hashes())

    def test_new_identity_is_registered_without_invented_semantics(self):
        self.put('algorithms/new/fresh.cpp', '// zoi: fresh\n#pragma once\n')
        before = self.read('rules/verification.json')
        self.run_sync()
        self.assertIn('fresh\talgorithms/new/fresh.cpp', self.read('zoi/_catalog.txt'))
        self.assertTrue((self.root / 'zoi/fresh.h').exists())
        self.assertEqual(before, self.read('rules/verification.json'))

    def test_registered_note_follows_unique_name(self):
        self.put('zoi/_catalog.txt', self.read('zoi/_catalog.txt')+'^note\talgorithms/old/note.txt\n')
        self.put('algorithms/new/note.txt', 'A formula, not executable code.\n')
        self.run_sync()
        self.assertIn('^note\talgorithms/new/note.txt', self.read('zoi/_catalog.txt'))
        self.assertFalse((self.root / 'zoi/note.h').exists())

    def test_duplicate_id_refuses_without_writes(self):
        self.put('algorithms/copy.cpp', self.read('algorithms/old/part.cpp'))
        before = self.hashes()
        self.run_sync(success=False)
        self.assertEqual(before, self.hashes())
        with self.assertRaises(ValueError): engine(self.root, 'part')

    def test_ambiguous_suite_refuses_without_writes(self):
        self.move('algorithms/old/probe_check.cpp', 'algorithms/new/probe_check.cpp')
        self.put('algorithms/copy/probe_check.cpp', 'int main() {}\n')
        before = self.hashes()
        self.run_sync(success=False)
        self.assertEqual(before, self.hashes())
        with self.assertRaises(ValueError): suite(self.root, 'probe_check.cpp')

    def test_deleted_engine_and_replacement_refuse(self):
        self.move('algorithms/old/part.cpp', 'algorithms/new/part.cpp')
        self.put('algorithms/old/part.cpp', '// unrelated replacement\n')
        before = self.hashes()
        self.run_sync(success=False)
        self.assertEqual(before, self.hashes())
        self.put('algorithms/new/part.cpp', '// removed identity\n')
        self.run_sync(success=False)

    def test_ambiguous_include_refuses_without_writes(self):
        self.move('algorithms/old/part.cpp', 'algorithms/new/deep/part.cpp')
        self.put('algorithms/new/base/core.cpp', '// unrelated file\n')
        before = self.hashes()
        self.run_sync(success=False)
        self.assertEqual(before, self.hashes())

    def test_expand_entry_runs_sync_automatically(self):
        self.put('scripts/zoi.ps1', (ROOT / 'scripts/zoi.ps1').read_text())
        self.put('problem.cpp', '#include "part.h"\nint main() { return part()!=7; }\n')
        self.move('algorithms/old/part.cpp', 'algorithms/new/deep/part.cpp')
        run = subprocess.run([SHELL, '-NoProfile', '-ExecutionPolicy', 'Bypass', '-File',
                              str(self.root / 'scripts/zoi.ps1'), 'expand', str(self.root / 'problem.cpp'),
                              '-NoClipboard'], cwd=self.root, stdout=subprocess.PIPE,
                             stderr=subprocess.STDOUT, timeout=30)
        self.assertEqual(run.returncode, 0, run.stdout.decode(errors='replace'))
        self.assertIn('inline int core()', self.read('problem.cpp'))
        self.assertIn('algorithms/new/deep/part.cpp', self.read('zoi/part.h'))

    def test_stub_entry_propagates_duplicate_failure(self):
        self.put('scripts/make_stubs.ps1', (ROOT / 'scripts/make_stubs.ps1').read_text())
        self.put('algorithms/copy.cpp', self.read('algorithms/old/part.cpp'))
        before = self.hashes()
        run = subprocess.run([SHELL, '-NoProfile', '-ExecutionPolicy', 'Bypass', '-File',
                              str(self.root / 'scripts/make_stubs.ps1')], cwd=self.root,
                             stdout=subprocess.PIPE, stderr=subprocess.STDOUT, timeout=30)
        self.assertNotEqual(run.returncode, 0)
        self.assertEqual(before, self.hashes())


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('--shell', default='pwsh')
    parser.add_argument('--compiler', default='g++')
    args = parser.parse_args()
    SHELL, COMPILER = args.shell, args.compiler
    unittest.main(argv=[__file__], verbosity=2)
