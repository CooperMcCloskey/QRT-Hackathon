"""Known-feasible stress tests for the solver audit; no judge bounds are assumed."""
import itertools
import json
import random
from pathlib import Path

ROOT = Path(__file__).resolve().parent / "audit_tests"
ROOT.mkdir(exist_ok=True)
RNG = random.Random(271828)
ALL = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"


def line_strings(grid, extent):
    n = len(grid)
    result = []
    for dr, dc in ((0, 1), (1, 0), (1, 1), (1, -1)):
        starts = [(r, 0) for r in range(n)] if dr == 0 else [(0, c) for c in range(n)]
        for r, c in starts:
            cycle = "".join(grid[(r + i * dr) % n][(c + i * dc) % n] for i in range(n))
            result.append((cycle * ((n + extent - 1 + n - 1) // n))[:n + extent - 1])
    return result


def extracted(grid, count, lower, upper):
    n = len(grid)
    result = []
    for _ in range(count):
        r, c = RNG.randrange(n), RNG.randrange(n)
        dr, dc = RNG.choice(((0, 1), (0, -1), (1, 0), (-1, 0),
                             (1, 1), (-1, -1), (1, -1), (-1, 1)))
        length = RNG.randint(lower, upper)
        result.append("".join(grid[(r + i * dr) % n][(c + i * dc) % n]
                              for i in range(length)))
    return result


def save(name, grid, required, forbidden):
    lines = line_strings(grid, max(map(len, required + forbidden), default=1))
    assert all(any(w in s or w[::-1] in s for s in lines) for w in required)
    assert not any(w in s or w[::-1] in s for w in forbidden for s in lines)
    (ROOT / (name + ".in")).write_text(json.dumps({"required_words": required,
                                                  "forbidden_words": forbidden}) + "\n")
    (ROOT / (name + ".witness")).write_text("\n".join(grid) + "\n")


for index, n in enumerate((4, 6, 8)):
    alphabet = list("ABCD")
    RNG.shuffle(alphabet)
    grid = ["".join(alphabet[2 * (r % 2) + c % 2] for c in range(n)) for r in range(n)]
    witnessed = line_strings(grid, 3)
    forbidden = list(ALL[4:]) + [ch * 2 for ch in "ABCD"]
    forbidden += ["".join(word) for word in itertools.product("ABCD", repeat=3)
                  if not any("".join(word) in s for s in witnessed)]
    save("0{}_four_colors".format(index), grid, extracted(grid, 60, 2, n), forbidden)

for index, n in enumerate((5, 10)):
    grid = ["".join("ABCDE"[(r + 2 * c) % 5] for c in range(n)) for r in range(n)]
    forbidden = list(ALL[5:]) + [ch * 2 for ch in "ABCDE"]
    save("0{}_five_colors".format(index + 3), grid, extracted(grid, 80, 3, n), forbidden)

n = 8
grid = [["ABCD"[2 * (r % 2) + c % 2] for c in range(n)] for r in range(n)]
for _ in range(32):
    r, c = RNG.randrange(n), RNG.randrange(n)
    ch = RNG.choice("EF")
    if all(grid[(r + dr) % n][(c + dc) % n] != ch
           for dr in (-1, 0, 1) for dc in (-1, 0, 1) if dr or dc):
        grid[r][c] = ch
grid = list(map("".join, grid))
save("05_six_colors", grid, extracted(grid, 70, 3, n),
     list(ALL[6:]) + [ch * 2 for ch in "ABCDEF"])

for name, n, count, forbidden_count in (("06_large_1000", 45, 1000, 1000),
                                         ("07_large_1000_no_forbidden", 45, 1000, 0)):
    grid = ["".join(RNG.choice(ALL) for _ in range(n)) for _ in range(n)]
    lines = line_strings(grid, 10)
    forbidden = []
    while len(forbidden) < forbidden_count:
        word = "".join(RNG.choice(ALL) for _ in range(RNG.randint(3, 10)))
        if not any(word in line or word[::-1] in line for line in lines):
            forbidden.append(word)
    save(name, grid, extracted(grid, count, 4, 20), forbidden)

print("Generated eight known-feasible audit cases and explicit witness grids.")
