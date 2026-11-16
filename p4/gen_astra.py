#!/usr/bin/env python3
"""Independent, reproducible valid-loop witnesses and held-out Masyu cases."""
from pathlib import Path
import random

ROOT = Path(__file__).resolve().parent / 'tests_astra'
ROOT.mkdir(exist_ok=True)


def generate(rows, cols, seed):
    rng = random.Random(seed)
    # Start from the board perimeter; replace single edges with three-edge ears.
    path = [(0, c) for c in range(cols)]
    path += [(r, cols - 1) for r in range(1, rows)]
    path += [(rows - 1, c) for c in range(cols - 2, -1, -1)]
    path += [(r, 0) for r in range(rows - 2, 0, -1)]
    occupied = set(path)
    for unused in range(rows * cols * 100):
        i = rng.randrange(len(path))
        u, v = path[i], path[(i + 1) % len(path)]
        dr, dc = v[0] - u[0], v[1] - u[1]
        sign = rng.choice((-1, 1))
        a = (u[0] + sign * dc, u[1] - sign * dr)
        b = (v[0] + sign * dc, v[1] - sign * dr)
        if (0 <= a[0] < rows and 0 <= a[1] < cols and
                0 <= b[0] < rows and 0 <= b[1] < cols and
                a not in occupied and b not in occupied):
            path[i + 1:i + 1] = [a, b]
            occupied.update((a, b))
    directions = {(-1, 0): 1, (0, 1): 2, (1, 0): 4, (0, -1): 8}
    shapes = {3: 'L', 9: 'J', 6: 'F', 12: '7', 5: '|', 10: '-'}
    result = [['.'] * cols for _ in range(rows)]
    straight = []
    for i, (r, c) in enumerate(path):
        mask = 0
        for q in (path[i - 1], path[(i + 1) % len(path)]):
            mask |= directions[(q[0] - r, q[1] - c)]
        result[r][c] = shapes[mask]
        straight.append(mask in (5, 10))
    eligible = []
    for i, p in enumerate(path):
        left, right = straight[i - 1], straight[(i + 1) % len(path)]
        if straight[i] and not (left and right):
            eligible.append((p, 'W'))
        elif not straight[i] and left and right:
            eligible.append((p, 'B'))
    for density in (1.0, 0.25, 0.03):
        selected = [(p, ch) for p, ch in eligible if rng.random() < density]
        if len(selected) < 2:
            selected = eligible[:2]
        grid = [['.'] * cols for _ in range(rows)]
        for (r, c), ch in selected:
            grid[r][c] = ch
        name = '{}x{}_s{}_d{}_astra'.format(rows, cols, seed, int(density * 100))
        (ROOT / (name + '.in')).write_text(str(rows) + '\n' + '\n'.join(map(''.join, grid)) + '\n')
        (ROOT / (name + '.out')).write_text('\n'.join(map(''.join, result)) + '\n')
        print(name, 'pearls', len(selected), 'witness_length', len(path))


if __name__ == '__main__':
    for rows, cols, seed in ((6, 50, 101), (50, 6, 103), (20, 20, 107),
                             (35, 48, 109), (50, 50, 113)):
        generate(rows, cols, seed)
