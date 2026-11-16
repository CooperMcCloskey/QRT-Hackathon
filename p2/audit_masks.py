"""Randomized semantic audit of bitmask packing; defaults to sol_v3.py."""
import random
import sys

from regression import literal_present, load


def main():
    solver = load("packing_under_test", sys.argv[1] if len(sys.argv) > 1 else "sol_v3.py")
    rng = random.Random(424242)
    accepted = 0
    for n in range(1, 9):
        for trial in range(30):
            forbidden = ["AA", "BCB"] if trial % 2 else []
            packing = solver._Packing(n, forbidden, random.Random(trial), float("inf"))
            required = []
            for _ in range(20):
                word = "".join(rng.choice("ABCD") for _ in range(rng.randint(1, n)))
                if packing.place(word):
                    required.append(word)
                    accepted += 1
                rows = ["".join(ch or "." for ch in packing.grid[r * n:(r + 1) * n])
                        for r in range(n)]
                assert all(literal_present(rows, w) for w in required), (n, trial, required, rows)
                assert not any(literal_present(rows, w) for w in forbidden), (n, trial, rows)
                for line, path in enumerate(packing.paths):
                    for k, cell in enumerate(path):
                        ch = packing.grid[cell]
                        assert bool(packing.free[line] & (1 << k)) == (not ch)
                        for letter in "ABCD":
                            assert bool(packing.letters[line].get(letter, 0) & (1 << k)) == (ch == letter)
    print("PASS: 4,800 randomized placement attempts; {} accepted; required occurrences, forbidden rejection, and masks verified".format(accepted))


if __name__ == "__main__":
    main()
