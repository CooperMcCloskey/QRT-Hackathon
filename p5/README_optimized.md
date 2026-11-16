# Question 5: recommended submission

**Submit `sol_optimized.cpp`.** It is a standalone C++ program, compatible with C++14 and C++17, with no runtime data files. `sol_v3.cpp` is preserved.

The selected file is 48,684 bytes. `sol_optimized_readable.cpp` is the corresponding readable source before source-size compaction and removal of an unused helper; submit the compact file.

## Measured result

All 19 document cases passed the independent checker and the two-second local limit.

| Test group | Cases | Original sol_v3 | New sol_optimized |
|---|---:|---:|---:|
| Original repository tests | 7 | 11.851% | 16.037% |
| Fresh standard/varied cuts | 8 | 13.681% | 17.439% |
| Final held-out cases | 4 | 13.796% | 15.883% |
| **All document cases** | **19** | **13.031%** | **16.595%** |

These are raw reconstruction percentages from `check.py`, not the runner's normalized `mean rel`. The improvement is about 27.3% relative, or 3.56 percentage points. Sixteen cases improved and three regressed: `holdout_varied7` (12.827 → 12.497), `normal4` (17.636 → 17.000), and `varied6` (22.010 → 21.335). Official hidden-test performance is unknown.

Maximum observed runtime was approximately 1.46 seconds for the new solver versus 1.60 seconds for the baseline. Three additional edge cases passed: one whole page, duplicate whole pages, and 4,522 individual-character pieces. The sample passed output-format and non-overlap validation; its ground-truth placement was not supplied, so no sample reconstruction score is claimed.

AddressSanitizer and UndefinedBehaviorSanitizer passed the sample, an ordinary document case, a varied-size-piece case, and a whole-page case. Leak detection was disabled because this environment runs under tracing. C++14 compilation also passed.

## What changed

- Replaces dictionary-derived character trigrams with a compact model of one- to five-character conditional probabilities, selecting patterns by their information gain over shorter contexts. Scores the actual characters across each proposed seam.
- Retains a 4,000-word dictionary with prefix/suffix and inflection checks, balancing language quality against the submission-size budget.
- Uses stable logarithmic confidence accumulation, avoiding saturation and subtraction errors for strong fragment matches.
- Keeps singleton fragments as separate candidate pages instead of discarding them. Additional disjoint candidates cannot lower the optimal one-to-one page matching score.
- Removes the obsolete 4,096-piece truncation.
- Stops unchanged merge passes after convergence and scans only live candidates.
- Checks the clock during candidate scans and keeps a valid partial assembly when the 1.45-second internal budget expires.
- Avoids treating entirely blank rows of a multirow piece as conclusive right-edge evidence.
- Adds a singleton-placement fallback if the assembly function throws.

The solver still assembles fragments rather than reliably reconstructing entire pages. Its measured improvement should not be mistaken for near-complete reconstruction.

## Limits and provenance

The contest statement omits its time, memory, and source-size limits; the user could not confirm them. HackerRank's [published execution environment](https://support.hackerrank.com/articles/6693750503-execution-environment) lists a 50 KB source limit and standard C++ limits of two seconds and 512 MB. The submission stays below 50,000 bytes and uses a 1.45-second internal deadline; contest-specific overrides remain unknown.

The additional language model was trained exclusively on public [WikiText-2 TRAIN](https://raw.githubusercontent.com/pytorch/examples/main/word_language_model/data/wikitext-2/train.txt), separate from the [validation corpus](https://raw.githubusercontent.com/pytorch/examples/main/word_language_model/data/wikitext-2/valid.txt) used to generate tests. The embedded constants are generic English language statistics and vocabulary, not puzzle solutions. `train_selected_review.py`, `build_ngram_review.py`, and the intermediate model files retain the training/integration work. Fresh and held-out tests were generated with distinct seeds; the held-out cases were evaluated after selecting the model and weight.

`check.py` now uses exact one-to-one maximum-weight page assignment instead of limiting each real page to its five best candidates. Baseline scores were unchanged by this checker fix. Its translation search remains the inherited local model, and the official scorer implementation is unavailable.

## Reproduce

From `p5`:

```sh
python3 ../tools/run_tests.py --tl 2 --tests 'validation_review/*.in' sol_v3.cpp sol_optimized.cpp
python3 ../tools/run_tests.py --tl 2 --tests 'edge_review/*.in' sol_optimized.cpp
g++ -O2 -std=c++14 sol_optimized.cpp -o /tmp/p5_solution
```

Full results: `validation_optimized.txt`, `validation_edges_optimized.txt`, `benchmark_optimized.csv`, and `sanitizer_optimized.txt`. Runner measurements are appended to `scores.jsonl`.
