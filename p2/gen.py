#!/usr/bin/env python3
"""Deterministic local cases; hidden judge limits are unknown."""
import json
import random
from pathlib import Path

ROOT = Path(__file__).resolve().parent / "tests"
ROOT.mkdir(exist_ok=True)
RNG = random.Random(20261116)
LETTERS = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"


def save(name, required, forbidden):
    (ROOT / (name + ".in")).write_text(json.dumps({
        "required_words": required, "forbidden_words": forbidden}) + "\n")


def occurs(grid, word):
    n = len(grid)
    for r in range(n):
        for c in range(n):
            for dr, dc in ((0, 1), (1, 0), (1, 1), (1, -1)):
                s = "".join(grid[(r + k * dr) % n][(c + k * dc) % n]
                            for k in range(len(word)))
                if s == word or s == word[::-1]:
                    return True
    return False


def planted(name, n, count, low, high, forbidden_count, alphabet=LETTERS):
    grid = ["".join(RNG.choice(alphabet) for _ in range(n)) for _ in range(n)]
    words = []
    for _ in range(count):
        r, c = RNG.randrange(n), RNG.randrange(n)
        dr, dc = RNG.choice(((0, 1), (1, 0), (1, 1), (1, -1)))
        length = RNG.randint(low, min(high, n))
        words.append("".join(grid[(r + k * dr) % n][(c + k * dc) % n]
                             for k in range(length)))
    forbidden = []
    while len(forbidden) < forbidden_count:
        w = "".join(RNG.choice(LETTERS) for _ in range(RNG.randint(3, 8)))
        if not occurs(grid, w):
            forbidden.append(w)
    save(name, words, forbidden)


save("00_sample", ["ACE", "FIG", "OAK"], [])
save("01_single", ["A"], [])
save("02_empty", [], ["A", "BB"])
save("03_duplicates", ["ABRACADABRA", "ABRA", "ARBADACARBA", "ABRA"] * 3, ["XYZ"])
save("04_all_equal", ["A" * i for i in range(1, 17)], ["B", "CA", "DE"])
save("05_reverse_wrap", ["ABCDEF", "FEDCBA", "DEFABC", "EFAB", "CDE"], ["AAAA", "Q"])
save("06_forbidden_wrap", ["ABC"], ["CA", "CCCC", "BCABCABCABC"])
save("07_short_forbidden", ["ACE", "FACE", "CAFE", "FEED", "DEAF", "BEEF"], list("GHIJKLMNOPQRSTUVWXYZ") + ["AAA", "BBB"])
save("08_alphabet_pairs", ["ABC", "DEF", "GHI", "JKL", "MNO", "PQR", "STU", "VWX", "YZA"], [ch * 2 for ch in LETTERS])
save("09_english", "PYTHON JAVA RUBY PERL RUST SWIFT SCALA LUA CODE PROGRAM ALGORITHM FUNCTION VARIABLE ARRAY MATRIX GRAPH SEARCH SORT TREE STACK QUEUE HEAP HASH STRING INTEGER FLOAT BOOLEAN CLASS OBJECT METHOD RETURN BREAK CONTINUE IMPORT MODULE PACKAGE EXCEPTION LAMBDA ITERATOR GENERATOR RECURSION DYNAMIC MEMORY POINTER COMPILER INTERPRETER".split(), ["BUG", "FAIL", "ERROR", "CRASH", "HANG"])
save("10_long", [LETTERS + LETTERS[:14], "ZYXWVUTSRQPONMLKJIHGFEDCBA"], ["AAA", "ZZZ"])
planted("11_small", 7, 12, 3, 7, 12)
planted("12_medium", 16, 60, 4, 12, 80)
planted("13_dense", 20, 100, 5, 14, 150, "AAABCDEEEFGHIIJKLMNNOOOPRRSSSTTU")
planted("14_stress", 35, 250, 4, 18, 300)
planted("15_long_forbidden", 10, 20, 3, 9, 20)
save("16_periodic_forbidden", ["AB"], ["ABABABA", "BABA", "CDCDCDCDCD"])
save("17_single_letters", list(LETTERS), [])
