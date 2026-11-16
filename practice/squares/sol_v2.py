#!/usr/bin/env python3
"""Skyline packing over many strip widths, then timed random improvement.

Holds a valid answer at all times: starts from the one-row baseline and only
replaces it with a better valid packing. Stops at 75% of the time limit.
"""
import random
import sys
import time

T0 = time.perf_counter()
TIME_LIMIT = 8.0
TIME_BUDGET = 0.75 * TIME_LIMIT
random.seed(12345)


def pack(order, sides, width):
    """Place squares in this order at the lowest free spot in a strip.

    Returns (area, positions), or None if a square is wider than the strip.
    """
    sky = [[0, width, 0]]   # segments [x, length, height], left to right
    pos = [None] * len(sides)
    used_w = used_h = 0
    for i in order:
        s = sides[i]
        best = None
        for k in range(len(sky)):
            x = sky[k][0]
            if x + s > width:
                break
            y, j, left = 0, k, s
            while left > 0:
                if sky[j][2] > y:
                    y = sky[j][2]
                left -= sky[j][1]
                j += 1
            if best is None or y < best[0]:
                best = (y, x)
        if best is None:
            return None
        y, x = best
        pos[i] = (x, y)
        end = x + s
        new = []
        for sx, sw, sh in sky:
            if sx + sw <= x or sx >= end:
                new.append([sx, sw, sh])
            else:
                if sx < x:
                    new.append([sx, x - sx, sh])
                if sx + sw > end:
                    new.append([end, sx + sw - end, sh])
        new.append([x, s, y + s])
        new.sort()
        sky = new
        used_w = max(used_w, end)
        used_h = max(used_h, y + s)
    return used_w * used_h, pos


def main():
    data = sys.stdin.read().split()
    n = int(data[0])
    sides = [int(x) for x in data[1:1 + n]]

    # Baseline: one row. Valid by construction.
    best_pos, x = [], 0
    for s in sides:
        best_pos.append((x, 0))
        x += s
    best_area = x * max(sides)

    order = sorted(range(n), key=lambda i: -sides[i])
    lo, hi = max(sides), sum(sides)
    best_width = hi

    def attempt(candidate_order, width):
        nonlocal best_area, best_pos, best_width
        result = pack(candidate_order, sides, width)
        if result is not None and result[0] < best_area:
            best_area, best_pos = result
            best_width = width
            return True
        return False

    # Sweep strip widths with the largest-first order.
    steps = min(hi - lo + 1, 200)
    for t in range(steps):
        if time.perf_counter() - T0 > TIME_BUDGET * 0.4:
            break
        attempt(order, lo + (hi - lo) * t // max(1, steps - 1))

    # Random changes to the order and width; keep any that improve.
    while n > 1 and time.perf_counter() - T0 < TIME_BUDGET:
        cand = order[:]
        i, j = random.randrange(n), random.randrange(n)
        cand[i], cand[j] = cand[j], cand[i]
        spread = max(1, best_width // 20)
        width = min(hi, max(lo, best_width + random.randint(-spread, spread)))
        if attempt(cand, width):
            order = cand

    sys.stdout.write("".join(f"{px} {py}\n" for px, py in best_pos))


if __name__ == "__main__":
    main()
