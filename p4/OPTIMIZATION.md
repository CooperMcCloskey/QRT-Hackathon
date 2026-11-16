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

## Validation (default seed)

`validation_optimized.txt`: 79/79 valid and in time for both versions.

| Measure | finSolve.cpp | finSolve_optimized.cpp |
|---|---:|---:|
| Runner mean relative length score | 98.6 | 99.1 |
| Squared-quality/speed local proxy | 96.637 | 98.242 |
| Maximum observed runtime | 5.277 s | 1.221 s |
| Original 41 cases, local proxy | 96.621 | 98.067 |
| Independent 15 cases, local proxy | 96.363 | 99.047 |
| Transformed 15 cases, local proxy | 96.455 | 96.977 |
| Fresh 8 cases, local proxy | 97.575 | 100.000 |

The new solver produced shorter/equal/longer loops on 27/32/20 cases respectively.
All four groups improved in aggregate in this run, but individual cases regressed.

The prebuilt compact-initial-rectangle variant (`finSolve_optimized_v7.cpp`) was also valid on 79/79 cases, but did not improve the overall proxy over the selected version. It is not the recommended submission.

`sanitizer_optimized.txt`: AddressSanitizer and UndefinedBehaviorSanitizer passed the sample, a dense 12x12 board, a sparse 50x50 board, and a narrow 50x6 board. Their outputs also passed `check.py`. LeakSanitizer was disabled because it is unsupported under this environment's tracing.

The selected file is byte-identical to `finSolve_optimized_v5_hardened.cpp`. Its SHA-256 is `9f4a5eb467eaa9209df57c0b2fd65f515512eb59f1e74f763215556d9a8006da`.
The unchanged original SHA-256 is `5cbf63f34e6619df2930f591ed8ded2fb7bb3b6b78f8fed9fd2840940dd4d5c9`.

## Repeat with SEED=1

`validation_optimized_seed1.txt`: again 79/79 valid and in time for both versions.

| Measure | finSolve.cpp | finSolve_optimized.cpp |
|---|---:|---:|
| Runner mean relative length score | 99.2 | 99.2 |
| Squared-quality/speed local proxy | 97.663 | 98.388 |
| Maximum observed runtime | 4.353 s | 1.315 s |
| Original 41 cases, local proxy | 98.749 | 97.693 |
| Independent 15 cases, local proxy | 97.938 | 99.150 |
| Transformed 15 cases, local proxy | 94.045 | 98.765 |
| Fresh 8 cases, local proxy | 98.364 | 99.813 |

Shorter/equal/longer loops: 19/39/21. The original test group regressed in this repeat; the other three groups improved. The overall proxy improved in both seeds, by 1.605 and 0.725 points. Averaging the two runs' pairwise-reference proxies gives 97.150 versus 98.315; these remain local estimates, not official judge scores. Across both comparisons, the selected program passed 158/158 outputs, with maximum observed runtime 1.315 seconds versus 5.277 seconds for the original.

`benchmark_optimized.csv` contains the repeat's per-case checker lengths, runtimes and proxy calculations. The two runner text files retain both comparisons.

**Recommendation: SUBMIT `finSolve_optimized.cpp`.** This is a measured overall improvement with individual-case regressions, not a claim of optimality or guaranteed hidden-test superiority.
