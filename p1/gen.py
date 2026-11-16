#!/usr/bin/env python3
"""Write the local tests for Square packing++ into tests/."""
import os
import random

random.seed(1)
os.makedirs("tests", exist_ok=True)


def dissect(w, h, depth):
    """Cut a w x h rectangle into squares (a perfect packing exists)."""
    if w == h:
        return [w]
    if depth == 0 or min(w, h) <= 2:
        s = min(w, h)
        if w > h:
            return [s] + dissect(w - s, h, 0)
        return [s] + dissect(w, h - s, 0)
    if random.random() < 0.5 and w > 3:
        c = random.randint(1, w - 1)
        return dissect(c, h, depth - 1) + dissect(w - c, h, depth - 1)
    if h > 3:
        c = random.randint(1, h - 1)
        return dissect(w, c, depth - 1) + dissect(w, h - c, depth - 1)
    return dissect(w, h, 0)


cases = {
    "00_s1": [1],
    "00_s2": [1, 2],
    "00_s3": [1, 2, 3],
    "00_s4": [1, 2, 3, 4],
    "01_single": [7],
    "02_equal5": [5] * 5,
    "02_equal16": [3] * 16,
    "03_rand10": [random.randint(1, 10) for _ in range(10)],
    "04_rand50": [random.randint(1, 50) for _ in range(50)],
    "05_rand128": [random.randint(1, 1000) for _ in range(128)],
    "06_rand300": [random.randint(1, 100) for _ in range(300)],
    "07_rand1000": [random.randint(1, 60) for _ in range(1000)],
    "08_skew40": [int(1 + 200 * random.random() ** 3) for _ in range(40)],
    "09_bigsmall": [500] + [random.randint(1, 20) for _ in range(60)],
    "20_perfect_a": dissect(60, 45, 3),
    "20_perfect_b": dissect(233, 144, 4),
    "20_perfect_c": dissect(1000, 618, 5),
}
for k in (5, 6, 8, 10, 12, 15, 18, 20, 24, 27, 32, 40, 50, 75, 100):
    cases["10_cons%03d" % k] = list(range(1, k + 1))
sh = list(range(1, 31))
random.shuffle(sh)
cases["11_cons030_shuffled"] = sh

for name, sides in cases.items():
    with open("tests/%s.in" % name, "w") as f:
        f.write("%d\n%s\n" % (len(sides), " ".join(map(str, sides))))
print("wrote %d tests" % len(cases))
