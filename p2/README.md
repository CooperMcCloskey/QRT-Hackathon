Paste the entire contents of **submission.py**, including imports and helpers,
into the judge. It defines `generate_grid(required_words, forbidden_words)` and
returns a list of row strings. It is self-contained, uses only the standard
library, reads no files, and contains no local stdin adapter or test answers.
Its solver definitions are identical to the tested `sol_v7.py`.

The statement does not specify input bounds or a time limit. `SEARCH_SECONDS`
defaults to 1.5 seconds, using 75% of a provisional 2-second local limit.
Preprocessing and construction are included in that budget, although Python
operations between clock checks can overrun it. Uppercase input and feasible
instances are assumed. Impossible inputs and unsuccessful construction raise
`ValueError`; the statement supplies no impossible-case return convention.

The solver:

- Removes duplicate, reversed, and contained requirements, while retaining the
  original total length for score targeting. Required words containing a
  forbidden word in either direction are rejected immediately.
- Uses the longest word and per-letter multiplicities as lower bounds. If a
  required word fills a whole toroidal cycle, its forced forbidden occurrences
  can strengthen the bound. Validated periodic constructions handle some small
  alphabets directly.
- Builds a fallback by packing rows. Without forbidden words, all rows can be
  used. With forbidden words, separator cells and rows simplify construction;
  free-cell repair checks complete toroidal lines before accepting an answer.
- Searches smaller grids using bit masks of legal starting positions in four
  families of cyclic lines, considering each word and its reverse. It prefers
  overlap and checks new forbidden occurrences after every placement.
- Repairs nearly complete placements by removing blocking words and reinserting
  them. It keeps the best valid grid and returns it when the budget ends.

Every compass direction has cycle length N. A forbidden word longer than N can
occur only if it is N-periodic; in that case its first N letters fully determine
the constraint. This is checked explicitly, so multiple wraps are covered.

This is an optimisation heuristic, not a proof of global minimality or a
guaranteed constructor for all feasible inputs of unspecified size. All scores
below are local checker measurements, not judge results. The 1,000-word cases
are chosen stress sizes, not claimed judge maxima.

| Local case | sol_v3 | sol_v7 |
|---|---:|---:|
| Sample | 58.8235 | 58.8235 |
| Programming words | 72.9412 | 72.9412 |
| Medium, 60 words | 70.4412 | 78.0512 |
| Dense, 100 words | 71.9761 | 71.9761 |
| Stress, 250 words | 54.8092 | 54.8092 |
| 1,000 words + 1,000 forbiddens | Failed | 32.9910 |
| 1,000 words, no forbiddens | 33.0737 | 71.9314 |

Version 7 passed all 18 standard, 5 edge, and 8 audit cases under the provisional
2-second limit; the slowest measured run was 1.54 seconds. It had no score
regressions against version 3 in those recorded runs. Timing-based stopping can
produce different scores on slower or busier machines despite the fixed seed.

Reproduce scoring from this folder:

```sh
python3 ../tools/run_tests.py --tl 2 sol_v7.py
python3 ../tools/run_tests.py --tl 2 --tests 'edge_tests/*.in' sol_v7.py
python3 ../tools/run_tests.py --tl 2 --tests 'audit_tests/*.in' sol_v7.py
```

`check.py` is independent of the solver. `regression.py` compares it and forbidden
matching with a literal eight-direction oracle; `audit_masks.py` checks placement
mask invariants. `gen.py` and `gen_audit.py` create synthetic tests. Witness grids
are only test artifacts and are never read or embedded by the submission.
`scores.jsonl` contains the measured run history and must not be edited.

The export passed a Python 3.8 grammar check. Its sample and both 1,000-word
cases were also run through `run_submission.py`; the large cases passed with
the Python 3.8 popcount fallback forced on. This is a compatibility check, not
a measurement on an actual Python 3.8 interpreter.

The final correctness audit passed 400 independent checker comparisons, 4,064
exhaustive periodic-word cases, and 4,800 placement attempts. Every grid returned
in 160 periodic-construction trials and three forced-large completion trials
passed an independent literal eight-direction check.

Use `run_submission.py` only for local JSON-input testing of the exported file.
Submit **submission.py / sol_v7**, not the earlier experimental versions.
