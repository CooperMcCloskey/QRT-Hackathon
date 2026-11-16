"""Independent small-grid oracle for checker and forbidden-word reduction.

Run with Python 3.8+. This does not score solvers; use run_tests.py for scores.
"""
import contextlib
import importlib.util
import io
import itertools
import json
import random
import sys
import tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parent


def load(name, filename):
    spec = importlib.util.spec_from_file_location(name, str(ROOT / filename))
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


def literal_present(grid, word):
    n = len(grid)
    for r in range(n):
        for c in range(n):
            for dr in (-1, 0, 1):
                for dc in (-1, 0, 1):
                    if dr == dc == 0:
                        continue
                    if all(grid[(r + k * dr) % n][(c + k * dc) % n] == ch
                           for k, ch in enumerate(word)):
                        return True
    return False


def literal_valid(grid, required, forbidden):
    n = len(grid)
    return (bool(n) and all(len(row) == n for row in grid)
            and all("A" <= ch <= "Z" for row in grid for ch in row)
            and n >= max(map(len, required), default=0)
            and all(literal_present(grid, w) for w in required)
            and not any(literal_present(grid, w) for w in forbidden))


def main():
    checker = load("independent_checker", "check.py")
    solver = load("solver_helpers", sys.argv[1] if len(sys.argv) > 1 else "sol_v1.py")
    rng = random.Random(314159)
    checked = 0
    with tempfile.TemporaryDirectory(prefix="oracle_", dir=str(ROOT)) as temp:
        inp, out = Path(temp) / "case.in", Path(temp) / "case.out"
        old_argv = sys.argv
        try:
            for n in range(1, 6):
                for _ in range(80):
                    grid = ["".join(rng.choice("ABC") for _ in range(n))
                            for _ in range(n)]
                    words = ["".join(rng.choice("ABC") for _ in range(rng.randint(1, 3 * n + 2)))
                             for _ in range(8)]
                    required, forbidden = words[:2], words[2:]
                    if rng.randrange(2):
                        required = []
                    expected = literal_valid(grid, required, forbidden)
                    inp.write_text(json.dumps({"required_words": required,
                                               "forbidden_words": forbidden}))
                    out.write_text("\n".join(grid) + "\n")
                    sys.argv = ["check.py", str(inp), str(out)]
                    capture = io.StringIO()
                    actual = True
                    with contextlib.redirect_stdout(capture):
                        try:
                            checker.main()
                        except SystemExit as error:
                            actual = error.code == 0
                    assert actual == expected, (n, grid, words, capture.getvalue())
                    pattern, extent = solver._pattern(n, forbidden)
                    flat = list("".join(grid))
                    got = False
                    if pattern is not None:
                        for path in solver._lines(n):
                            line = "".join(flat[k] for k in path)
                            got |= pattern.search(line + line[:extent - 1]) is not None
                    want = any(literal_present(grid, w) for w in forbidden)
                    assert got == want, (n, grid, forbidden)
                    checked += 1
        finally:
            sys.argv = old_argv
    # Exhaustive 2x2 binary grids and words, including more than two wraps.
    checked_words = 0
    for chars in itertools.product("AB", repeat=4):
        grid = ["".join(chars[:2]), "".join(chars[2:])]
        for length in range(1, 8):
            for letters in itertools.product("AB", repeat=length):
                word = "".join(letters)
                pattern, extent = solver._pattern(2, [word])
                flat = list("".join(grid))
                actual = False
                if pattern is not None:
                    for path in solver._lines(2):
                        line = "".join(flat[k] for k in path)
                        actual |= pattern.search(line + line[:extent - 1]) is not None
                assert actual == literal_present(grid, word), (grid, word)
                checked_words += 1
    print("PASS: {} independent checker cases; {} exhaustive periodic-word cases".format(
        checked, checked_words))


if __name__ == "__main__":
    main()
