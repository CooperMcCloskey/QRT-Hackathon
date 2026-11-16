#!/usr/bin/env python3
"""Team scoreboard, built from the scores.jsonl files that run_tests.py writes.

    python3 tools/board.py                show every problem
    python3 tools/board.py --watch 10     refresh every 10 seconds

Inside a problem folder, after submitting a version to the judge:

    python3 ../tools/board.py judge sol_v2.py 87.5

Once the contest has started, to share scores and code between laptops:

    python3 tools/board.py sync           commit, pull, push

'close' is the mean closeness to the best answer any logged version has
reached on each test, out of 100. A failed test counts as 0.
"""
import json
import os
import subprocess
import sys
import time

from run_tests import closeness

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
SKIP = {"tools", "templates"}


def problem_folders():
    found = []
    for path, dirs, files in os.walk(ROOT):
        dirs[:] = sorted(d for d in dirs if not d.startswith(".") and d not in SKIP)
        rel = os.path.relpath(path, ROOT)
        if rel.count(os.sep) >= 2:
            dirs[:] = []
        if "statement.md" in files or "scores.jsonl" in files:
            found.append(rel)
    return found


def load(folder):
    path = os.path.join(ROOT, folder, "scores.jsonl")
    entries = []
    if os.path.exists(path):
        for line in open(path):
            line = line.strip()
            if not line:
                continue
            try:
                entries.append(json.loads(line))
            except ValueError:
                pass   # a half-written or merge-damaged line; skip it
    return sorted(entries, key=lambda e: e.get("t", 0))


def summarise(entries):
    """Returns (direction, versions, submitted version name or None)."""
    runs = [e for e in entries if e.get("type") == "run"]
    judges = [e for e in entries if e.get("type") == "judge"]
    direction = runs[-1]["dir"] if runs else "min"
    better = min if direction == "min" else max

    best = {}
    for run in runs:
        for name, r in run["tests"].items():
            if r["ok"]:
                best[name] = better(best[name], r["v"]) if name in best else r["v"]

    versions = {}
    for run in runs:   # later runs of the same version replace earlier ones
        rels = [closeness(r["v"], best[name], direction) if r["ok"] else 0.0
                for name, r in run["tests"].items()]
        versions[run["sol"]] = {
            "close": sum(rels) / len(rels) if rels else 0.0,
            "failed": sum(1 for r in run["tests"].values() if not r["ok"]),
            "slowest": max([r["s"] for r in run["tests"].values()] or [0.0]),
            "tl": run.get("tl"), "t": run["t"], "judge": None}
    for j in judges:
        versions.setdefault(j["sol"], {"close": None, "failed": None, "slowest": None,
                                       "tl": None, "t": j["t"], "judge": None})
        versions[j["sol"]]["judge"] = j["score"]
    submitted = judges[-1]["sol"] if judges else None
    return direction, versions, submitted


def age(t):
    mins = int((time.time() - t) / 60)
    return "just now" if mins < 1 else f"{mins} min ago" if mins < 120 else f"{mins // 60} h ago"


def best_local(versions):
    ok = [(v["close"], v["t"], name) for name, v in versions.items()
          if v["close"] is not None and v["failed"] == 0]
    return max(ok)[2] if ok else None


def render():
    lines = [f"QRT C3 board   {time.strftime('%H:%M:%S')}", ""]
    lines.append(f"{'problem':<18}{'submitted':<16}{'judge':>8}  {'best local':<16}{'close':>6}  "
                 f"{'last run':<12}note")
    details = []
    for folder in problem_folders():
        entries = load(folder)
        if not entries:
            lines.append(f"{folder:<18}no runs yet")
            continue
        direction, versions, submitted = summarise(entries)
        top = best_local(versions)
        judge = versions[submitted]["judge"] if submitted else None
        note = ""
        if top is None:
            note = "NO VALID VERSION"
        elif submitted is None:
            note = "NOTHING SUBMITTED"
        elif top != submitted and (versions[submitted]["close"] is None or
                                   versions[top]["close"] > versions[submitted]["close"]):
            note = f"SUBMIT {top}?"
        last = max(v["t"] for v in versions.values())
        lines.append(
            f"{folder:<18}{(submitted or '-'):<16}"
            f"{(f'{judge:g}' if judge is not None else '-'):>8}  "
            f"{(top or '-'):<16}"
            f"{(format(versions[top]['close'], '.1f') if top else '-'):>6}  "
            f"{age(last):<12}{note}")

        details.append("")
        details.append(f"{folder}   ({direction} is better)")
        details.append(f"  {'version':<18}{'close':>6}{'failed':>8}{'slowest':>9}{'judge':>8}   run")
        for name, v in sorted(versions.items(), key=lambda kv: kv[1]["t"]):
            mark = "*" if name == submitted else " "
            close = "-" if v["close"] is None else f"{v['close']:.1f}"
            failed = "-" if v["failed"] is None else str(v["failed"])
            slowest = "-" if v["slowest"] is None else f"{v['slowest']:.2f}s"
            judged = "-" if v["judge"] is None else f"{v['judge']:g}"
            details.append(f" {mark}{name:<18}{close:>6}{failed:>8}{slowest:>9}{judged:>8}   {age(v['t'])}")
    if details:
        details.append("")
        details.append("* = the version last submitted to the judge")
    return "\n".join(lines + details)


def record_judge(argv):
    if len(argv) != 2:
        sys.exit("usage: python3 ../tools/board.py judge <solution file> <judge score>")
    sol, score = argv[0], float(argv[1])
    if os.path.abspath(os.getcwd()) == ROOT or not os.path.exists("check.py"):
        sys.exit("Run this from inside the problem folder (the one with check.py).")
    with open("scores.jsonl", "a") as f:
        f.write(json.dumps({"type": "judge", "t": time.time(), "sol": sol, "score": score}) + "\n")
    print(f"recorded: {sol} scored {score:g} on the judge")


def sync():
    def git(*args):
        return subprocess.run(["git", "-C", ROOT] + list(args), capture_output=True, text=True)
    git("add", "-A")
    git("commit", "-q", "-m", f"sync {time.strftime('%H:%M')}")   # nothing to commit is fine
    for step in (("pull", "--no-rebase", "--no-edit", "-q"), ("push", "-q")):
        r = git(*step)
        if r.returncode != 0:
            sys.exit(f"git {step[0]} failed:\n{(r.stderr or r.stdout).strip()}")
    print("synced")


def main():
    argv = sys.argv[1:]
    if argv[:1] == ["judge"]:
        record_judge(argv[1:])
    elif argv[:1] == ["sync"]:
        sync()
    elif argv[:1] == ["--watch"]:
        every = float(argv[1]) if len(argv) > 1 else 10
        try:
            while True:
                print("\033[2J\033[H" + render(), flush=True)
                time.sleep(every)
        except KeyboardInterrupt:
            pass
    elif argv:
        print(__doc__)
    else:
        print(render())


if __name__ == "__main__":
    main()
