#!/usr/bin/env python3
"""Run solutions on every local test and print a score table.

Run from inside a problem folder that has check.py and tests/*.in:

    python3 ../tools/run_tests.py --tl 2 sol_v1.py sol_v2.cpp

- .py files run with python3; .cpp files are compiled with $CXX (default g++).
- Each solution gets the test on stdin. Its answer is taken from stdout, or
  from the file named by the OUTPUT_PATH environment variable if the judge's
  code template writes there.
- check.py must print 'SCORE <value> <min|max>' or 'INVALID: <reason>'.
- 'rel' is closeness to the best answer among the solutions run, out of 100.
- Every run is appended to scores.jsonl here; tools/board.py shows the history.
"""
import argparse
import glob
import json
import os
import subprocess
import sys
import time


def build(sol):
    """Return the command that runs this solution, compiling if needed."""
    if sol.endswith(".py"):
        return [sys.executable, sol]
    if sol.endswith(".cpp"):
        os.makedirs(".bin_astra", exist_ok=True)
        exe = os.path.join(".bin_astra", os.path.basename(sol)[:-4])
        cxx = os.environ.get("CXX", "g++").split()
        r = subprocess.run(cxx + ["-O2", "-std=c++17", "-o", exe, sol],
                           capture_output=True, text=True)
        if r.returncode != 0:
            print(f"COMPILE ERROR in {sol}:\n{r.stderr}")
            sys.exit(1)
        return [exe]
    return [os.path.abspath(sol)]


def run_one(cmd, test, out_path, kill_after):
    """Run one test. Returns (status, seconds)."""
    if os.path.exists(out_path):
        os.remove(out_path)
    env = dict(os.environ, OUTPUT_PATH=os.path.abspath(out_path))
    start = time.perf_counter()
    try:
        with open(test) as fin:
            r = subprocess.run(cmd, stdin=fin, capture_output=True, text=True,
                               timeout=kill_after, env=env)
    except subprocess.TimeoutExpired:
        return "KILLED", time.perf_counter() - start
    secs = time.perf_counter() - start
    if r.returncode != 0:
        tail = r.stderr.strip().splitlines()[-1:] or ["no error text"]
        return f"CRASH: {tail[0][:60]}", secs
    wrote_file = os.path.exists(out_path) and os.path.getsize(out_path) > 0
    if not wrote_file:
        with open(out_path, "w") as f:
            f.write(r.stdout)
    return "OK", secs


def check(test, out_path):
    """Returns (value, direction) or (None, reason)."""
    r = subprocess.run([sys.executable, "check_astra.py", test, out_path],
                       capture_output=True, text=True)
    lines = r.stdout.strip().splitlines()
    last = lines[-1] if lines else ""
    parts = last.split()
    if r.returncode == 0 and len(parts) == 3 and parts[0] == "SCORE":
        return float(parts[1]), parts[2]
    reason = last or (r.stderr.strip().splitlines() or ["checker printed nothing"])[-1]
    return None, reason[:60]


def fmt(value):
    return str(int(value)) if value == int(value) else f"{value:.6g}"


def closeness(value, best, direction):
    if value == best:
        return 100.0
    if direction == "min":
        return 100.0 * best / value if value > 0 else 0.0
    return 100.0 * value / best if best > 0 else 0.0


def main():
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("solutions", nargs="+")
    ap.add_argument("--tl", type=float, required=True,
                    help="the judge's time limit in seconds")
    ap.add_argument("--tests", default="tests/*.in")
    args = ap.parse_args()

    if not os.path.exists("check_astra.py"):
        sys.exit("No check.py here. Run this from inside the problem folder.")
    tests = sorted(glob.glob(args.tests))
    if not tests:
        sys.exit(f"No tests match {args.tests}")

    cmds = {s: build(s) for s in args.solutions}
    results = {}   # (test, sol) -> (value or None, note, seconds)
    direction = "min"
    for test in tests:
        name = os.path.basename(test)[:-3]
        for sol in args.solutions:
            out_dir = os.path.join("out_astra", os.path.basename(sol))
            os.makedirs(out_dir, exist_ok=True)
            out_path = os.path.join(out_dir, name + "_astra.out")
            status, secs = run_one(cmds[sol], test, out_path, args.tl * 2 + 1)
            if status != "OK":
                results[test, sol] = (None, status, secs)
                continue
            value, info = check(test, out_path)
            if value is None:
                results[test, sol] = (None, info, secs)
            else:
                direction = info
                note = "TOO SLOW" if secs > args.tl else ""
                results[test, sol] = (value, note, secs)

    width = max(len(os.path.basename(t)) for t in tests) - 3
    width = max(width, 4)
    col = 30
    print(f"\n{'test':<{width}}  " + "  ".join(f"{s:<{col}}" for s in args.solutions))
    totals = {s: [] for s in args.solutions}
    problems = {s: 0 for s in args.solutions}
    slowest = {s: 0.0 for s in args.solutions}
    for test in tests:
        values = [results[test, s][0] for s in args.solutions
                  if results[test, s][0] is not None and not results[test, s][1]]
        best = (min(values) if direction == "min" else max(values)) if values else None
        cells = []
        for s in args.solutions:
            value, note, secs = results[test, s]
            slowest[s] = max(slowest[s], secs)
            if value is None or note:
                problems[s] += 1
                totals[s].append(0.0)
                shown = note if value is None else f"{fmt(value)} {note}"
                cells.append(f"{shown[:col - 8]:<{col - 8}} {secs:5.2f}s ")
            else:
                rel = closeness(value, best, direction)
                totals[s].append(rel)
                cells.append(f"{fmt(value):<12} rel {rel:5.1f} {secs:5.2f}s ")
        print(f"{os.path.basename(test)[:-3]:<{width}}  " + "  ".join(f"{c:<{col}}" for c in cells))

    print(f"\nobjective: {direction} is better; time limit {args.tl:g}s")
    for s in args.solutions:
        avg = sum(totals[s]) / len(totals[s])
        verdict = "all valid and in time" if problems[s] == 0 else f"{problems[s]} FAILED (scored 0)"
        print(f"{s:<{col}} mean rel {avg:5.1f}   slowest {slowest[s]:.2f}s   {verdict}")
    save_runs(args, tests, results, direction)
    sys.exit(1 if problems[args.solutions[-1]] else 0)


def save_runs(args, tests, results, direction):
    """Append one line per solution to scores.jsonl, for tools/board.py."""
    now = time.time()
    with open("scores_astra.jsonl", "a") as f:
        for s in args.solutions:
            per_test = {}
            for test in tests:
                value, note, secs = results[test, s]
                per_test[os.path.basename(test)[:-3]] = {
                    "v": value, "ok": value is not None and not note,
                    "s": round(secs, 3)}
            f.write(json.dumps({"type": "run", "t": now, "sol": s, "tl": args.tl,
                                "dir": direction, "tests": per_test}) + "\n")


if __name__ == "__main__":
    main()
