#!/usr/bin/env python3
"""Checker for Square packing++.

Usage:  python check.py <input file> <output file>
Prints: 'SCORE <(L/A)^20> max' or 'INVALID: <reason>' (exit code 1).
Written from the statement alone; shares no code with any solver.
"""
import re
import sys


def fail(reason):
    print("INVALID: " + reason)
    sys.exit(1)


def read_sides(path):
    rows = []
    for line in open(path).read().splitlines():
        nums = [int(t) for t in re.findall(r"\d+", line)]
        if nums:
            rows.append(nums)
    flat = [v for r in rows for v in r]
    if len(rows) >= 2 and len(rows[0]) == 1 and rows[0][0] == len(flat) - 1:
        flat = flat[1:]
    return flat


def lower_bound(sides):
    m = max(sides)
    s = sum(v * v for v in sides)
    best = None
    w = m
    while True:
        h = max(m, -(-s // w))
        if best is None or w * h < best:
            best = w * h
        if h == m:
            break
        w += 1
    return best


def main():
    sides = read_sides(sys.argv[1])
    n = len(sides)
    lines = [l for l in open(sys.argv[2]).read().splitlines() if l.strip()]
    if len(lines) != n:
        fail("expected %d lines, got %d" % (n, len(lines)))
    xs, ys = [], []
    for k, line in enumerate(lines):
        if "." in line or "e" in line.lower():
            fail("line %d is not integer: %r" % (k + 1, line))
        toks = re.findall(r"-?\d+", line)
        if len(toks) != 2:
            fail("line %d needs 2 integers: %r" % (k + 1, line))
        xs.append(int(toks[0]))
        ys.append(int(toks[1]))

    # Overlap test: open interiors intersect on both axes. Sweep over x order.
    order = sorted(range(n), key=lambda i: xs[i])
    for a in range(n):
        i = order[a]
        for b in range(a + 1, n):
            j = order[b]
            if xs[j] >= xs[i] + sides[i]:
                break
            if ys[i] < ys[j] + sides[j] and ys[j] < ys[i] + sides[i]:
                fail("squares %d and %d overlap" % (i, j))

    height = max(xs[i] + sides[i] for i in range(n)) - min(xs)
    width = max(ys[i] + sides[i] for i in range(n)) - min(ys)
    area = height * width
    lb = lower_bound(sides)
    ratio = float(lb) / area
    print("Valid packing. BoundingArea=%d (%dx%d), LowerBound=%d, Ratio=%.4f"
          % (area, height, width, lb, ratio))
    print("SCORE %.6f max" % (ratio ** 20))


if __name__ == "__main__":
    main()
