#!/usr/bin/env python3
"""Checker for the square packing practice problem.

Usage:  python3 check.py <input file> <output file>
Prints: 'SCORE <area> min' or 'INVALID: <reason>' (exit code 1).
"""
import sys

DIRECTION = "min"


def fail(reason):
    print(f"INVALID: {reason}")
    sys.exit(1)


def main():
    inp = open(sys.argv[1]).read().split()
    out = open(sys.argv[2]).read().split()

    n = int(inp[0])
    sides = [int(x) for x in inp[1:1 + n]]

    if len(out) != 2 * n:
        fail(f"expected {2 * n} numbers, got {len(out)}")
    try:
        nums = [int(x) for x in out]
    except ValueError:
        fail("output contains a non-integer")
    xs, ys = nums[0::2], nums[1::2]

    for i in range(n):
        for j in range(i + 1, n):
            apart = (xs[i] + sides[i] <= xs[j] or xs[j] + sides[j] <= xs[i] or
                     ys[i] + sides[i] <= ys[j] or ys[j] + sides[j] <= ys[i])
            if not apart:
                fail(f"squares {i} and {j} overlap")

    width = max(xs[i] + sides[i] for i in range(n)) - min(xs)
    height = max(ys[i] + sides[i] for i in range(n)) - min(ys)
    print(f"SCORE {width * height} {DIRECTION}")


if __name__ == "__main__":
    main()
