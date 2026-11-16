# P4 Masyu — handoff

## UPDATE (after the first handoff): submit `sol_v7.cpp`

`sol_v7.cpp` is the combined solver and replaces v4 as the recommendation.
`run_tests.py --tl 10 sol_v4.cpp sol_v7.cpp` on all 41 tests: v7 mean rel 99.2,
v4 96.4, both all valid and in time. v7's slowest test was 1.43 s (v4 0.93 s).
Across 3 seeds on the 28 larger tests the quality-sum was about 25.8 (v7) vs 24.5 (v4),
measured before the last two changes below.

What v7 does differently from v4 (default mode `v7` in `dev/sol_v2_dev.cpp`):
1. SA from the border for 0.25 s. If that is already valid (sparse boards), go to step 4.
2. Candidate A: window-by-window SAT repair of the SA loop, then a 30 ms hard
   face-flip polish (same flip code, penalty 1000 so pearl rules are never broken).
3. Candidate B (only grids of at most 1300 cells, or when A failed): SAT from scratch
   plus 50 ms flip polish; keep the shorter of A and B. If A failed, SAT runs as
   hedged restarts (hint / no hint, then randomised variable order, growing budgets)
   to avoid the slow tail seen on test 43 (4.7 s with a single hinted solve).
4. SAT-window LNS until 0.9 s, with a 12 ms hard flip burst every 40 steps
   (this also gives free equal-length moves), and a whole-grid exact step on small grids.
5. If the first valid loop arrives later than 0.65 s, 0.25 s of LNS is still run.
6. Every candidate and the final loop pass an internal validity check before use.

Known weak spots:
- Grid-filling dense boards (tests 42, 43): repair is sometimes slow, so runtime
  varies between 0.9 s and about 3 s from run to run.
- If no valid loop is found by about 8.6 s, v7 still prints the border rectangle
  (invalid). That only matters when the test would score 0 anyway.
- Results depend on machine load: all quality numbers were measured with other jobs
  running on the box.

Not done from the plan below: the shortest-path shortcut (item 4).
`sol_v7.cpp` and the dev changes are not committed.

---

## Earlier handoff (written 19:45)

## What to submit right now

**`sol_v4.cpp`** (C++17, single file, standard library only). Nothing newer beats it.
It reads the grid from stdin and writes to `$OUTPUT_PATH` if set, else stdout.
Valid on all 41 local tests, about 0.9 s each.

Open risk: the statement's template is C and its `main()` is cut off. If the judge
only accepts C, this needs porting.

## What this session changed

Nothing in the solvers. This session (the "combine everything" request) only read
the repo: `reports/Masyu fastest solution.md`, `potentialIssues.md`,
`sol_astra.cpp`, `README_astra.md`. No new solution file was written.

Small additions:
- `HANDOFF.md` (this file).
- `dev/tools/` — copies of the evaluation scripts and best-known lengths from the
  session scratchpad, which may be wiped (`ev.sh`, `score.py`, `runall.sh`,
  `refs.json`). The scripts hardcode the old scratchpad path in `S=`; change that
  line before using them.

Nothing from this session is committed. `HANDOFF.md` and `dev/tools/` are untracked.

## Files that matter

| File | What it is |
|---|---|
| `sol_v4.cpp` | Current best. SA from border → local SAT repair of broken pearls → SAT-window LNS. |
| `dev/sol_v2_dev.cpp` + `dev/sat_part.h` | Source that the single files are built from. Default mode is `multi` (this is v5/v6 behaviour, not v4). |
| `sol_v5.cpp`, `sol_v6.cpp` | Restarts / plateau moves. Not better than v4 (within noise, worse on grid-filling tests). |
| `sol_astra.cpp` | The other agent's solver. SAT first → face-flip descent → window DFS → shortest-path shortcuts. |
| `check.py`, `gen.py` (`python3 gen.py extra` for tests 40–47) | Checker and generators. |

Build a single file from dev: replace the line `#include "sat_part.h"` in
`dev/sol_v2_dev.cpp` with the contents of `dev/sat_part.h`.

Env knobs in the dev build: `MODE` (`multi` default, `full` = v4 flow, `sat`, `sa`,
`lns`), `TEND`, `NRS`, `FSA`, `PLP`, `WMIN`, `WMAX`, `CL`, `SAW`, `SATS`, `SATE`, `SEED`.

## What the evidence says

- Small grids (≤ 12×12): v4 matches proven optima.
- v4 vs astra on my 41 tests: quality-sum 37.2 vs 35.1. They win on different tests.
  - astra is better on dense boards (17, 21, 25) and the grid-filling ones (42, 43),
    but took 2.7 s and 6.5 s on 42 and 43.
  - v4 is better on mid and sparse boards (14, 16, 18, 26, 27, 30, 31, 32, 44, 47).
- The research report puts the per-test best of the two at about 96.8 against about
  91.5 for either alone. That is an estimate for a combined solver, not a measurement.
- Running v4 past 1 s does not pay (quality 10.96 at 4.9 s vs 10.87 at 0.9 s on the
  12 large tests).

## Plan for the combined solver (not started)

Build `sol_v7.cpp` from the dev sources. In order of expected gain:

1. **Hard face-flip polish on any valid loop.** Reuse `sa::run` with a very large
   penalty (e.g. `W=1000`) and a low temperature, started from a valid loop, for
   20–50 ms. This is astra's biggest win (it took one 50×50 loop from 1386 to 378 in
   22 ms). v4 never does this; its LNS spends SAT windows removing slack that flips
   remove for free.
2. **Two candidates on dense boards, keep the shorter.**
   (a) v4 path: SA from border → `repairLoop`.
   (b) astra path: `satSolve` → hard flip polish.
   On sparse boards (SA reaches zero violations) skip (b).
3. **Interleave flips with LNS.** Every N LNS steps, or when the fail counter grows,
   run 10–20 ms of hard flips. Neutral corner flips give plateau moves for free,
   replacing the expensive SAT plateau step.
4. **Shortest-path shortcut** between consecutive pearls on long pearl-free
   stretches (astra's `shortcutImprove`, BFS through cells not on the loop).
5. **Safety.** Validate the final loop inside the solver before printing and fall
   back to the previous valid best. Never print the border rectangle as a fallback
   (v4 still does when everything fails); keep trying SAT to about 8.5 s instead.
6. Keep the 0.9 s deadline. Do not stop early.

Measure every step with `dev/tools/ev.sh` + `score.py` over at least 3 seeds;
single-run differences under about 1 quality point (out of 28 large tests) are noise.
Only replace v4 if the new file is `all valid and in time` in `run_tests.py` and
better across seeds.

## Housekeeping notes

- Earlier commits of mine swept in `*_astra` files and build outputs via
  `git add -A`; commit `e866bd5` untracked them (files kept on disk, nothing pushed).
- `research_notes/Masyu fastest solution/literature.md` is still tracked; I did not
  know whose it is, so I left it.
- `scores.jsonl` has uncommitted appended runs from `run_tests.py`.
