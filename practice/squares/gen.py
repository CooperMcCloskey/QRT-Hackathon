#!/usr/bin/env python3
"""Write the local tests for square packing into tests/."""
import os
import random

random.seed(1)
os.makedirs("tests", exist_ok=True)

cases = {
    "00_sample": [2, 1, 1],
    "01_single": [7],
    "02_equal": [5] * 5,
    "03_small": [random.randint(1, 10) for _ in range(10)],
    "04_medium": [random.randint(1, 50) for _ in range(50)],
    "05_max_random": [random.randint(1, 1000) for _ in range(128)],
    "06_max_ladder": list(range(1, 129)),
}
for name, sides in cases.items():
    with open(f"tests/{name}.in", "w") as f:
        f.write(f"{len(sides)}\n" + "\n".join(map(str, sides)) + "\n")
print(f"wrote {len(cases)} tests")
