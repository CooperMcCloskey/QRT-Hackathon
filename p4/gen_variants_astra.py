#!/usr/bin/env python3
"""Orientation checks of known-solvable inputs; does not modify source cases."""
from pathlib import Path
root=Path(__file__).resolve().parent
sources=['tests/00_sample.in','tests/21_30x40_d100.in','tests/26_40x40_d50.in',
         'tests/29_50x50_d100.in','tests_astra/50x50_s113_d100_astra.in']
out=root/'variants_astra'
out.mkdir(exist_ok=True)
for number,source in enumerate(sources):
    g=(root/source).read_text().split()[1:]
    variants=[[''.join(row) for row in zip(*g[::-1])], [row[::-1] for row in g], list(reversed(g))]
    for i,grid in enumerate(variants):
        (out/('case{}_transform{}_astra.in'.format(number,i))).write_text(str(len(grid))+'\n'+'\n'.join(grid)+'\n')
print('Wrote 15 orientation checks to variants_astra.')
