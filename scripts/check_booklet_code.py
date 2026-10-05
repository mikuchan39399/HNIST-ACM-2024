"""Compile the actual C++20 raw blocks sent to Typst, without source fallbacks."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import re
import shutil
import subprocess
import tempfile


def require(ok, message):
    if not ok:
        raise RuntimeError(message)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("directory", type=Path)
    parser.add_argument("--compiler", default="g++")
    args = parser.parse_args()
    root = Path(__file__).resolve().parent.parent
    index = json.loads((args.directory / "chapters.json").read_text(encoding="utf-8-sig"))
    require(index.get("cppStandard") == "c++20" and index.get("profile") == "contest", "Missing contest profile identity")
    entries = {}
    pattern = re.compile(
        r'^// entry: ([^\n]+)\n#entrymeta\[([^\n]+)\]\n'
        r'#raw\(("(?:\\.|[^"\\])*"), block: true, lang: "cpp"\)', re.M
    )
    for chapter in index["chapters"]:
        typ = (args.directory / chapter["source"]).read_text(encoding="utf-8")
        audit = (args.directory / chapter["code"]).read_text(encoding="utf-8")
        audit_pattern = re.compile(r'^<!-- entry: ([^\n]+) -->\n(.*?)^(`{3,})cpp\n(.*?)\n\3$', re.M | re.S)
        audit_entries = {}
        for key, meta, _, code in audit_pattern.findall(audit):
            require(key not in audit_entries, f"Duplicate audit source: {key}")
            fingerprint = hashlib.sha256(code.encode("utf-8")).hexdigest()
            require(f"SHA256: {fingerprint}; lines: {len(code.split(chr(10)))}" in meta,
                    f"Readable code fingerprint/line count differs: {key}")
            audit_entries[key] = code
        chapter_keys = set()
        for relative, meta, literal in pattern.findall(typ):
            code = json.loads(literal)
            require(audit_entries.get(relative) == code, f"Readable code differs from print input: {relative}")
            require("ZOI_BOOKLET" not in code, f"Print profile directive leaked: {relative}")
            chapter_keys.add(relative)
            fingerprint = hashlib.sha256(code.encode("utf-8")).hexdigest()[:8]
            require(meta.endswith(fingerprint), f"Printed fingerprint mismatch: {relative}")
            require(f"{len(code.split(chr(10)))} 行" in meta, f"Printed line count mismatch: {relative}")
            require(relative not in entries, f"Duplicate printed source: {relative}")
            entries[relative] = code
        require(chapter_keys == audit_entries.keys(), f"Audit/Typst entry inventory differs: {chapter['chapter']}")
    require(entries, "No printable C++ blocks found")
    # Extend this list for further compatibility work and printed template regressions.
    kmp_key = "algorithms/字符串/KMP/kmp.cpp"
    utils_key = "algorithms/杂项/utils/utils.cpp"
    manacher_key = "algorithms/字符串/Manacher/manacher.cpp"
    z_key = "algorithms/字符串/Z函数/z_function.cpp"
    require(all(key in entries for key in (kmp_key, utils_key, manacher_key, z_key)), "Missing printed KMP/Manacher/Z/utils")
    kmp, utils = entries[kmp_key], entries[utils_key]
    require(kmp.count("struct KMPSeq") == 1, "KMP printed multiple engines")
    for signature in ("void build(auto s)", "int find_first(auto s)", "VI find_all(auto s)"):
        require(kmp.count(signature) == 1, f"KMP printed entry missing or duplicated: {signature}")
    for key in (kmp_key, manacher_key, z_key):
        code = entries[key]
        for old in ("build_raw(", "build_seq(", "extend_seq(", "first_raw(", "all_raw(", "class A>", "span<U, N>"):
            require(old not in code, f"Electronic adapter leaked into print: {key}: {old}")
    require("__cplusplus" not in kmp + utils + entries[manacher_key] + entries[z_key], "Version branch leaked into printed code/Usage")
    require(entries[z_key].count("void build(auto s)") == 1 and
            entries[z_key].count("static VI extend(auto text, auto pattern)") == 1,
            "Z/E printed core missing/duplicated")
    require("#ifdef LOCAL" in utils, "LOCAL guard lost")
    parent = root / ".zoi-checks" / "codex-work"
    parent.mkdir(parents=True, exist_ok=True)
    work = Path(tempfile.mkdtemp(prefix="booklet-code-", dir=parent))
    env = dict(os.environ, TEMP=str(work), TMP=str(work), TMPDIR=str(work))

    def run(source, name, flags=(), extra=()):
        src = work / (name + ".cpp")
        src.write_text(source, encoding="utf-8")
        exe = work / (name + (".exe" if os.name == "nt" else ""))
        command = [args.compiler, "-std=c++20", "-pedantic-errors", "-Wall", "-Wextra",
                   "-Werror", "-UNDEBUG", "-O2", "-I", str(work), *flags, str(src), *extra, "-o", str(exe)]
        subprocess.run(command, cwd=work, env=env, check=True, timeout=120)
        return subprocess.run([str(exe)], cwd=work, env=env, check=True, timeout=120,
                              capture_output=True, text=True, encoding="utf-8").stdout

    try:
        shutil.copyfile(root / "scripts/booklet_test_adapter.h", work / "booklet_test_adapter.h")
        # No -I zoi: every included algorithm below is the exact printed block.
        (work / "kmp.h").write_text(kmp, encoding="utf-8")
        (work / "utils.h").write_text(utils, encoding="utf-8")
        suite = (root / "algorithms/字符串/KMP/对拍/kmp_check.cpp").read_text(encoding="utf-8-sig")
        suite = suite.replace('#include "../kmp.cpp"', '#include "booklet_test_adapter.h"')
        require('../kmp.cpp' not in suite, "KMP regression still imports the original engine")
        result = run(suite, "kmp", flags=("-DBOOKLET_CHECK_KMP",))
        for marker in ("kmp: PASS", "sequence kmp: PASS", "span kmp: PASS"):
            require(marker in result, f"Missing regression result: {marker}")
        print(result.strip())
        usage = re.search(r'/\*\s*Usage\b(.*?)\*/', kmp, re.S)
        require(usage is not None, "Printed KMP Usage missing")
        result = run(usage[1], "kmp-usage")
        require(result.split() == '1 0 0 1 1 3 5 1 3'.split(), "Printed Usage result differs")
        manacher = entries[manacher_key]
        (work / "manacher.h").write_text(manacher, encoding="utf-8")
        suite = (root / "algorithms/字符串/Manacher/对拍/manacher_check.cpp").read_text(encoding="utf-8-sig")
        suite = suite.replace('#include "../manacher.cpp"', '#include "booklet_test_adapter.h"')
        require('../manacher.cpp' not in suite, "Manacher regression still imports the original engine")
        result = run(suite, "manacher", flags=("-DBOOKLET_CHECK_MANACHER",))
        for marker in ("manacher: PASS", "sequence manacher: PASS", "span manacher: PASS"):
            require(marker in result, f"Missing printed Manacher regression result: {marker}")
        print(result.strip())
        usage = re.search(r'/\*\s*Usage\b(.*?)\*/', manacher, re.S)
        require(usage is not None, "Printed Manacher Usage missing")
        result = run(usage[1], "manacher-usage")
        require(result.split() == '1 7 1 12 3'.split(), "Printed Manacher Usage result differs")
        z_code = entries[z_key]
        (work / "zFunction.h").write_text(z_code, encoding="utf-8")
        suite = (root / "algorithms/字符串/Z函数/对拍/z_function_check.cpp").read_text(encoding="utf-8-sig")
        suite = suite.replace('#include "../z_function.cpp"', '#include "booklet_test_adapter.h"')
        require('../z_function.cpp' not in suite, "Z regression still imports the original engine")
        result = run(suite, "z-function", flags=("-DBOOKLET_CHECK_Z",))
        for marker in ("z_function: PASS", "span z: PASS", "extend: PASS"):
            require(marker in result, f"Missing printed Z regression result: {marker}")
        print(result.strip())
        usage = re.search(r'/\*\s*Usage\b(.*?)\*/', z_code, re.S)
        require(usage is not None, "Printed Z Usage missing")
        result = run(usage[1], "z-usage")
        require(result.split() == '7 0 1 0 3 0 1 2 3 1 0 2 1 0 3'.split(), "Printed Z Usage result differs")
        run(r'''
#include "kmp.h"
#include "manacher.h"
#include "zFunction.h"
int main() {
    KMP k; Manacher m; ZFunction z;
    assert(k.m == 0 && k.p.size() == 1 && k.pi == VI({0}));
    assert(m.n == 0 && m.p == VI({0, 0}) && m.longest() == PII(0, 0) && m.count() == 0);
    assert(z.n == 0 && z.z == VI({0}));
    int a[] = {7, 2, 7};
    assert(k.find_first(span<const char>()) == 1);
    KMPSeq<VI> seq;
    assert(seq.find_all(span(a)) == VI({1, 2, 3, 4}));
    m.build(span(a)); z.build(span(a));
    assert(m.longest() == PII(1, 3) && z.z == VI({0, 3, 0, 1}));
}
''', "printed-direct-defaults")
        other = work / "utils-other.cpp"
        other.write_text('#include "utils.h"\nint other() { return dx4[0]; }\n', encoding="utf-8")
        sample = r'''
#include "utils.h"
#include "utils.h"
int other();
int main() {
    int hi = 3, lo = 3;
    assert(cmax(hi, 7) && hi == 7 && !cmax(hi, 7));
    assert(cmin(lo, 1) && lo == 1 && !cmin(lo, 1));
    for (int n : {200000, 0, 1, 257, 200000}) {
        VI a(n + 20, 7); VLL b(n + 1, 9); vector<bool> bits(n + 11, true);
        z_fill_n(n, 0, a, b, bits);
        for (size_t i = 0; i < a.size(); ++i) assert(a[i] == (i < (size_t)n + 10 ? 0 : 7));
        for (LL x : b) assert(x == 0);
        for (size_t i = 0; i < bits.size(); ++i) assert(bits[i] == (i >= (size_t)n + 10));
    }
    dx4[0] = 3; assert(other() == 3);
    static_assert(integral<LL>); assert(popcount(7u) == 3);
    string_view word = "abc"; assert(word.size() == 3);
    VI v = {1, 2}; span<int> view(v); assert(view[1] == 2);
#ifdef LOCAL
    debug(hi); debug_array(v, 1);
#endif
}
'''
        run(sample, "utils-release", extra=(str(other),))
        run(sample, "utils-local", flags=("-DLOCAL",), extra=(str(other),))
        print(f"PASS: {len(entries)} printed fingerprints; C++20 printed KMP/Manacher/Z full regression/Usage and utils LOCAL/multi-TU")
    except Exception:
        print(f"Failure fixture retained: {work}")
        raise
    else:
        require(work.resolve().parent == parent.resolve(), "Unsafe fixture cleanup")
        shutil.rmtree(work)


if __name__ == "__main__":
    main()
