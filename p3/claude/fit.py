#!/usr/bin/env python3
"""Fit every case from all saved runs and write a scored file.

    python3 fit.py NAME [--smooth 2]

Reads feedback/collector_*.json (Codex's batch format) and claude/results_*.json.
Per case k (t = 0.02 k): estimates slope a and curvature b at x = 0 from the
+-h probes (Richardson over h = 0.5 and 1 when both exist), smooths them over
neighbouring t, and returns x* in two ways:
  xq = a/(2b)               (local quadratic peak)
  xm = sign(a)*sqrt(M/b)    (uses the judge's noiseless target M)
"""
import glob
import json
import math
import os
import statistics
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
import mk

T = json.load(open(os.path.join(HERE, "targets.json")))


def load():
    pts = {k: [] for k in range(51)}
    files = sorted(glob.glob(os.path.join(HERE, "..", "feedback", "collector_*.json")))
    files += sorted(glob.glob(os.path.join(HERE, "results_*.json")))
    seen = set()
    for path in files:
        data = json.load(open(path))
        rounds = data.get("rounds") or [{"id": os.path.basename(path), "values": None,
                                          "observations": data.get("observations", [])}]
        for r in rounds:
            vals = r.get("values")
            for o in r.get("observations", []):
                if "raw_noisy_score" not in o or o.get("N") != 1:
                    continue
                x = o.get("candidate")
                if x is None and isinstance(vals, list) and len(vals) == 1:
                    x = vals[0]
                if x is None:
                    continue
                k = int(round(o["t"] / 0.02))
                key = (r.get("id"), k, o["raw_noisy_score"])
                if key in seen:
                    continue
                seen.add(key)
                pts[k].append((float(x), float(o["raw_noisy_score"])))
    return pts


def avg_at(p, x0):
    v = [f for x, f in p if abs(x - x0) < 1e-6]
    return sum(v) / len(v) if v else None


def main():
    name = sys.argv[1]
    w = int(sys.argv[sys.argv.index("--smooth") + 1]) if "--smooth" in sys.argv else 2
    pts = load()
    a0, b0 = [None] * 51, [None] * 51
    for k in range(51):
        est = {}
        for h in (0.5, 1.0):
            yp, ym = avg_at(pts[k], h), avg_at(pts[k], -h)
            if yp is not None and ym is not None:
                est[h] = ((yp - ym) / (2 * h), -(yp + ym) / (2 * h * h))
        if 0.5 in est and 1.0 in est:
            a0[k] = (4 * est[0.5][0] - est[1.0][0]) / 3
            b0[k] = (4 * est[0.5][1] - est[1.0][1]) / 3
        elif est:
            a0[k], b0[k] = est[min(est)]
    have = [k for k in range(51) if a0[k] is not None]
    print(f"cases with +-h data: {len(have)}; points per case: "
          f"{statistics.median([len(pts[k]) for k in range(51)])}")
    if not have:
        sys.exit("no usable data yet")
    sm = lambda arr, k: statistics.mean([arr[j] for j in range(max(0, k - w), min(51, k + w + 1))
                                         if arr[j] is not None] or [0.0])
    A = [sm(a0, k) for k in range(51)]
    B = [max(sm(b0, k), 0.05) for k in range(51)]
    xs, rows = [], []
    for k in range(51):
        M = T["M"][k]
        xq = A[k] / (2 * B[k])
        xm = math.copysign(math.sqrt(M / B[k]), A[k])
        ratio = A[k] ** 2 / (4 * B[k]) / M
        rows.append((k, M, A[k], B[k], xq, xm, ratio))
    print(f"{'t':>5} {'M':>7} {'a0':>8} {'b0':>7} {'xq':>8} {'xm':>8} {'peak/M':>7}")
    for k, M, a, b, xq, xm, ratio in rows:
        print(f"{0.02 * k:>5.2f} {M:>7.4f} {a:>+8.4f} {b:>7.4f} {xq:>+8.4f} {xm:>+8.4f} {ratio:>7.2f}")
    ratios = [r[6] for r in rows]
    med = statistics.median(ratios)
    mode = "xq" if 0.5 <= med <= 2 else "xm"
    for k, M, a, b, xq, xm, ratio in rows:
        xs.append(round(xq if mode == "xq" else xm, 5))
    print(f"\nmedian predicted peak/target = {med:.2f}; using {mode}")
    json.dump({"x": xs, "mode": mode, "a": A, "b": B}, open(os.path.join(HERE, f"plan_{name}.json"), "w"))
    print(mk.write(name, xs))


if __name__ == "__main__":
    main()
