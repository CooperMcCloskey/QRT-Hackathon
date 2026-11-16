#!/usr/bin/env python3
"""Fit a concave quadratic to black-box probes and predict the best value.

    python3 tools/bbfit.py --target 0.0148  0:0  1:-0.5035  -1:-0.21

Each W:F pair is one probe: value returned W, raw noisy payoff F.
--target is the judge's raw_noiseless_target for that test case.

Model (an assumption, not a fact): f(w) = a*w - b*w^2 with b > 0, so f(0) = 0.
If the target is the true noiseless maximum then a^2 / (4b) = target, which
leaves one free parameter (b) plus the sign of a. Both fits are printed.
"""
import math
import sys


def parse(argv):
    target, pts = None, []
    i = 0
    while i < len(argv):
        if argv[i] == "--target":
            target = float(argv[i + 1])
            i += 2
            continue
        w, f = argv[i].split(":")
        pts.append((float(w), float(f)))
        i += 1
    return target, pts


def free_fit(pts):
    """Least squares for f = a w - b w^2 (through the origin)."""
    # f = a w + c w^2, normal equations, then b = -c.
    s2 = sum(w ** 2 for w, _ in pts)
    s3 = sum(w ** 3 for w, _ in pts)
    s4 = sum(w ** 4 for w, _ in pts)
    y1 = sum(w * f for w, f in pts)
    y2 = sum(w * w * f for w, f in pts)
    det = s2 * s4 - s3 * s3
    if abs(det) < 1e-12:
        return None
    a = (y1 * s4 - y2 * s3) / det
    c = (s2 * y2 - s3 * y1) / det
    return a, -c


def constrained_fit(pts, target):
    """For each sign of a, the b > 0 that best fits, with a^2 = 4 b target."""
    out = []
    for sign in (+1, -1):
        best = None
        for k in range(-4000, 4001):
            b = 10 ** (k / 1000)
            a = sign * 2 * math.sqrt(target * b)
            sse = sum((a * w - b * w * w - f) ** 2 for w, f in pts)
            if best is None or sse < best[0]:
                best = (sse, a, b)
        out.append(best)
    return sorted(out)


def describe(label, a, b, target):
    if b <= 0:
        print(f"{label}: a={a:.4f} b={b:.4f}  NOT CONCAVE (no finite best value)")
        return
    w_star = a / (2 * b)
    peak = a * a / (4 * b)
    print(f"{label}: a={a:.4f} b={b:.4f}  best w = {w_star:.4f}  "
          f"predicted payoff {peak:.4f}" + (f" ({peak / target:.2f} x target)" if target else ""))
    if target:
        for d in (0.01, 0.02, 0.05):
            print(f"    if w is off by {d}: normalised score about "
                  f"{max(0.0, (peak - b * d * d) / target):.2f}")


def main():
    target, pts = parse(sys.argv[1:])
    if len(pts) < 2:
        sys.exit(__doc__)
    nz = [p for p in pts if p[0] != 0]
    print(f"{len(pts)} probes; f(0) = {dict(pts).get(0.0, 'not probed')}")
    fit = free_fit(pts)
    if fit and len(nz) >= 2:
        describe("free fit       ", fit[0], fit[1], target)
    if target:
        fits = constrained_fit(pts, target)
        for rank, (sse, a, b) in enumerate(fits):
            tag = "best" if rank == 0 else "other"
            describe(f"target fit {tag:<5}", a, b, target)
            print(f"    residual sum of squares {sse:.5f}")


if __name__ == "__main__":
    main()
