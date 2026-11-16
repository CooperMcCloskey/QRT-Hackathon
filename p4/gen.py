#!/usr/bin/env python3
"""Generate Masyu tests: random simple loop (grown by face flips), pearls on eligible cells."""
import os
import random
import sys


def make(R, C, steps, dens, seed, bw=0.5):
    rnd = random.Random(seed)
    # edges: h[r][c] between (r,c)-(r,c+1); v[r][c] between (r,c)-(r+1,c)
    h = [[0] * (C - 1) for _ in range(R)]
    v = [[0] * C for _ in range(R - 1)]
    deg = [[0] * C for _ in range(R)]
    r0, c0 = rnd.randrange(R - 1), rnd.randrange(C - 1)

    def edges(i, j):  # face (i,j): top, right, bottom, left as (arr, r, c, endpoints)
        return [(h, i, j, (i, j), (i, j + 1)), (v, i, j + 1, (i, j + 1), (i + 1, j + 1)),
                (h, i + 1, j, (i + 1, j + 1), (i + 1, j)), (v, i, j, (i + 1, j), (i, j))]

    def flip(i, j, force=False):
        es = edges(i, j)
        used = [e[0][e[1]][e[2]] for e in es]
        k = sum(used)
        if not force:
            if k == 0 or k == 4:
                return False
            if k == 2 and used[0] == used[2]:
                return False
            corners = [(i, j), (i, j + 1), (i + 1, j + 1), (i + 1, j)]
            # corner q is between edge q-1 and q (edge index: corner0 = left(3)&top(0))
            for q in range(4):
                a, b = used[(q - 1) % 4], used[q]
                if a == 0 and b == 0 and deg[corners[q][0]][corners[q][1]] != 0:
                    return False
        for e in es:
            arr, r, c, p1, p2 = e
            d = 1 - 2 * arr[r][c]
            arr[r][c] ^= 1
            deg[p1[0]][p1[1]] += d
            deg[p2[0]][p2[1]] += d
        return True

    flip(r0, c0, True)
    done = 0
    tries = 0
    while done < steps and tries < steps * 50:
        tries += 1
        i, j = rnd.randrange(R - 1), rnd.randrange(C - 1)
        if flip(i, j):
            done += 1

    def on(r, c, d):
        if d == 'E':
            return c + 1 < C and h[r][c]
        if d == 'W':
            return c > 0 and h[r][c - 1]
        if d == 'S':
            return r + 1 < R and v[r][c]
        return r > 0 and v[r - 1][c]

    D = {'N': (-1, 0), 'S': (1, 0), 'E': (0, 1), 'W': (0, -1)}

    def dirs(r, c):
        return [d for d in 'NESW' if on(r, c, d)]

    def straight(r, c):
        ds = dirs(r, c)
        return len(ds) == 2 and (set(ds) == {'E', 'W'} or set(ds) == {'N', 'S'})

    g = [['.'] * C for _ in range(R)]
    L = 0
    for r in range(R):
        for c in range(C):
            ds = dirs(r, c)
            if not ds:
                continue
            L += 1
            nb = [(r + D[d][0], c + D[d][1]) for d in ds]
            if straight(r, c):
                if any(not straight(a, b) for a, b in nb) and rnd.random() < dens * (1 - bw) * 2:
                    g[r][c] = 'W'
            else:
                if all(straight(a, b) for a, b in nb) and rnd.random() < dens * bw * 2:
                    g[r][c] = 'B'
    np = sum(row.count('.') != C for row in g)
    if sum(ch != '.' for row in g for ch in row) < 2:
        return make(R, C, steps, dens, seed + 1000, bw)
    return g, L


def main():
    os.makedirs("tests", exist_ok=True)
    cases = []
    seed = 1
    for (R, C) in [(6, 6), (8, 10), (12, 12), (20, 15), (25, 25), (30, 40), (40, 40), (50, 50)]:
        for dens in [1.0, 0.5, 0.2, 0.05]:
            cases.append((R, C, R * C, dens, seed))
            seed += 1
    for idx, (R, C, steps, dens, s) in enumerate(cases):
        g, L = make(R, C, steps, dens, s)
        name = f"tests/{idx + 1:02d}_{R}x{C}_d{int(dens * 100)}.in"
        with open(name, "w") as f:
            f.write(f"{R}\n" + "\n".join("".join(row) for row in g) + "\n")
        print(name, "genloop", L)


if __name__ == "__main__" and len(sys.argv) == 1:
    main()


def extra():
    """Robustness cases: thin grids, very sparse, grid-filling dense loops."""
    os.makedirs("tests", exist_ok=True)
    specs = [("40_6x50_d50", 6, 50, 600, 0.5, 0.5), ("41_50x6_d100", 50, 6, 600, 1.0, 0.5),
             ("42_50x50_fill_d70", 50, 50, 20000, 0.7, 0.5), ("43_50x50_fill_d100", 50, 50, 20000, 1.0, 0.5),
             ("44_45x50_fill_d30_black", 45, 50, 20000, 0.3, 0.8), ("45_35x35_fill_d50_white", 35, 35, 8000, 0.5, 0.2)]
    for name, R, C, steps, dens, bw in specs:
        g, L = make(R, C, steps, dens, hash(name) % 1000, bw)
        with open(f"tests/{name}.in", "w") as f:
            f.write(f"{R}\n" + "\n".join("".join(row) for row in g) + "\n")
        print(name, "genloop", L)
    # very sparse: a few pearls far apart
    rnd = random.Random(7)
    for name, R, C, k in [("46_50x50_sparse2", 50, 50, 2), ("47_50x50_sparse5", 50, 50, 5)]:
        while True:
            g = [['.'] * C for _ in range(R)]
            for _ in range(k):
                g[rnd.randrange(2, R - 2)][rnd.randrange(2, C - 2)] = rnd.choice('WB')
            break
        with open(f"tests/{name}.in", "w") as f:
            f.write(f"{R}\n" + "\n".join("".join(row) for row in g) + "\n")
        print(name)


if __name__ == "__main__" and len(sys.argv) > 1 and sys.argv[1] == "extra":
    extra()
