#!/usr/bin/env python3
"""Square packing++ baseline: skyline packing over many strip widths.

Always holds a valid answer (one row), only replaces it with a better packing
that passed an overlap self-check. Stops at 75% of the assumed time limit.
"""
import random
import re
import sys
import time

T0 = time.time()
TIME_LIMIT = 2.0          # ASSUMED: judge limit not known yet
TIME_BUDGET = 0.6 * TIME_LIMIT
random.seed(12345)


def pack(order, sides, width):
    """Bottom-left skyline in a strip. Returns (area, positions) or None."""
    sky = [[0, width, 0]]   # [start, length, height]
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
        pos[i] = (y, x)     # (X = row, Y = column)
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
        if end > used_w:
            used_w = end
        if y + s > used_h:
            used_h = y + s
    return used_w * used_h, pos


def valid(sides, pos):
    n = len(sides)
    if len(pos) != n:
        return False
    order = sorted(range(n), key=lambda i: pos[i][0])
    for a in range(n):
        i = order[a]
        xi, yi = pos[i]
        si = sides[i]
        if xi < 0 or yi < 0:
            return False
        for b in range(a + 1, n):
            j = order[b]
            xj, yj = pos[j]
            if xj >= xi + si:
                break
            if yi < yj + sides[j] and yj < yi + si:
                return False
    return True


def solve(sides):
    n = len(sides)
    if n == 0:
        return []
    row, x = [], 0
    for s in sides:
        row.append((0, x))
        x += s
    best = [x * max(sides), row, x]
    try:
        order = sorted(range(n), key=lambda i: -sides[i])
        lo, hi = max(sides), sum(sides)
        total = sum(s * s for s in sides)

        def attempt(cand, width):
            r = pack(cand, sides, width)
            if r is not None and r[0] < best[0]:
                best[0], best[1], best[2] = r[0], r[1], width
                return True
            return False

        # Widths near sqrt(total) matter most; sweep a spread of widths.
        root = int(total ** 0.5)
        widths = sorted(set(max(lo, min(hi, int(root * f)))
                            for f in [0.5 + 0.02 * k for k in range(101)]))
        widths.sort(key=lambda w: abs(w - root))
        for w in widths:
            if time.time() - T0 > TIME_BUDGET * 0.5:
                break
            attempt(order, w)
        while n > 1 and time.time() - T0 < TIME_BUDGET:
            cand = order[:]
            i, j = random.randrange(n), random.randrange(n)
            cand[i], cand[j] = cand[j], cand[i]
            spread = max(1, best[2] // 20)
            width = min(hi, max(lo, best[2] + random.randint(-spread, spread)))
            if attempt(cand, width):
                order = cand
        if valid(sides, best[1]):
            return best[1]
    except Exception:
        pass
    return row


def read_sides(text):
    rows = []
    for line in text.splitlines():
        nums = [int(t) for t in re.findall(r"\d+", line)]
        if nums:
            rows.append(nums)
    flat = [v for r in rows for v in r]
    if len(rows) >= 2 and len(rows[0]) == 1 and rows[0][0] == len(flat) - 1:
        flat = flat[1:]
    return flat


def main():
    sides = read_sides(sys.stdin.read())
    res = solve(sides)
    sys.stdout.write("".join("%d %d\n" % (px, py) for px, py in res))


if __name__ == "__main__":
    main()
