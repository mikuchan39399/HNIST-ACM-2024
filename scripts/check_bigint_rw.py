#!/usr/bin/env python3
"""Independent Python-int oracle for bigint and process-exit checks for rw."""
import argparse
import hashlib
import json
import math
import os
from pathlib import Path
import random
import re
import subprocess
import sys
import time
from source_lookup import engine, suite

ROOT = Path(__file__).resolve().parent.parent
SOURCES = [suite(ROOT, name) for name in ('bigint_check.cpp', 'rw_check.cpp')]


def snapshot():
    pending = [ROOT / p for p in SOURCES] + [Path(__file__), ROOT / 'scripts/source_lookup.py']
    files = {}
    while pending:
        path = pending.pop().resolve()
        name = path.relative_to(ROOT).as_posix()
        if name in files:
            continue
        body = path.read_text(encoding="utf-8-sig").replace("\r\n", "\n")
        files[name] = hashlib.sha256(body.encode()).hexdigest()
        if path.suffix in (".cpp", ".h"):
            code = re.sub(r'/\*.*?\*/', '', body, flags=re.S)
            pending += [path.parent / p for p in re.findall(r'^\s*#\s*include\s+"([^"]+)"', code, re.M)]
    return dict(sorted(files.items()))


def cases():
    rng = random.Random(20260908)
    requests, expected = [], []

    def add(op, a, b=None):
        requests.append(f"{op} {a}" + (f" {b}" if b is not None else ""))
        x = int(a)
        y = int(b) if b is not None else 0
        if op in ("str", "i128"): value = x
        elif op == "add": value = x + y
        elif op == "sub": value = x - y
        elif op == "mul": value = x * y
        elif op == "div":
            q = abs(x) // abs(y) * (-1 if (x < 0) != (y < 0) else 1)
            value = f"{q} {x - q * y}"  # C++ truncates toward zero; Python // does not.
        elif op == "mod": value = x % y
        elif op == "gcd": value = math.gcd(x, y)
        elif op == "lcm": value = abs(x // math.gcd(x, y) * y) if x and y else 0
        elif op == "sqrt": value = math.isqrt(x)
        elif op == "pow": value = pow(x, y)
        elif op == "fact": value = math.factorial(x)
        elif op == "cmp": value = (x > y) - (x < y)
        else: raise ValueError(op)
        expected.append(str(value))

    def digits(n):
        return rng.randrange(1, 10) * 10 ** (n - 1) + rng.randrange(10 ** (n - 1))

    for s in ("0", "-0", "+000", "-0001", "+000123", "-" + "0" * 100000): add("str", s)
    for x in (-(1 << 127), -(1 << 127) + 1, (1 << 127) - 1, -(1 << 63), (1 << 64) - 1): add("i128", x)
    lengths = [1, 8, 9, 10, 17, 18, 19, 350, 351, 359, 360, 361, 369, 719, 720, 721, 2000]
    for i in range(600):
        a = digits(rng.choice(lengths)) * rng.choice((-1, 1))
        b = digits(rng.choice(lengths)) * rng.choice((-1, 1))
        for op in ("add", "sub", "mul", "div", "gcd", "lcm", "cmp"): add(op, a, b)
        add("mod", a, rng.choice((1, 2, 998244353, (1 << 63) - 1)))
        if i < 120: add("sqrt", abs(a))
    # Divisor normalization, estimate correction, exact division and tiny remainders.
    base = 10 ** 9
    for n in (2, 3, 4, 39, 40, 41, 80):
        for top in (1, base // 2 - 1, base // 2, base - 1):
            d = top * base ** (n - 1) + rng.randrange(base ** (n - 1))
            for q in (1, base - 1, base, base + 1):
                for r in (0, 1, d - 1):
                    add("div", d * q + r, d)
                    add("div", -(d * q + r), -d)
    for n in (1, 9, 18, 19, 359, 360, 361, 2000, 10000, 50000):
        root = digits(n)
        for delta in (-1, 0, 1): add("sqrt", root * root + delta)
    for n in (10000, 100000):
        a = int("9" * n)
        add("add", a, 1); add("sub", a + 1, 1); add("mul", a, a)
        add("div", a, 999999937)
        add("div", a, int("9" * (n // 2)))
    a, b = digits(20000), digits(731)
    add("mul", a, b); add("div", a, b)
    # Consecutive Fibonacci numbers stress Euclid without constructing the oracle with BigInt.
    a, b = 0, 1
    for _ in range(10000): a, b = b, a + b
    add("gcd", a, b); add("lcm", -a, b)
    for a, b in ((0, 0), (0, 17), (-17, 0), (-2, 10000), (2, 100000), (12345, 123)):
        add("pow", a, b)
    for n in (0, 1, 2, 64, 65, 66, 300, 1000, 10000, 100000): add("fact", n)
    return requests, expected


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--compiler", default="g++")
    parser.add_argument("--sanitize", action="store_true")
    parser.add_argument("--report-dir", default=".zoi-checks/codex-work/bigint-rw-depth")
    args = parser.parse_args()
    if hasattr(sys, "set_int_max_str_digits"): sys.set_int_max_str_digits(0)
    report = (ROOT / args.report_dir).resolve()
    report.mkdir(parents=True, exist_ok=True)
    env = dict(os.environ, TEMP=str(report), TMP=str(report), TMPDIR=str(report))
    flags = ["-std=c++20", "-Wall", "-Wextra", "-Werror", "-UNDEBUG"]
    flags += (["-O1", "-g", "-fsanitize=address,undefined", "-fno-sanitize-recover=all",
               "-fno-omit-frame-pointer", "-D_GLIBCXX_ASSERTIONS"] if args.sanitize else ["-O2"])
    before = snapshot()
    summary = {"platform": sys.platform, "flags": flags, "files": before, "results": [], "status": "FAIL"}

    def run(name, argv, data=None, seconds=240):
        start = time.monotonic()
        result = subprocess.run(argv, input=data, stdout=subprocess.PIPE, stderr=subprocess.PIPE,
                                cwd=report, env=env, timeout=seconds)
        (report / (name + ".stderr.log")).write_bytes(result.stderr)
        summary["results"].append({"name": name, "returncode": result.returncode,
                                   "seconds": round(time.monotonic() - start, 3)})
        if result.returncode:
            raise RuntimeError(f"{name} failed ({result.returncode}); see {report / (name + '.stderr.log')}")
        return result.stdout

    try:
        summary["compiler"] = run("compiler", [args.compiler, "--version"]).decode(errors="replace").splitlines()[0]
        exes = []
        for source in SOURCES:
            exe = report / (Path(source).stem + (".exe" if os.name == "nt" else ""))
            run("compile-" + exe.stem, [args.compiler, *flags, str(ROOT / source), "-o", str(exe)])
            exes.append(str(exe))
        requests, expected = cases()
        output = run("bigint-python", [exes[0], "--oracle"], ("\n".join(requests) + "\n").encode())
        actual = output.decode().splitlines()
        if len(actual) != len(expected): raise AssertionError(f"Output lines {len(actual)} != {len(expected)}")
        for i, (got, want) in enumerate(zip(actual, expected)):
            if got != want:
                (report / "failure.txt").write_text(requests[i] + "\nexpected: " + want + "\nactual: " + got, encoding="utf-8")
                raise AssertionError(f"Python oracle mismatch at case {i}; see failure.txt")
        summary["oracle_cases"] = len(requests)
        summary["max_decimal_digits"] = 100001
        summary["factorial_n"] = 100000
        for name, exe in zip(("bigint-native", "rw-native"), exes):
            result = run(name, [exe])
            (report / (name + ".stdout.log")).write_bytes(result)
            if b"_check passed" not in result: raise AssertionError(name + " missing success marker")
        tail = run("rw-exit-flush", [exes[1], "--exit-flush"])
        if tail != b"E" * (2 * (1 << 22) + 17): raise AssertionError("Destructor flush lost bytes")
        usage_cases = [
            ("bigint", engine(ROOT, 'bigint'), b"-123 97\n",
             "-26\n-220\n-11931\n-1 -26\n71\n11\n1 11931\n6 94\n1 1\n1 1\n0 1\n6\n1\n2432902008176640000\n"),
            ("rw", engine(ROOT, 'rw'), b"3\n1 -2 3\n1.25e3 hello X\n",
             "2\n1 -2 3\n1250.000000\nhello X\nanswer=2\n-170141183460469231731687303715884105728\n340282366920938463463374607431768211455\n"),
        ]
        for name, source, data, want in usage_cases:
            body = (ROOT / source).read_text(encoding="utf-8-sig")
            usage = re.search(r'/\* Usage\s*\n(.*?)\*/', body, re.S)
            if not usage: raise AssertionError(name + " missing runnable Usage")
            cpp = report / ("usage-" + name + ".cpp")
            cpp.write_text(usage[1], encoding="utf-8")
            exe = cpp.with_suffix(".exe" if os.name == "nt" else "")
            run("compile-usage-" + name, [args.compiler, *flags, "-I", str(ROOT / "zoi"), str(cpp), "-o", str(exe)])
            got = run("usage-" + name, [str(exe)], data).decode().splitlines()
            if got != want.splitlines(): raise AssertionError(name + " Usage output mismatch")
            if run("usage-empty-" + name, [str(exe)], b""): raise AssertionError(name + " empty input produced output")
        summary["stable"] = before == snapshot()
        if not summary["stable"]: raise AssertionError("Sources changed during validation")
        summary["status"] = "PASS"
        print(f"PASS: {len(requests)} Python bigint cases; 100000-digit scale; 100000!; rw native + destructor flush; both Usage examples")
    finally:
        (report / "summary.json").write_text(json.dumps(summary, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")


if __name__ == "__main__":
    main()
