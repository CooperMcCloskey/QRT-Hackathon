#!/usr/bin/env python3
"""Analyse a scan run: payoff per case normalised by its target, grouped by u.

    python3 an.py results_S1.json plan_S1.json
"""
import json
import math
import statistics
import sys

res = json.load(open(sys.argv[1]))
plan = json.load(open(sys.argv[2]))
T = json.load(open("targets.json"))
obs = res["observations"]
print("reader:", res.get("method"), res.get("diag"))
rows = []
for o in obs:
    if "raw_noisy_score" not in o:
        print("missing:", o)
        continue
    k = int(round(o["t"] / 0.02))
    x_plan = plan["x"][k]
    if o.get("candidate") is not None and abs(o["candidate"] - x_plan) > 1e-3:
        print(f"WARNING case t={o['t']}: page x={o['candidate']} but plan x={x_plan}")
    M = o["raw_noiseless_target"]
    if abs(M - T["M"][k]) > 1e-4:
        print(f"WARNING t={o['t']}: target {M} differs from saved {T['M'][k]}")
    rows.append(dict(k=k, t=o["t"], u=plan["u"][k], x=x_plan, f=o["raw_noisy_score"], M=M, g=o["raw_noisy_score"] / M))
seen = {}
for r in rows:
    seen.setdefault(r["k"], r)
rows = sorted(seen.values(), key=lambda r: r["k"])
print(f"{len(rows)} distinct cases")
by_u = {}
for r in rows:
    by_u.setdefault(r["u"], []).append(r["g"])
print(f"{'u':>6} {'n':>3} {'mean f/M':>9} {'median':>8}  values")
for u in sorted(by_u):
    v = by_u[u]
    print(f"{u:>6} {len(v):>3} {statistics.mean(v):>9.3f} {statistics.median(v):>8.3f}  "
          + " ".join(f"{x:.2f}" for x in v))
for lo, hi in ((0, 0.2), (0.2, 0.8), (0.8, 1.01)):
    sub = {}
    for r in rows:
        if lo <= r["t"] < hi:
            sub.setdefault(r["u"], []).append(r["g"])
    print(f"t in [{lo},{hi}): " + ", ".join(f"u={u}: {statistics.mean(v):.2f}" for u, v in sorted(sub.items())))
json.dump(rows, open(sys.argv[1].replace(".json", "_rows.json"), "w"))
