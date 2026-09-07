#!/usr/bin/env python3
"""Check local Markdown links and reachability from the user/AI entry points."""
import argparse
from collections import deque
from pathlib import Path
import os
import re
import subprocess
import sys
import tempfile
import unittest
import unicodedata
from urllib.parse import unquote, urlsplit

ROOT = Path(__file__).resolve().parent.parent
ENTRY_POINTS = ('README.md', 'AGENTS.md')


def frozen(path):
    # Historical originals retain links relative to their original locations.
    return (path == 'records/tooling/rule_history.md' or
            (path.startswith(('records/tooling/docs-20260906/', 'records/tooling/docs-20260906-pass2/'))
             and not path.endswith('/README.md')))


def prose(text):
    lines = []
    fence = None
    for line in text.splitlines():
        match = re.match(r'^ {0,3}(`{3,}|~{3,})', line)
        if fence:
            if match and match[1][0] == fence[0] and len(match[1]) >= len(fence):
                fence = None
            continue
        if match:
            fence = match[1]
            continue
        lines.append(line)
    return re.sub(r'<!--.*?-->', '', '\n'.join(lines), flags=re.S)


def links(text):
    text = re.sub(r'(`+).*?\1', '', prose(text))
    # Current repository uses inline links; handle angle paths, escaped chars,
    # balanced parentheses and optional titles without parsing code samples.
    for match in re.finditer(r'\]\(\s*', text):
        start = match.end()
        if start < len(text) and text[start] == '<':
            end = text.find('>', start + 1)
            if end >= 0:
                yield text[start + 1:end]
            continue
        end, depth = start, 0
        while end < len(text):
            char = text[end]
            if char == '\\':
                end += 2
                continue
            if char == '(':
                depth += 1
            elif char == ')':
                if not depth:
                    break
                depth -= 1
            elif char.isspace() and not depth:
                break
            end += 1
        if end > start:
            yield re.sub(r'\\([() ])', r'\1', text[start:end])
    for match in re.finditer(r'^ {0,3}\[[^\]\n]+\]:\s*(?:<([^>]+)>|(\S+))', text, re.M):
        yield match[1] or match[2]


def anchors(text):
    result, counts = set(), {}
    for line in prose(text).splitlines():
        match = re.match(r'^ {0,3}#{1,6}\s+(.+?)(?:\s+#+)?$', line)
        if not match:
            continue
        title = re.sub(r'\[([^\]]+)\]\([^)]*\)', r'\1', match[1])
        title = re.sub(r'<[^>]+>', '', title).replace('`', '').lower()
        slug = ''.join(c for c in title if c in '_- ' or unicodedata.category(c)[0] in 'LN')
        slug = slug.replace(' ', '-')
        number = counts.get(slug, 0)
        counts[slug] = number + 1
        result.add(slug + ('-' + str(number) if number else ''))
    result.update(re.findall(r'<a\s+(?:id|name)=["\']([^"\']+)', text))
    return result


def audit(root, names):
    names = set(names)
    bodies = {name: (root / name).read_text(encoding='utf-8-sig') for name in names}
    graph = {name: set() for name in names}
    errors = []
    for name, body in bodies.items():
        if frozen(name):
            continue
        for link in links(body):
            parsed = urlsplit(link)
            if parsed.scheme or parsed.netloc:
                continue
            path = unquote(parsed.path)
            target = ((root / path.lstrip('/')) if path.startswith('/') else (root / name).parent / path) if path else root / name
            spelled = Path(os.path.normpath(target))
            target = target.resolve()
            try:
                target.relative_to(root.resolve())
            except ValueError:
                errors.append(f'{name}: link leaves repository: {link}')
                continue
            if not target.exists():
                errors.append(f'{name}: missing target: {link}')
                continue
            if target.is_dir():
                if not (target / 'README.md').is_file():
                    continue
                target = target / 'README.md'
                spelled = spelled / 'README.md'
            relative = spelled.relative_to(root).as_posix()
            if relative.lower().endswith('.md') and relative not in names:
                errors.append(f'{name}: Markdown target is untracked, ignored or has wrong case: {link}')
            if relative in names:
                graph[name].add(relative)
                fragment = unquote(parsed.fragment)
                if fragment and not frozen(relative) and fragment not in anchors(bodies[relative]):
                    errors.append(f'{name}: missing anchor: {link}')
    reachability = {}
    for entry in ENTRY_POINTS:
        if entry not in names:
            errors.append('missing entry point: ' + entry)
        seen, queue = set(), deque([entry])
        while queue:
            current = queue.popleft()
            if current in seen:
                continue
            seen.add(current)
            queue.extend(graph.get(current, ()))
        unreachable = sorted(names - seen)
        reachability[entry] = len(names) - len(unreachable)
        errors.extend(f'{entry}: unreachable document: {name}' for name in unreachable)
    return errors, reachability


def self_test():
    class NavigationTests(unittest.TestCase):
        def setUp(self):
            work = ROOT / '.zoi-checks/codex-work'
            work.mkdir(parents=True, exist_ok=True)
            self.temp = tempfile.TemporaryDirectory(prefix='docs-nav-', dir=work)
            self.addCleanup(self.temp.cleanup)
            self.root = Path(self.temp.name)
            self.names = set()
            self.put('README.md', '[AI](AGENTS.md)\n[Guide](guide.md#中文标题)')
            self.put('AGENTS.md', '[User](README.md)')
            self.put('guide.md', '# 中文标题\n[home](README.md)')

        def put(self, name, body):
            target = self.root / name
            target.parent.mkdir(parents=True, exist_ok=True)
            target.write_text(body, encoding='utf-8')
            self.names.add(name)

        def errors(self):
            return audit(self.root, self.names)[0]

        def test_both_roots_and_read_only(self):
            before = {p: (self.root / p).read_bytes() for p in self.names}
            self.assertEqual(self.errors(), [])
            self.assertEqual(before, {p: (self.root / p).read_bytes() for p in self.names})

        def test_missing_target_and_anchor(self):
            self.put('guide.md', '# 中文标题\n[x](lost.md)\n[x](README.md#lost)')
            errors = self.errors()
            self.assertTrue(any('missing target' in e for e in errors))
            self.assertTrue(any('missing anchor' in e for e in errors))

        def test_orphan_cycle_and_one_way_entry(self):
            self.put('a.md', '[b](b.md)')
            self.put('b.md', '[a](a.md)')
            self.put('AGENTS.md', '# Isolated')
            errors = self.errors()
            self.assertTrue(any('README.md: unreachable document: a.md' in e for e in errors))
            self.assertTrue(any('AGENTS.md: unreachable document: README.md' in e for e in errors))

        def test_code_samples_and_external_urls(self):
            self.put('guide.md', '# 中文标题\n```md\n[x](no.md)\n```\n`[x](no.md)`\n<!-- [x](no.md) -->\n[x](https://example.com/no.md)')
            self.assertEqual(self.errors(), [])

        def test_spaces_parentheses_reference_and_directory(self):
            self.put('guide.md', '# 中文标题\n[x](<folder/file (1).md>)\n[x](folder/file(2).md "title")\n[x](folder)\n[x](data)\n[ref]: <folder/file (1).md>')
            (self.root / 'data').mkdir()
            self.put('folder/file (1).md', '# One')
            self.put('folder/file(2).md', '# Two')
            self.put('folder/README.md', '# Folder')
            self.assertEqual(self.errors(), [])

        def test_duplicate_heading_and_percent_fragment(self):
            self.put('guide.md', '# 中文标题\n# Repeat\n# Repeat\n[x](#repeat-1)\n[x](#%E4%B8%AD%E6%96%87%E6%A0%87%E9%A2%98)')
            self.assertEqual(self.errors(), [])

        def test_frozen_original_still_needs_entry(self):
            name = 'records/tooling/docs-20260906/rule.md'
            self.put(name, '[old location](lost.md)')
            self.assertTrue(any('unreachable document: ' + name in e for e in self.errors()))
            self.put('guide.md', '# 中文标题\n[Archive](' + name + ')')
            self.assertEqual(self.errors(), [])

        def test_case_missing_root_and_path_escape(self):
            self.put('guide.md', '# 中文标题\n[x](../outside.md)\n[x](readme.md)')
            errors = self.errors()
            self.assertTrue(any('leaves repository' in e for e in errors))
            self.assertTrue(any('readme.md' in e for e in errors))
            self.names.remove('AGENTS.md')
            self.assertTrue(any('missing entry point' in e for e in self.errors()))

    result = unittest.TextTestRunner(verbosity=2).run(unittest.defaultTestLoader.loadTestsFromTestCase(NavigationTests))
    return int(not result.wasSuccessful())


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--self-test', action='store_true')
    args = parser.parse_args()
    if args.self_test:
        return self_test()
    result = subprocess.run(['git', 'ls-files', '--cached', '--others', '--exclude-standard', '-z'], cwd=ROOT, check=True, capture_output=True)
    names = [name for name in result.stdout.decode('utf-8').split('\0') if name.lower().endswith('.md') and (ROOT / name).is_file()]
    errors, reachability = audit(ROOT, names)
    for error in errors:
        print('[FAIL] ' + error)
    if not errors:
        print(f'[PASS] {len(set(names))} Markdown documents; user={reachability["README.md"]}, AI={reachability["AGENTS.md"]}; local links and anchors valid')
    return int(bool(errors))


if __name__ == '__main__':
    sys.exit(main())
