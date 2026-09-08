"""Resolve logical stress-test roles without encoding algorithm directories."""
from pathlib import Path


def suite(root, filename):
    if Path(filename).name != filename or not filename.endswith('_check.cpp'):
        raise ValueError('Expected a suite file name, not a directory path')
    return unique(root, (p for p in (root / 'algorithms').rglob(filename) if p.is_file()), filename)


def engine(root, identity):
    matches = []
    for path in (root / 'algorithms').rglob('*.cpp'):
        with path.open(encoding='utf-8-sig') as stream:
            if stream.readline().rstrip('\r\n') == '// zoi: ' + identity:
                matches.append(path)
    return unique(root, matches, identity)


def unique(root, paths, label):
    paths = list(paths)
    if len(paths) != 1:
        raise ValueError(f'Missing or ambiguous source identity {label}: {paths}')
    return paths[0].resolve().relative_to(root.resolve()).as_posix()
