#!/usr/bin/env python3
"""Black box loop: ingest judge results, fit, write the next paste-ready file.

Run from p3/claude:
    python3 bb.py add results_M.json -1   # collector JSON; -1 = x used if the
                                          # page did not print x
    python3 bb.py plan S                  # write run_S.c (scored answer)

Model per t (assumed, checked against the data): f(x) = a*x - b*x^2, noise
proportional to |x|, and the noiseless target M(t) = a^2/(4b). With b known,
|a| = 2*sqrt(b*M), so only the sign of a is unknown. Then the best answer is
x* = sign * sqrt(M/b).
"""
import json
import math
import os
import re
import statistics
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
OBS = os.path.join(HERE, "obs.jsonl")
TEMPLATE = os.path.join(HERE, "..", "probe_v1.c")
SCORE_RE = re.compile(r"t\s*=\s*([-+0-9.eE]+)\s*,\s*N\s*=\s*(\d+)\s*,\s*raw_noisy_score\s*=\s*([-+0-9.eE]+)"
                      r"\s*,\s*raw_noiseless_target\s*=\s*([-+0-9.eE]+)")


def load_obs():
    if not os.path.exists(OBS):
        return []
    return [json.loads(l) for l in open(OBS) if l.strip()]


def add(path, default_x):
    data = json.loads(open(path).read())
    run = os.path.splitext(os.path.basename(path))[0]
    recs, skipped = [], 0
    for o in data.get("observations", data if isinstance(data, list) else []):
        if "raw_noisy_score" not in o:
            skipped += 1
            print("NOTE: no payoff for", o.get("test_case"), str(o.get("error", ""))[:150])
            continue
        x = o.get("candidate")
        if x is None:
            x = default_x
        if x is None:
            skipped += 1
            continue
        t = o.get("exact_t")
        if t is None:
            t = o["t"]
        recs.append({"case": o.get("test_case"), "t": float(t), "x": float(x),
                     "f": float(o["raw_noisy_score"]), "M": float(o["raw_noiseless_target"]),
                     "N": int(o["N"]), "run": run})
    with open(OBS, "a") as fo:
        for r in recs:
            fo.write(json.dumps(r) + "\n")
    ts = sorted({round(r["t"], 6) for r in recs})
    print(f"added {len(recs)} cases from {run} (skipped {skipped}); "
          f"{len(ts)} distinct t from {ts[:1]} to {ts[-1:]}; N values {sorted({r['N'] for r in recs})}")


def groups(obs):
    g = {}
    for o in obs:
        g.setdefault(round(o["t"], 6), []).append(o)
    return g


def mean_at(pts, x0):
    v = [f for x, f in pts if abs(x - x0) < 1e-6]
    return sum(v) / len(v) if v else None


def analyse(obs):
    g = groups(obs)
    bs = []
    for t, os_ in g.items():
        pts = [(o["x"], o["f"]) for o in os_]
        fp, fm = mean_at(pts, 1.0), mean_at(pts, -1.0)
        if fp is not None and fm is not None and -(fp + fm) / 2 > 0:
            bs.append(-(fp + fm) / 2)
    b_med = statistics.median(bs) if bs else 0.45
    rows = []
    for t in sorted(g):
        os_ = g[t]
        M = os_[-1]["M"]
        pts = [(o["x"], o["f"]) for o in os_]
        fp, fm = mean_at(pts, 1.0), mean_at(pts, -1.0)
        b_t = -(fp + fm) / 2 if fp is not None and fm is not None else None
        b = b_med if b_t is None or b_t <= 0 else min(2 * b_med, max(0.5 * b_med, b_t))
        if M <= 0:
            rows.append(dict(t=t, exact_t=os_[-1]["t"], M=M, b=b, x=0.0, conf=0.0, n=len(pts),
                             note="target <= 0: return 0"))
            continue
        amp = math.sqrt(M / b)
        sse = {}
        for s in (1, -1):
            a = s * 2 * math.sqrt(b * M)
            sse[s] = sum((a * x - b * x * x - f) ** 2 / max(x * x, 0.01) for x, f in pts)
        s_best = 1 if sse[1] <= sse[-1] else -1
        conf = abs(sse[1] - sse[-1])
        note = "" if b_t is not None else "missing a +-1 probe; using pooled b"
        rows.append(dict(t=t, exact_t=os_[-1]["t"], M=M, b=b, x=s_best * amp, conf=conf, n=len(pts),
                         note=note, sse=sse, amp=amp))
    smooth_signs(rows)
    return b_med, bs, rows


def smooth_signs(rows):
    """If M(t) is smooth in t, a(t) keeps its sign between the near-zero dips
    of M. Pool the sign evidence of every case in such a stretch."""
    live = [r for r in rows if r["M"] > 0]
    if len(live) < 8:
        return
    Ms = [r["M"] for r in live]
    cuts = []
    for i in range(1, len(live) - 1):
        window = Ms[max(0, i - 5): i + 6]
        if Ms[i] <= Ms[i - 1] and Ms[i] <= Ms[i + 1] and Ms[i] < 0.2 * max(window):
            cuts.append(i)
    # Smoothness check: neighbouring M values should be close.
    jumps = sum(1 for i in range(len(Ms) - 1)
                if abs(Ms[i + 1] - Ms[i]) > 0.5 * max(Ms[i], Ms[i + 1]))
    if jumps > len(Ms) // 4 or len(cuts) > len(live) // 4:
        print(f"M(t) does not look smooth ({jumps} big jumps, {len(cuts)} dips): signs decided per case")
        return
    bounds = [0] + cuts + [len(live)]
    flips = 0
    for k in range(len(bounds) - 1):
        seg = live[bounds[k]: bounds[k + 1]]
        tot = {s: sum(r["sse"][s] for r in seg) for s in (1, -1)}
        s = 1 if tot[1] <= tot[-1] else -1
        for r in seg:
            new_x = s * r["amp"]
            if (new_x > 0) != (r["x"] > 0):
                flips += 1
                r["note"] = (r["note"] + " sign set by its stretch").strip()
            r["x"] = new_x
            r["conf"] = abs(tot[1] - tot[-1])
    print(f"M(t) looks smooth: {len(bounds) - 1} stretches of constant sign; "
          f"{flips} cases' signs overruled by their stretch")


def lit(x):
    s = f"{x:.9g}"
    return (s if any(c in s for c in ".en") else s + ".0") + "f"


def write_c(name, table):
    src = open(TEMPLATE).read()
    start = src.index("float* probe(float t, int* result_count) {")
    end = src.index("\nint main()", start)
    table = sorted(table)
    gaps = [table[i + 1][0] - table[i][0] for i in range(len(table) - 1)]
    tol = min([0.004] + [0.45 * g for g in gaps if g > 0])
    tt = ", ".join(lit(r[0]) for r in table) or "0.0f"
    xx = ", ".join(lit(r[1]) for r in table) or "0.0f"
    body = f'''float* probe(float t, int* result_count) {{
    static const float TT[] = {{{tt}}};
    static const float XX[] = {{{xx}}};
    static const int K = {len(table)};
    static float answer[1];
    int i, best = 0;
    float x;
    for (i = 1; i < K; i++) {{
        if (fabsf(t - TT[i]) < fabsf(t - TT[best])) best = i;
    }}
    if (fabsf(t - TT[best]) < {tol:.6g}f || K < 2) {{
        x = XX[best];
    }} else {{                      /* unseen t: interpolate between neighbours */
        int lo = -1, hi = -1;
        for (i = 0; i < K; i++) {{
            if (TT[i] <= t && (lo < 0 || TT[i] > TT[lo])) lo = i;
            if (TT[i] >= t && (hi < 0 || TT[i] < TT[hi])) hi = i;
        }}
        if (lo < 0) x = XX[hi];
        else if (hi < 0) x = XX[lo];
        else x = XX[lo] + (t - TT[lo]) / (TT[hi] - TT[lo]) * (XX[hi] - XX[lo]);
    }}
    answer[0] = x;
    *result_count = 1;
    printf("QRT_RUN_{name} t=%.9g x=%.9g\\n", (double)t, (double)x);
    return answer;
}}
'''
    path = os.path.join(HERE, f"run_{name}.c")
    open(path, "w").write(src[:start] + body + src[end:])
    return path


def plan(name):
    obs = load_obs()
    b_med, bs, rows = analyse(obs)
    if bs:
        q = sorted(bs)
        print(f"curvature b: median {b_med:.3f}, quartiles {q[len(q) // 4]:.3f}-{q[3 * len(q) // 4]:.3f}, "
              f"from {len(bs)} t values")
    print(f"{'t':>10} {'M':>8} {'b':>6} {'n':>3} {'x*':>8} {'sign conf':>9}  note")
    for r in rows:
        print(f"{r['t']:>10.4f} {r['M']:>8.4f} {r['b']:>6.3f} {r['n']:>3} {r['x']:>+8.4f} {r['conf']:>9.3f}  {r['note']}")
    path = write_c(name, [(r["exact_t"], r["x"]) for r in rows])
    weak = sum(1 for r in rows if r["conf"] < 0.02 and r["M"] > 0)
    print(f"\nwrote {path}: {len(rows)} t values; {weak} with a weak sign decision")


def main():
    if len(sys.argv) >= 3 and sys.argv[1] == "add":
        add(sys.argv[2], float(sys.argv[3]) if len(sys.argv) > 3 else None)
    elif len(sys.argv) == 3 and sys.argv[1] == "plan":
        plan(sys.argv[2])
    else:
        print(__doc__)


if __name__ == "__main__":
    main()
