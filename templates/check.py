#!/usr/bin/env python3
"""Checker skeleton. Copy into the problem folder and fill in the TODOs.

Usage:  python3 check.py <input file> <output file>
Prints: 'SCORE <value> <min|max>' on success,
        'INVALID: <reason>' and exit code 1 on any rule violation.

Write this from the statement alone. Do not import or copy solver code.
"""
import sys

DIRECTION = "min"  # TODO: "min" if a lower value is better, "max" if higher


def fail(reason):
    print(f"INVALID: {reason}")
    sys.exit(1)


def main():
    if len(sys.argv) != 3:
        print(__doc__)
        sys.exit(2)
    inp = open(sys.argv[1]).read().split()
    out = open(sys.argv[2]).read().split()

    # TODO 1: parse the input.
    # TODO 2: parse the output; fail() on wrong token count or non-numbers.
    # TODO 3: check every validity rule from notes.md; fail() on the first break.
    # TODO 4: compute the value the statement scores.
    value = 0

    print(f"SCORE {value} {DIRECTION}")


if __name__ == "__main__":
    main()
