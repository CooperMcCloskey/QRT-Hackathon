#!/usr/bin/env python3
"""Independent statement checker: check.py local-input.json output.txt."""
import json
import sys

DIRECTION = "max"


def fail(reason):
    print("INVALID: " + reason)
    sys.exit(1)


def main():
    if len(sys.argv) != 3:
        print(__doc__)
        sys.exit(2)
    with open(sys.argv[1]) as handle:
        data = json.load(handle)
    with open(sys.argv[2]) as handle:
        raw = handle.read()
    # Accept the function's JSON return value or the local CLI's plain rows.
    try:
        rows = json.loads(raw) if raw.lstrip().startswith("[") else raw.splitlines()
    except ValueError:
        fail("malformed JSON output")
    if not isinstance(rows, list) or not rows:
        fail("grid must be a nonempty list of rows")
    n = len(rows)
    if any(not isinstance(row, str) or len(row) != n for row in rows):
        fail("grid is not square")
    if any(not ("A" <= ch <= "Z") for row in rows for ch in row):
        fail("cells must be uppercase A-Z")
    required = data["required_words"]
    forbidden = data["forbidden_words"]
    if n < max([len(word) for word in required] or [0]):
        fail("N is smaller than the longest required word")
    # Enumerate each complete toroidal cycle, independently of solver placements.
    cycles = []
    for row in rows:
        cycles.append(row)
    for c in range(n):
        cycles.append("".join(rows[r][c] for r in range(n)))
    for start in range(n):
        cycles.append("".join(rows[r][(start + r) % n] for r in range(n)))
        cycles.append("".join(rows[r][(start - r) % n] for r in range(n)))

    def present(word):
        if not word:
            return True
        for cycle in cycles:
            extent = n + len(word) - 1
            repeated = (cycle * ((extent + n - 1) // n))[:extent]
            if word in repeated or word[::-1] in repeated:
                return True
        return False

    for word in required:
        if not present(word):
            fail("missing required word " + repr(word))
    for word in forbidden:
        if present(word):
            fail("forbidden word present " + repr(word))
    value = 100.0 * min(1.0, sum(map(len, required)) / (1.7 * n * n))
    print("SCORE {} {}".format(value, DIRECTION))


if __name__ == "__main__":
    main()
