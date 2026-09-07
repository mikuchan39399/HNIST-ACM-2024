#!/usr/bin/env python3
"""Independent PDF text checks after the real Typst build (requires pypdf)."""
import argparse
from pathlib import Path
import re
from pypdf import PdfReader


def compact(text):
    return re.sub(r'\s|\u200b|\u00ad', '', text)


def plain(text):
    text = re.sub(r'\[([^\]]+)\]\((?:<[^>]+>|[^()]*(?:\([^()]*\)[^()]*)*)\)', r'\1', text)
    # Math glyph order differs from TeX source; headings/list checks use prose.
    text = re.sub(r'\$[^$]+\$', '', text)
    return text.replace('**', '').replace('`', '').strip()


def check(pdf, root):
    reader = PdfReader(pdf)
    pages = [p.extract_text() or '' for p in reader.pages]
    whole = compact('\n'.join(pages))
    miku = [i+1 for i, text in enumerate(pages) if 'MIKU' in text]
    expected = [39] if len(pages) >= 39 else []
    assert miku == expected, f'MIKU leaked or missing: {miku}, expected {expected}'
    assert sum(t.count('MIKU') for t in pages) == len(expected), 'MIKU repeated on page'
    typ = pdf.with_suffix('.typ').read_text(encoding='utf-8-sig')
    # PDF bookmarks are the same outlined headings as the printed contents.
    # Match every title and level in order, including nested and sibling sources.
    headings = re.findall(r'^#heading\(level: (\d+), "((?:[^"\\]|\\.)*)"\) <([^>]+)>', typ, re.M)
    import json
    expected_outline = [(int(depth), json.loads('"'+title+'"'), label) for depth, title, label in headings]
    actual_outline = []
    def collect_outline(items, depth=1):
        for item in items:
            if isinstance(item, list):
                collect_outline(item, depth + 1)
            else:
                actual_outline.append((depth, item.title, reader.get_destination_page_number(item)))
    collect_outline(reader.outline)
    assert [(d, t) for d, t, _ in actual_outline] == [(d, t) for d, t, _ in expected_outline], 'PDF contents omit, flatten or reorder a directory/source'
    entry_pages = [actual_outline[i][2] for i, (_, _, label) in enumerate(expected_outline) if label.startswith('e-')]
    assert entry_pages == sorted(set(entry_pages)), 'Source entries share their opening page'
    # Bookmarks alone are insufficient: each heading must also be printed in TOC.
    body_start = min(p for _, _, p in actual_outline)
    contents_text = compact('\n'.join(pages[1:body_start]))
    for _, title, _ in expected_outline:
        assert compact(title) in contents_text, f'Title missing from printed contents: {title}'
    for i, (_, title, label) in enumerate(expected_outline):
        if not label.startswith('e-'):
            continue
        page = reader.pages[actual_outline[i][2]]
        header = []
        def visit_header(text, cm, tm, font, size):
            if size == 6 and cm[5] > float(page.mediabox.height) - 30:
                header.append(text)
        page.extract_text(visitor_text=visit_header)
        assert compact(title) in compact(''.join(header)), f'Opening page header hides source title: {title}'
    print(f'[PASS] PDF contents: {len(actual_outline)} directory/source titles; {len(entry_pages)} independent entry starts')
    manuals = re.findall(r'^// manual: (.+)$', typ, re.M)
    directories = re.findall(r'^// directory: (.+)$', typ, re.M)
    assert len(directories) == len(set(directories)), 'Repeated algorithm directory'
    if '// directory-scope: all' in typ:
        expected_dirs = {p.relative_to(root).as_posix() for p in (root / 'algorithms').rglob('*')
                         if p.is_dir() and '对拍' not in p.parts
                         and not any(part.startswith('.') for part in p.relative_to(root).parts)}
        assert set(directories) == expected_dirs, f'Directory discovery mismatch: {set(directories) ^ expected_dirs}'
    for directory in directories:
        assert compact(Path(directory).name) in whole, f'Directory title missing from PDF: {directory}'
    if any(Path(d).name == 'DEPTHLEVEL7' for d in directories):
        # Read painted text sizes, not the generator's style table. The old
        # level-2-only rule made deeper contents entries larger than parents.
        samples = {depth: [] for depth in range(1, 8)}
        for page_index, page in enumerate(reader.pages):
            def visit(text, cm, tm, font, size):
                match = re.fullmatch(r'DEPTHLEVEL([1-7])', text.strip())
                if match and size > 6:  # Page headers are 6 pt, not chapter titles.
                    samples[int(match[1])].append((page_index, size, cm[4], cm[5]))
            page.extract_text(visitor_text=visit)
        assert all(len(v) == 2 for v in samples.values()), 'Deep chapter missing from contents or body'
        for position in (0, 1):
            sizes = [samples[depth][position][1] for depth in range(1, 8)]
            assert all(a >= b for a, b in zip(sizes, sizes[1:])), f'Child title outgrows parent: {sizes}'
            assert sizes[0] > sizes[1] > sizes[2], f'Primary hierarchy flattened: {sizes}'
        for depth in range(1, 7):
            parent, child = samples[depth][0], samples[depth + 1][0]
            if parent[0] == child[0]:
                assert child[2] > parent[2], 'Contents child lost its indentation'
                assert abs(child[3] - parent[3]) >= min(parent[1], child[1]), 'Contents rows overlap'
        print('[PASS] PDF hierarchy: seven levels present; contents and body sizes descend')
    equations = typ.count('#metadata("booklet-math")')
    if equations:
        fonts = set()
        for page in reader.pages:
            resources = page.get('/Resources')
            font_map = resources.get_object().get('/Font') if resources else None
            if font_map:
                fonts.update(str(font.get_object().get('/BaseFont', ''))
                             for font in font_map.get_object().values())
        assert any('NewCMMath' in font or 'NewComputerModernMath' in font for font in fonts), 'Math font missing from PDF'
    assert len(manuals) == len(set(manuals)), 'Repeated manual'
    checks = 0
    for relative in manuals:
        text = (root / relative).read_text(encoding='utf-8-sig')
        fenced = False
        for line in text.splitlines():
            if re.match(r'^\s*(`{3,}|~{3,})', line):
                fenced = not fenced
                continue
            if fenced:
                continue
            # Headings and the starts of numbered items must really be visible.
            # This catches disappearing content even if the .typ still has it.
            m = re.match(r'^\s*(?:#{1,6}\s+|\d+[.)]\s+)(.+)', line)
            if not m:
                continue
            needle = compact(plain(m[1]))[:28]
            assert needle in whole, f'Missing printed README text: {relative}: {needle}'
            checks += 1
    if 'GROWTH_1200' in typ:
        # Keep separators: removing whitespace glues a page footer digit to the
        # last marker on that page and creates a false missing-line report.
        growth = [int(n) for n in re.findall(r'GROWTH_\s*(\d+)', '\n'.join(pages))]
        assert growth == list(range(1, 1201)), 'Growth lines missing, duplicated or out of order'
        for marker in ('SOURCE_B', 'SHARED_MANUAL', 'NUMBERED_ONE_KEEP', 'NUMBERED_TWO_KEEP',
                       'BULLET_KEEP', 'TABLE_KEEP', 'SECOND_ROW_KEEP', 'FENCED_KEEP', 'LAST_MANUAL_KEEP',
                       'MATH_INLINE_KEEP', 'MATH_DISPLAY_KEEP', 'MATH_TABLE_KEEP'):
            assert marker in whole, f'Fixture content absent: {marker}'
        assert 'MUST_NOT_PRINT' not in whole, 'Roadmap entered PDF'
        assert whole.index('SOURCE_B') < whole.index('SHARED_MANUAL') < whole.index('LAST_MANUAL_KEEP'), 'Detached manual'
    print(f'[PASS] PDF: {len(pages)} pages, {len(directories)} directories, {len(manuals)} manuals, {equations} math equations, {checks} headings/list checks, MIKU={miku}')


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('pdf', type=Path)
    parser.add_argument('--root', type=Path, default=Path(__file__).resolve().parent.parent)
    args = parser.parse_args()
    check(args.pdf.resolve(), args.root.resolve())
