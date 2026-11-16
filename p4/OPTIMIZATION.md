# Question 4 optimisation

Submission file: `finSolve_optimized.cpp` (standalone C++17, standard library only).
The original `finSolve.cpp` is unchanged.

## Changes

- Combines the original repair/LNS pipeline with Astra's independently implemented SAT engine. Dense boards first receive a bounded 180 ms Astra attempt; difficult repair failures also receive an independent fallback.
- Restores every pearl constraint before a full SAT solve. Previously a repair timeout could leave a partially constrained problem behind.
- Checks SAT deadlines and conflict budgets on every search path, including subtour conflicts and decision-only progress. Feasibility and late improvement share a 7.45 second search cap.
- Avoids repeatedly tracing the same selected-edge component within one connectivity check.
- Uses direct cell masks for cheap legal face flips, interleaved with SAT window optimisation and bounded shortest-path shortcuts through pearl-free arcs.
- Stops on a rigorous bounding-box/pearl-count lower bound when attained.
- Validates candidates and the final improved loop; preserves the valid incumbent if improvement validation fails. No bounded feasibility search can guarantee a valid answer on every hidden instance.

## Selection

Read the repository's statement, prior mainline and Astra implementations, handoff, risk review, research report and research notes. Tested the component optimisations separately before combining them. The first rewritten pipeline, a SAT-first variant using only the original engine, and a more expensive two-start portfolio did not beat the baseline overall; they were not selected.

The research's independent engine was the useful complementary feasibility method. Timing-based stochastic search can change lengths between runs despite fixed random seeds. All reported values are local measurements; official reference lengths and hidden tests are unavailable.

## Reproduction

From `p4`:

```sh
python3 ../tools/run_tests.py --tl 10 --tests 'validation_optimized/*.in' finSolve.cpp finSolve_optimized.cpp
```

The validation set contains 41 original cases, 15 independent Astra cases, 15 rotation/reflection cases, and 8 fresh generated cases. The normal runner checks every output using `check.py` and appends measurements to `scores.jsonl`.

The runner's `mean rel` is a linear length comparison, not the contest score. The supplementary proxy applies squared length quality and the statement's speed multiplier, taking the shortest valid compared answer as the local reference and assuming s=1. This is only an estimate for local comparison.
