# Astra Masyu solver

**Submit `sol_astra.cpp` only.** It is a self-contained C++17 program with no external dependencies.

All code, tests, executables, outputs, and logs created for this implementation use `_astra` names. Shared solver files, test inputs, the shared checker, and `scores.jsonl` were not modified by this work. Earlier `sol_v*_astra.cpp` files are development snapshots, not the recommended submission.

## Implementation

The solver uses a two-watched-literal CDCL engine for edge constraints, exact pearl-rule clauses, and lazy connectivity clauses. A bounded portfolio switches between forbidding premature cycles and requiring an edge across a disconnected pearl component boundary. It then shortens the saved valid loop with seven-state constraint-domain regional search, valid face flips, and shortest-path replacements in pearl-free portions. Every saved incumbent passes an internal full validity check. Random choices use a fixed seed.

Easy cases stop early. Improvement work normally finishes within about one second; difficult feasibility searches may continue to a hard internal deadline of 7.3 seconds. The judge wrapper in the supplied statement is incomplete, so this program implements standalone stdin/stdout and honours OUTPUT_PATH when set.

## Final validation

Final full-suite run: **63/63 valid and in time** using the independent checker and `run_tests_astra.py`.

| Suite | Valid | Maximum observed runtime |
|---|---:|---:|
| Repository cases | 33/33 | 1.440 s |
| Independent generated cases | 15/15 | 5.170 s |
| Rotation/reflection cases | 15/15 | 2.414 s |

Median runtime: 0.788 s. Maximum: 5.170 s. Cases below one second: 54/63.
The sample is solved with length 18, the optimum stated in the problem.

Against the 48-case validated Astra v5 baseline, final output lengths are shorter on 14 cases, equal on 21, and longer on 13. The final version is not uniformly better on every case. Runs occurred in a shared workspace with concurrent work, and clock-based stopping means timings and lengths can vary with available CPU.

No official judge score or hidden-test reference lengths are available. The runner's `rel` value is a linear local comparison, NOT the Masyu contest score; a single-candidate `100.0` does not mean a perfect answer. These results establish local validity and observed performance, not universal optimality or a guarantee of solving all hidden instances. If the search budget expires before the first valid loop, the solver has no generally valid fallback.

## Reproduce

From `p4`:

```bash
g++ -O2 -std=c++17 sol_astra.cpp -o .bin_astra/sol_astra
.bin_astra/sol_astra < tests/00_sample.in
python3 run_tests_astra.py --tl 10 --tests "tests/*.in,tests_astra/*.in,variants_astra/*.in" sol_astra.cpp
```

The isolated runner is derived from the repository runner, with paths changed to `check_astra.py`, `.bin_astra`, `out_astra`, and `scores_astra.jsonl`; it additionally accepts comma-separated test globs. It leaves the shared runner and score log untouched.

`gen_astra.py` generates 15 known-solvable tests and their witness outputs; all witnesses passed the independent checker. `gen_variants_astra.py` creates 15 orientation checks. Witness loop lengths are feasible upper bounds, not reference optima.

Raw per-case measurements are in `benchmark_astra.csv`; the full run history is in `scores_astra.jsonl`.

## Development evidence

The initial domain-only solver passed 21/33 cases. Conflict learning and a bounded portfolio raised that to all 48 initial and independent tests. Orientation checks exposed timeout sensitivity; adding alternate connectivity cuts and allowing longer initial feasibility searches resolved all failures in the final 63-case run. The shortest-path pass tested before local repair regressed overall, so the final source applies it only after the main repair passes, retaining an answer only when shorter and valid. Historical versions are retained for review.

## Safety checks

AddressSanitizer and UndefinedBehaviorSanitizer passed on four representative cases, including a 50×50 input; all four outputs also passed the independent checker. LeakSanitizer is unsupported under this environment's ptrace setup, so leak detection was disabled. Details: `sanitizer_astra.txt`.

Frozen source SHA-256: `51e27c43fd014d5570fcff7f5f5820629d4f8164822db34b2a95c1689da6f1f5`.
