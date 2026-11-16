#!/usr/bin/env python3
"""Quick parallel experiment runner (not the official measurement).

python tune.py <exe> [KEY=VAL ...] [--tests glob] [--jobs N]
Runs the exe on the tests with the env vars set, scores with check.py,
prints the per-test scores on one line and the mean.
"""
import glob
import os
import subprocess
import sys
from concurrent.futures import ThreadPoolExecutor

args = sys.argv[1:]
exe = args[0]
env = dict(os.environ)
pattern = "tests/*.in"
jobs = 6
label = []
i = 1
while i < len(args):
    if args[i] == "--tests":
        pattern = args[i + 1]; i += 2
    elif args[i] == "--jobs":
        jobs = int(args[i + 1]); i += 2
    else:
        k, v = args[i].split("=")
        env[k] = v
        label.append(args[i])
        i += 1
tests = sorted(glob.glob(pattern))
tag = "_".join(label).replace("=", "") or "default"
os.makedirs(os.path.join("out", "tune_" + tag), exist_ok=True)


def run(test):
    name = os.path.basename(test)[:-3]
    out = os.path.join("out", "tune_" + tag, name + ".out")
    with open(test) as fin, open(out, "w") as fout:
        subprocess.run([exe], stdin=fin, stdout=fout, env=env)
    r = subprocess.run([sys.executable, "check.py", test, out],
                       capture_output=True, text=True)
    last = r.stdout.strip().splitlines()[-1]
    if not last.startswith("SCORE"):
        return name, -1.0
    return name, float(last.split()[1])


with ThreadPoolExecutor(jobs) as ex:
    res = list(ex.map(run, tests))
print(" ".join(label) or "default")
print("  " + " ".join("%s=%.3f" % (nm.split("_", 1)[-1][:9], sc) for nm, sc in res))
print("  MEAN %.4f  (min %.3f, n=%d)" % (sum(s for _, s in res) / len(res),
                                          min(s for _, s in res), len(res)))
