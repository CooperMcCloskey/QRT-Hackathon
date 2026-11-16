#!/usr/bin/env python3
"""Timed simulated annealing skeleton (Python).

Runs as-is on a toy problem: order numbers to minimise the sum of gaps between
neighbours. Replace the parts marked REPLACE. Keep the time guard and the
"best answer so far" logic. Avoid syntax newer than Python 3.8: the judge may
be older than this laptop.
"""
import math
import random
import sys
import time

T0 = time.perf_counter()
TIME_LIMIT = 2.0                  # REPLACE: the judge's limit in seconds
TIME_BUDGET = 0.75 * TIME_LIMIT   # stop here; the judge may be slower
random.seed(12345)                # fixed seed: results repeat


def read_input():                 # REPLACE
    data = sys.stdin.read().split()
    n = int(data[0])
    return [int(x) for x in data[1:1 + n]]


def initial_state(a):             # REPLACE: any valid answer (the baseline)
    return list(range(len(a)))


def cost(a, s):                   # REPLACE: lower is better
    return sum(abs(a[s[i]] - a[s[i + 1]]) for i in range(len(s) - 1))


def write_output(a, s):           # REPLACE: exact format from the statement
    print(" ".join(str(a[i]) for i in s))


def main():
    a = read_input()
    cur = initial_state(a)
    n = len(cur)
    if n < 2:
        write_output(a, cur)
        return
    cur_cost = cost(a, cur)
    best, best_cost = cur[:], cur_cost

    t_start, t_end = max(1.0, cur_cost / n), 1e-3   # REPLACE: tune
    temp = t_start
    it = 0
    while True:
        if it % 64 == 0:
            frac = (time.perf_counter() - T0) / TIME_BUDGET
            if frac >= 1.0:
                break
            temp = t_start * (t_end / t_start) ** frac
        it += 1

        # REPLACE: one small random change, and a way to undo it.
        i, j = random.randrange(n), random.randrange(n)
        if i == j:
            continue
        cur[i], cur[j] = cur[j], cur[i]
        new_cost = cost(a, cur)   # REPLACE with a delta if cost() is slow

        delta = new_cost - cur_cost
        if delta <= 0 or random.random() < math.exp(-delta / temp):
            cur_cost = new_cost
            if cur_cost < best_cost:
                best, best_cost = cur[:], cur_cost
        else:
            cur[i], cur[j] = cur[j], cur[i]   # undo

    write_output(a, best)
    print(f"iters={it} best={best_cost}", file=sys.stderr)


if __name__ == "__main__":
    main()
