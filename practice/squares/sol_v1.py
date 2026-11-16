#!/usr/bin/env python3
"""Baseline: every square in one row. Always valid, far from optimal."""
import sys


def main():
    data = sys.stdin.read().split()
    n = int(data[0])
    sides = [int(x) for x in data[1:1 + n]]
    x = 0
    for s in sides:
        print(x, 0)
        x += s


if __name__ == "__main__":
    main()
