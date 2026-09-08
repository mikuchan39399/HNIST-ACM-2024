#!/usr/bin/env python3
"""Linux CI entry for graph stress profiles and bounded LCA stack probes."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import re
import signal
import subprocess
import sys
import tempfile
import time
import unittest
from unittest import mock
from source_lookup import suite as find_suite

ROOT = Path(__file__).resolve().parent.parent
SOURCE = find_suite(ROOT, 'lca_vt_stress_check.cpp')


def classify(code, output, timed_out=False, probe=False, success_marker="lca_vt_stress_check passed"):
    if timed_out:
        return "TIMEOUT"
    marker = "probe passed" if probe else success_marker
    if code == 0 and marker in output:
        return "PASS"
    # Only an explicit ASan stack-overflow diagnosis may be a known boundary.
    if (probe and code == 1 and "ERROR: AddressSanitizer: stack-overflow" in output
            and "runtime error:" not in output):
        return "STACK_LIMIT"
    return "FAIL"


def snapshot(source=SOURCE):
    pending = [ROOT / source, Path(__file__), ROOT / 'scripts/source_lookup.py', ROOT / ".github/workflows/ci.yml"]
    files = {}
    while pending:
        path = pending.pop().resolve()
        name = path.relative_to(ROOT).as_posix()
        if name in files:
            continue
        body = path.read_text(encoding="utf-8").replace("\r\n", "\n")
        files[name] = hashlib.sha256(body.encode("utf-8")).hexdigest()
        if path.suffix in (".cpp", ".h", ".hpp"):
            # Preserve offsets/newlines and quoted operands, but ignore Usage,
            # commented directives and raw-string examples. Conditional branches
            # both remain dependencies; missing real includes must still fail.
            pattern = r'''R"(?P<delimiter>[^ ()\\\t\r\n]{0,16})\(.*?\)(?P=delimiter)"|"(?:\\.|[^"\\\r\n])*"|'(?:\\.|[^'\\\r\n])*'|/\*.*?\*/|//[^\r\n]*'''
            code = re.sub(pattern, lambda m: re.sub(r'[^\r\n]', ' ', m[0])
                          if m[0].startswith(('/', 'R"')) else m[0], body, flags=re.S)
            for inc in re.findall(r'^\s*#\s*include\s+"([^"]+)"', code, re.M):
                pending.append(path.parent / inc)
    return dict(sorted(files.items()))


def run_case(name, argv, report, seconds, stack_kib=None, probe=False, compile_only=False,
             success_marker="lca_vt_stress_check passed"):
    def limits():
        import resource
        resource.setrlimit(resource.RLIMIT_CORE, (0, 0))
        if stack_kib is not None:
            resource.setrlimit(resource.RLIMIT_STACK, (stack_kib * 1024, stack_kib * 1024))

    env = os.environ.copy()
    # Compiler scratch files belong to this repository report, including in WSL.
    for key in ("TEMP", "TMP", "TMPDIR"):
        env[key] = str(report)
    env["ASAN_OPTIONS"] = "detect_leaks=1:halt_on_error=1"
    env["UBSAN_OPTIONS"] = "halt_on_error=1:print_stacktrace=1"
    start = time.monotonic()
    timed_out = False
    with (report / (name + ".stdout.log")).open("w") as out, (report / (name + ".stderr.log")).open("w") as err:
        with subprocess.Popen(argv, cwd=ROOT, env=env, stdout=out, stderr=err,
                              start_new_session=True, preexec_fn=limits) as process:
            try:
                code = process.wait(timeout=seconds)
            except subprocess.TimeoutExpired:
                os.killpg(process.pid, signal.SIGKILL)
                process.wait()
                timed_out, code = True, None
    output = (report / (name + ".stdout.log")).read_text(errors="replace")
    output += (report / (name + ".stderr.log")).read_text(errors="replace")
    status = ("TIMEOUT" if timed_out else "PASS" if code == 0 else "FAIL") if compile_only else classify(code, output, timed_out, probe, success_marker)
    result = dict(name=name, status=status, exit_code=code, seconds=round(time.monotonic() - start, 3),
                  timeout_seconds=seconds, stack_kib=stack_kib, argv=argv)
    print("{}: {} (exit={}, stack_kib={})".format(name, status, code, stack_kib), flush=True)
    return result


class FailureTests(unittest.TestCase):
    def test_snapshot_real_dependencies_and_examples(self):
        with tempfile.TemporaryDirectory(dir=ROOT / ".zoi-checks") as tmp:
            folder = Path(tmp)
            source, dependency = folder / "source.cpp", folder / "real.hpp"
            dependency.write_text("// real dependency\n", encoding="utf-8")
            source.write_text('#include "real.hpp"\n/* Usage\n#include "missing-usage.h"\n*/\n'
                              '// #include "missing-comment.h"\n'
                              'const char* sample = R"demo(\n#include "missing-raw.h"\n)demo";\n', encoding="utf-8")
            relative = source.relative_to(ROOT).as_posix()
            before = snapshot(relative)
            self.assertIn(dependency.relative_to(ROOT).as_posix(), before)
            self.assertFalse(any("missing-" in name for name in before))
            dependency.write_text("// changed real dependency\n", encoding="utf-8")
            self.assertNotEqual(before, snapshot(relative))
            source.write_text('#include "missing-real.hpp"\n', encoding="utf-8")
            with self.assertRaises(FileNotFoundError):
                snapshot(relative)

    def test_both_profile_entry_points_reach_compilation(self):
        # Execute real main()/source discovery/snapshot/report writing; replace
        # only compiler execution. A failed compile must propagate to the gate.
        for profile, filename in (("lca-vt", "lca_vt_stress_check.cpp"),
                                  ("completed-graph", "completed_graph_stress_check.cpp")):
            with self.subTest(profile=profile), tempfile.TemporaryDirectory(dir=ROOT / ".zoi-checks") as tmp:
                argv = [__file__, "--profile", profile, "--report-dir", tmp]
                with mock.patch.object(sys, "argv", argv), \
                     mock.patch.object(subprocess, "check_output", return_value="fixture compiler\n"), \
                     mock.patch(__name__ + ".run_case", return_value={"status": "FAIL"}) as runner:
                    self.assertEqual(main(), 1)
                self.assertEqual(runner.call_count, 2)
                for call in runner.call_args_list:
                    self.assertIn(find_suite(ROOT, filename), call[0][1])
                result = json.loads((Path(tmp) / "summary.json").read_text(encoding="utf-8"))
                self.assertEqual(result["profile"], profile)
                self.assertTrue(result["stable"])
                self.assertTrue(result["failed"])

    def test_classification(self):
        self.assertEqual(classify(0, "lca_vt_stress_check passed"), "PASS")
        self.assertEqual(classify(0, ""), "FAIL")
        self.assertEqual(classify(1, "assertion failed"), "FAIL")
        self.assertEqual(classify(None, "", timed_out=True), "TIMEOUT")
        self.assertEqual(classify(0, "probe passed", probe=True), "PASS")
        self.assertEqual(classify(1, "ERROR: AddressSanitizer: stack-overflow", probe=True), "STACK_LIMIT")
        self.assertEqual(classify(1, "ERROR: AddressSanitizer: stack-overflow"), "FAIL")
        self.assertEqual(classify(-11, "AddressSanitizer:DEADLYSIGNAL", probe=True), "FAIL")
        self.assertEqual(classify(1, "ERROR: AddressSanitizer: heap-buffer-overflow", probe=True), "FAIL")
        self.assertEqual(classify(1, "runtime error: overflow\nERROR: AddressSanitizer: stack-overflow", probe=True), "FAIL")

    def test_process_failures(self):
        with tempfile.TemporaryDirectory(dir=ROOT / ".zoi-checks") as tmp:
            report = Path(tmp)
            bad = run_case("bad", [sys.executable, "-c", "raise SystemExit(7)"], report, 3, compile_only=True)
            self.assertEqual(bad["status"], "FAIL")
            slow = run_case("slow", [sys.executable, "-c", "import time; time.sleep(10)"], report, 1)
            self.assertEqual(slow["status"], "TIMEOUT")
            scratch = run_case("scratch", [sys.executable, "-c",
                               "import os, tempfile; print(tempfile.gettempdir()); print(os.environ['TEMP']); print(os.environ['TMP'])"],
                               report, 3, compile_only=True)
            self.assertEqual(scratch["status"], "PASS")
            self.assertEqual((report / "scratch.stdout.log").read_text().splitlines(), [str(report)] * 3)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--compiler", default="g++")
    parser.add_argument("--profile", choices=("lca-vt", "completed-graph"), default="lca-vt")
    parser.add_argument("--report-dir", help="Defaults to .ci-results/stress-PROFILE")
    parser.add_argument("--self-test", action="store_true")
    args = parser.parse_args()
    if sys.platform != "linux":
        parser.error("Run this stack-limit entry on Linux or WSL")
    (ROOT / ".zoi-checks").mkdir(exist_ok=True)
    if args.self_test:
        suite = unittest.defaultTestLoader.loadTestsFromTestCase(FailureTests)
        return 0 if unittest.TextTestRunner().run(suite).wasSuccessful() else 1
    report = (ROOT / (args.report_dir or (".ci-results/stress-" + args.profile))).resolve()
    report.mkdir(parents=True, exist_ok=True)
    completed = args.profile == "completed-graph"
    source = find_suite(ROOT, 'completed_graph_stress_check.cpp') if completed else SOURCE
    marker = "completed_graph_stress_check passed" if completed else "lca_vt_stress_check passed"
    size = "200000" if completed else "1000000"
    before = snapshot(source)
    results = []
    version = subprocess.check_output([args.compiler, "--version"], text=True, timeout=30)
    (report / "compiler.log").write_text(version)
    common = [args.compiler, "-std=c++20", "-Wall", "-Wextra", "-Werror", "-UNDEBUG"]
    with tempfile.TemporaryDirectory(prefix="lca-vt-", dir=ROOT / ".zoi-checks") as tmp:
        normal, sanitizer = str(Path(tmp) / "normal"), str(Path(tmp) / "sanitizer")
        results.append(run_case("compile-normal", common + ["-O2", source, "-o", normal], report, 180, compile_only=True))
        if results[-1]["status"] == "PASS":
            results.append(run_case("large-shallow", [normal, "--large", size], report, 180, 8192, success_marker=marker))
            results.append(run_case("large-chain", [normal, "--deep", size], report, 180, 262144, success_marker=marker))
        flags = ["-O1", "-g", "-fsanitize=address,undefined", "-fno-sanitize-recover=all",
                 "-fno-omit-frame-pointer", "-D_GLIBCXX_ASSERTIONS"]
        results.append(run_case("compile-sanitizer", common + flags + [source, "-o", sanitizer], report, 180, compile_only=True))
        if results[-1]["status"] == "PASS":
            if completed:
                for mode, stack in (("--large", 8192), ("--deep", 262144)):
                    results.append(run_case("sanitizer-" + mode[2:], [sanitizer, mode, size], report, 180, stack, success_marker=marker))
            else:
                for kind in ("dfn", "hld"):
                    results.append(run_case("stack-" + kind, [sanitizer, "--probe-" + kind, "200000"], report, 60, 8192, probe=True))
    stable = before == snapshot(source)
    failed = not stable or any(r["status"] not in ("PASS", "STACK_LIMIT") for r in results)
    summary = dict(schema="lca-vt-extreme-1", profile=args.profile, compiler=version.strip(), platform=sys.platform,
                   sources_sha256=before, stable=stable, results=results, failed=failed)
    (report / "summary.json").write_text(json.dumps(summary, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
    print("Stress gate: {}; stack probes are listed separately; report: {}".format("FAIL" if failed else "PASS", report))
    return int(failed)


if __name__ == "__main__":
    sys.exit(main())
