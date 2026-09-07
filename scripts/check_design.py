#!/usr/bin/env python3
"""Check that design tables cover progress entries and catalog identities."""
from collections import Counter
from pathlib import Path
import re
from urllib.parse import unquote

ROOT = Path(__file__).resolve().parent.parent
KINDS = ('固定引擎', '启发式骨架', '固定内核加题目骨架')


def rows(path):
    for line in path.read_text(encoding='utf-8-sig').splitlines():
        if not line.startswith('| '):
            continue
        cells = [c.strip() for c in line.strip('|').split('|')]
        if len(cells) in (3, 4) and cells[0] not in ('条目', '知识点或实现（依据）') and not re.fullmatch(r':?-+:?', cells[0]):
            yield cells


def label(item):
    return re.sub(r'^\[([^\]]+)\].*', r'\1', item)


def check():
    counts = Counter()
    classified_paths = set()
    summary = (ROOT / 'docs/features/template-design.md').read_text(encoding='utf-8-sig')
    for progress in sorted((ROOT / 'docs/progress').glob('*.md')):
        if progress.stem == 'README':
            continue
        design = ROOT / 'docs/features/design' / progress.name
        expected = Counter(label(r[0]) for r in rows(progress))
        actual_rows = list(rows(design))
        actual = Counter(label(r[0]) for r in actual_rows)
        assert actual == expected, f'{progress.stem}: design/progress mismatch: {actual-expected}, {expected-actual}'
        assert all(n == 1 for n in actual.values()), f'{progress.stem}: duplicate identity'
        local = Counter(r[2] for r in actual_rows)
        assert set(local) <= set(KINDS), f'{progress.stem}: unknown design type'
        counts.update(local)
        totals = [local[k] for k in KINDS] + [len(actual_rows)]
        expected_summary = '| [' + progress.stem + '](design/' + progress.name + ') | ' + ' | '.join(map(str,totals)) + ' |'
        assert expected_summary in summary, f'{progress.stem}: summary counts stale'
        for item, *_ in actual_rows:
            m = re.search(r'\]\(<([^>]+)>\)', item)
            if m and '#' not in m[1]:
                classified_paths.add((design.parent / unquote(m[1])).resolve())
    catalog = []
    for line in (ROOT / 'zoi/_catalog.txt').read_text(encoding='utf-8-sig').splitlines():
        if not line.strip() or line.startswith(('#', '!')):
            continue
        catalog.append((ROOT / line.split('\t')[1]).resolve())
    assert set(catalog) <= classified_paths, 'Catalog entries missing from design tables'
    for p in (ROOT / 'algorithms').rglob('README.md'):
        if p.parent == ROOT / 'algorithms':
            continue
        assert any(p.parent.glob('*.cpp')) or any(p.parent.glob('*.txt')), f'Roadmap/empty README returned to source tree: {p}'
        assert not p.read_text(encoding='utf-8-sig').startswith('# 骨架空目录'), f'Placeholder manual: {p}'
    print(f'[PASS] design: {sum(counts.values())} entries, {len(catalog)} catalog paths; {dict(counts)}; source README roles valid')


if __name__ == '__main__':
    check()
