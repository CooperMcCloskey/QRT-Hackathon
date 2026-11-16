#!/usr/bin/env python3
"""Masyu checker. Usage: python3 check.py <input> <output>"""
import sys

DIRECTION = "min"
CONN = {'.': '', '-': 'EW', '|': 'NS', 'L': 'NE', 'J': 'NW', 'F': 'SE', '7': 'SW'}
D = {'N': (-1, 0), 'S': (1, 0), 'E': (0, 1), 'W': (0, -1)}
OPP = {'N': 'S', 'S': 'N', 'E': 'W', 'W': 'E'}


def fail(reason):
    print(f"INVALID: {reason}")
    sys.exit(1)


def main():
    inp = open(sys.argv[1]).read().split()
    R = int(inp[0])
    g = inp[1:1 + R]
    C = len(g[0])
    out = open(sys.argv[2]).read().split()
    if len(out) != R:
        fail(f"expected {R} rows, got {len(out)}")
    for r in range(R):
        if len(out[r]) != C:
            fail(f"row {r} has length {len(out[r])}")
        for ch in out[r]:
            if ch not in CONN:
                fail(f"illegal char {ch!r}")
    cells = []
    for r in range(R):
        for c in range(C):
            ch = out[r][c]
            for d in CONN[ch]:
                nr, nc = r + D[d][0], c + D[d][1]
                if not (0 <= nr < R and 0 <= nc < C):
                    fail(f"({r},{c}) leaves grid")
                if OPP[d] not in CONN[out[nr][nc]]:
                    fail(f"({r},{c}) dir {d} not reciprocated")
            if ch != '.':
                cells.append((r, c))
    if not cells:
        fail("empty loop")
    # single loop: walk from first cell
    start = cells[0]
    prev, cur, n = None, start, 0
    while True:
        n += 1
        nxt = None
        for d in CONN[out[cur[0]][cur[1]]]:
            q = (cur[0] + D[d][0], cur[1] + D[d][1])
            if q != prev:
                nxt = q
                break
        if nxt is None:  # 2-cycle impossible with distinct dirs; safety
            fail("degenerate")
        prev, cur = cur, nxt
        if cur == start:
            break
        if n > len(cells):
            fail("walk too long")
    if n != len(cells):
        fail(f"multiple loops ({n} of {len(cells)} cells on first loop)")

    def ch(r, c):
        return out[r][c]

    def straight(r, c):
        return ch(r, c) in '-|'

    for r in range(R):
        for c in range(C):
            p = g[r][c]
            if p == '.':
                continue
            x = ch(r, c)
            if x == '.':
                fail(f"pearl at ({r},{c}) not on loop")
            if p == 'W':
                if not straight(r, c):
                    fail(f"white ({r},{c}) not straight")
                nb = [(r + D[d][0], c + D[d][1]) for d in CONN[x]]
                if all(straight(a, b) for a, b in nb):
                    fail(f"white ({r},{c}) no turn beside")
            else:
                if straight(r, c):
                    fail(f"black ({r},{c}) not turning")
                for d in CONN[x]:
                    a, b = r + D[d][0], c + D[d][1]
                    if not straight(a, b):
                        fail(f"black ({r},{c}) leg {d} not straight")
    print(f"length {len(cells)}")
    print(f"SCORE {len(cells)} {DIRECTION}")


if __name__ == "__main__":
    main()
