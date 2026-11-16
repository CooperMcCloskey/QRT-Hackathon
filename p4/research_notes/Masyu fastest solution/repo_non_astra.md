# Masyu p4: the non-"astra" solvers in the repo (sol_v1 .. sol_v5)

Scope and method. All sources are repo files (paths relative to `/home/server/coding/competitions/QRT-Hackathon`). Numbers are labelled "repo" (taken from `p4/scores.jsonl`, written by `tools/run_tests.py`) or "mine" (I rebuilt the code into the scratchpad and re-ran it with `p4/check.py`). The scratchpad is `/tmp/claude-1000/-home-server-coding-competitions/7aa75855-1aa5-43ba-9bc6-4dfd2d2f50cb/scratchpad/na`. The repo was changing while I read it. When I started there were three commits (sol_v1 baseline, CDCL, LNS). Commits `e4648cf` (final single-file sol_v2), `8526be6` (v3/v4) and `7871633` (v5 plus 8 extra tests, now 41 in `tests/`) arrived during the task. The non-astra line therefore goes v1 to v5, not only v1 and v2. I snapshotted `sol_v3/4/5.cpp` at 19:28, and file line numbers below refer to the files as read then.

Caveat on all my timings: the machine load average was 17-19 on 12 cores while I measured, because the other agent was benchmarking astra versions. Wall-clock-limited solvers (SA, LNS) are noisy under that load. See the variance section.

Scoring used throughout (`p4/statement.md`): `score = 100*min(1, s*Lref/L)^2 * (0.5+0.5*exp(-max(0,T-1)/5))`, TL 10 s, s=1. Lref is the best valid L for that test found anywhere in the repo or in my runs: `p4/out/*`, `p4/out_astra/*`, `scores_astra.jsonl` and my reruns. Several Lref values are astra outputs or my own runs, and astra files keep changing. The Lref list is in the "per test" table. Per-run noise (about +/-5 points on the average) is larger than most differences between adjacent versions.

---

## Q1. What algorithm does each non-astra version use?

### Takeaway
- v1 (`sol_v1.cpp`) runs penalised simulated annealing (SA) on face flips from the border rectangle for a fixed 0.6 s. If that finds no valid loop it falls back to a slow DFS with failed-literal probing, then to an invalid border rectangle. Its DFS almost never works above about 20x15.
- v2 (`sol_v2.cpp`) adds a hand-written CDCL SAT solver (Boolean satisfiability, conflict-driven clause learning) with in-search subtour cuts, then LNS (large neighbourhood search) that re-solves small windows with SAT.
- v3 swaps the Sinz counter for a native cardinality constraint. v4 adds window-by-window repair of the SA result. v5 adds two restarts and plateau moves.

### Cited Findings

**Shared model (v1 and v2 `build()`)** — `p4/sol_v1.cpp:40-105`. Edges are the variables: `E = R*(C-1) + (R-1)*C`, indexed by `eu/ev` and `cedge[cell*4+dir]` (N, E, S, W). Each pearl gets a list of allowed edge-literal configurations in `pat[pi].cfg` (`sol_v1.cpp:65-103`).
- Black: 4 configs, which are two ways to turn on each axis. Each fixes the two on-edges, the two beyond-edges (straight through both neighbours), and the off-edges.
- White: 2 axes, each with 2 configs, "beyond-1 off" or "beyond-1 on and beyond-2 off". This encodes "straight, and at least one neighbour turns".
- `edgePearls[e]` is the reverse index from an edge to the pearls it affects.

**v1 SA** — `sol_v1.cpp:371-481`.
- The state is a single simple loop, started from the border rectangle (`main`, lines 520-522). The move is a face flip: pick a random ON edge, then one of its two faces (`:445-446`). It toggles the 4 face edges unless the mask is 15, 5 or 10 (`:450`), or a corner would hit a cell that already has degree > 0 (`:458`). So it never breaks the single-loop invariant.
- The cost is `dL + W*(newViol-oldViol)` with W=20, evaluated only on the pearls near the face (`facePearls`, `:465-471`). Temperature runs geometrically from 3.0 to 0.3 (`:442`).
- `best` is recorded only when `nViol == 0` (`:475`). Measured SA speed is about 3.5-4.5M flips/s (my instrumented copy, under load).
- v1 runs it from t=0 to 0.6 s (`:534`), so every v1 run has T of about 0.60-0.63 s.

**v1 DFS** — `sol_v1.cpp:108-368`.
- It has trail-based undo and per-edge state `st`, with per-cell `don/dunk` (edges on and unknown). Path-end endpoints are tracked with `endc/plen/ppr`, so closing a loop is allowed only when it holds all pearls and all ON edges (`assign`, `:122-162`).
- Propagation is `checkCell` (`:164`: degree rules, a forced edge when one is on and one unknown, a pearl with 2 unknowns forces both on) and `checkPearl` (`:185`: an edge literal common to all still-compatible configs is forced).
- `connected()` (`:238`) is a BFS that prunes if the non-OFF edges cannot connect all pearls.
- `probe()` (`:267`) is failed-literal probing to a fixpoint. It calls `tryLit` on every edge, and each call runs full propagation plus BFS. The budget per node is `4*E` tries (`:304`).
- `search()` branches on the path end with the fewest unknown edges, picks a random incident edge, and uses restarts with node limit 200 growing by x1.5 (`:359-365`).
- The deadline is checked only every 64 nodes (`:295`).

**v1 control flow** — `main` `sol_v1.cpp:532-547`: SA to 0.6 s; if `best.empty()`, `dfs::solve(8.0)` then SA polish for 0.3 s; and at `:546` `if (best.empty()) best = start;` prints the border rectangle, which is invalid whenever a pearl is off the border.

**v2 CDCL SAT** — `sol_v2.cpp:371-659` (identical to `p4/dev/sat_part.h` except for a blank line).
- Data structures: a clause arena (`arena`, `cls{start,size,learnt,act}`), 2-watched-literal propagation (`:457`, MiniSat style), a VSIDS activity heap (`:386-404`, decay 0.95), first-UIP learning with simple minimisation (`analyze`, `:492`), Luby restarts with unit 100 (`:614`), learnt-clause DB reduction at 4000 learnts (`:539`, `:648`), and phase saving with default phase 0 (all edges off).
- Base CNF `buildBase` (`:665-730`): per cell, "degree not 1" and "no three edges on" as clauses, and every pearl cell has at least one edge. Black pearls: exactly one of N/S and one of E/W, with the beyond-edge forced on and the neighbour's side edges off. White pearls: N=S, E=W, no mixing, and `(not e1 or not b1 or not b2)` forcing a turn at one end.
- Subtour handling is two-tier.
  - In-search hook `cycleCheck()` (`:565-600`). After propagation, for each newly true edge it walks the path. If it closes a cycle holding some but not all pearls, it creates the clause "some edge leaving the cycle's cell set must be on". If that clause is all-false it becomes a conflict (`:629-646`).
  - Fallback lazy loop `solveLoop` (`:731-802`) re-solves after each model: it finds components, adds a boundary cut per cycle, and also a cut for the region enclosed (ray casting, `:768-791`). It returns the component that contains all pearls (extra empty cycles are dropped).
- Statistics from `MODE=sat` (mine): `hook` conflicts are 1-2.8k on big instances and `iters` (lazy re-solves) is 0-3, so the in-search hook handles almost everything.

**v2 LNS with SAT windows** — `lnsCells` `sol_v2.cpp:837-863`, `lnsStep` `:864`.
- Free all edges touching a window cell, fix all other edges to the current loop with unit clauses, and add `sum(free ON) <= curFree-2` with a Sinz sequential counter (`atMostK`, `:813`, `(n-1)*K` aux vars), so only a strictly shorter loop is accepted.
- Phases are set to the current solution (`:854`) and the conflict limit is 500 (`:1084`, `CL`).
- `buildBase(E+aux)` rebuilds the CNF from scratch every step. `inBox` limits clause generation to the window plus a margin (cells margin 1, pearls margin 3, `:664`).
- `lnsStretch` (`:887`, windows along the loop) exists but is off by default (`SP=0`, `:1087`).

**v2 final driver ("full" mode, the default)** — `sol_v2.cpp:1082-1133`.
1. SA from the border for `tsa=0.3` s (`:1086`, `:1091-1092`).
2. If the SA found no valid loop (or `SATALWAYS`), `satSolve(8.0, &hint)` (`:1097-1104`). The SA's final, possibly violating, state is used as the CDCL phase hint (`:807`).
3. Loop LNS windows: random loop edge, window height and width `3+grow..8+grow` with `grow=min(fails/30,6)`. Stop at `tend=0.9` s (`:1085`, `:1108-1120`).
4. Output; fallback `best = start` at `:1175`.

**History of v2** (git):
- `b61a3ac` committed the CDCL in `sat_part.h`. Its default driver was still the v1 pipeline (SA 0.6 s, then DFS), and SAT ran only with `MODE=sat` or `combo`. It also used `#include "sat_part.h"`, so it was not a single-file submission.
- `9eef82c` (LNS) made the default "full" mode `satSolve(9.0)` first, always, then SA, then LNS to 0.9 s.
- `e4648cf` is the final single-file version: SA first, SAT only if SA failed, SAT hint from the SA state, `lnsCells`/`lnsStretch`, and a `SEED` knob.

**v3** — `sol_v3.cpp` (diff to v2: lines 412-420, 473-483, 520-523). A native at-most-K constraint inside the CDCL replaces the Sinz counter: `cardOn/cardK/cardVars`, with lazy reason clauses (reason id -2, `buildCardReason`). The propagation loop counts newly true card vars and, on overflow, returns a conflict clause. `lnsCells` no longer adds aux variables (`buildBase(E)`).

**v4** — `sol_v4.cpp` (diff to v3). `repairLoop()` (lines 937-984): take the SA result, a single loop with pearl violations. Pick a violated pearl and free a square window of radius 3, then x1.5 growing, around it. The `reqP/bldP` sets restrict which pearls must be on the loop and which rule clauses are built. `lnsCells(..., useCard=false)` accepts any valid result (`:889`). Repeat until no pearl is violated. In the default driver this runs after the 0.3 s SA when SA found no valid loop, before falling back to SAT (`:1174-1182`).

**v5** — `sol_v5.cpp` (diff to v4). Default mode `multi` (`:1161-1209`).
- `NRS=2` restarts, each with a slot of `(tend-elapsed)/(NR-k)`. Each restart runs SA for 20% of its slot from the border, then `repairLoop` (deadline 6 s if no best yet), then SAT only if nothing yet, then LNS to the end of the slot. It keeps the best.
- LNS gets plateau moves (`plateau=true`, 20% of steps, `K=curFree`, plus a clause forcing a different assignment; an equal-length result is adopted, `:873-891`) and, for `E<600`, a global window every 10th step (`:1180`).
- `satSolve(8.5)` is the last resort.

### Inferences
- The only repeatable structural change that moved the large-instance score is making the LNS step cheap (v2 to v3) and replacing "SAT to get any valid loop" with "SA then local repair" (v4). See Q4.
- All non-astra versions share the same pearl model and the same SA, so the differences below come from the driver and the SAT/LNS efficiency.

### Gaps
- I did not diff `dev/sol_v2_dev.cpp` line by line against root `sol_v2.cpp`. It is the same driver and `#include "sat_part.h"`, and the root file inlines the header: the header matches the root file's lines 371-908 except for one blank line.
- `dev/sat_part.h` and `dev/sol_v2_dev.cpp` kept changing after 19:25, as part of v3 to v5 development. I read the snapshots only through the root `sol_v3/4/5.cpp`.

---

## Q2. Per-test L and T, failures, and the contest score

### Takeaway
- v1 gets 59-60 average over 33 tests (9 invalid outputs, all of them the fallback border rectangle after the 8 s DFS budget).
- v2 final gets 77-87 (0 invalid, but very run-dependent). v3 gets 88.6, v4 91.6 (repo run: 92.0) and v5 93.7 in single runs.
- For context, with the same Lref, astra v6 scores about 92.7 on the same 33 tests.

### Cited Findings

**Tests** (`p4/gen.py`, `p4/tests/`): 33 tests, 8 sizes (6x6 to 50x50) x 4 densities. A random simple loop is grown by random face flips (`gen.py:make`). Pearls are placed on eligible loop cells with probability `dens*bw*2`, so a valid solution is guaranteed. Actual pearl density is low at large sizes: 50x50 `d100` has 204 pearls (8.2%), 40x40 `d100` has 163 (10.2%), the densest is 20x15 `d100` with 66 (22%). Per-test pearl counts are in column "pearls" below. The 8 extra tests 40-47 added by `7871633` (`gen.py:extra`) are thin grids, "fill" loops and very sparse tests; they are not in the 33.

Legend: each cell is `L (T s if T > 1 s or invalid) **score**`, where score is the full contest score of that run using the Lref column. Rows with no T shown ran in T <= 1.0 s (v1: about 0.60-0.63 s; v2 and later: about 0.90-0.96 s), so their speed factor is exactly 1.0. INV means the output was invalid (the border-rectangle fallback, after about 8 s of DFS or SAT). Columns:
- `v1 repo`: `p4/scores.jsonl` run 2 of sol_v1. `v1 mine`: my run of sol_v1.
- `v2@b61`: v2 as of commit b61a3ac, default driver (that driver is the v1 pipeline; SAT is not used by default). `v2@9ee`: v2 as of commit 9eef82c (SAT first, then SA, then LNS).
- `v2 final repo-A`, `repo-B`: `scores.jsonl` runs 3 and 4 of the final single-file sol_v2 (commit e4648cf). `v2 final mine-1`, `mine-2`: two of my reruns of the same code.
- `v3 mine`, `v4 mine`, `v5 mine`: my single runs of sol_v3/v4/v5. `v4 repo`: `scores.jsonl` run 5 of sol_v4.

| test | pearls | Lref | v1 repo | v1 mine | v2@b61 | v2@9ee | v2 final repo-A | v2 final repo-B | v2 final mine-1 | v2 final mine-2 | v3 mine | v4 mine | v4 repo | v5 mine |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| 00_sample | 4 | 18 | 18 **100** | 18 **100** | 18 **100** | 18 **100** | 18 **100** | 18 **100** | 18 **100** | 18 **100** | 18 **100** | 18 **100** | 18 **100** | 18 **100** |
| 01_6x6_d100 | 5 | 14 | 14 **100** | 14 **100** | 14 **100** | 14 **100** | 14 **100** | 14 **100** | 14 **100** | 14 **100** | 14 **100** | 14 **100** | 14 **100** | 14 **100** |
| 02_6x6_d50 | 4 | 16 | 16 **100** | 16 **100** | 16 **100** | 16 **100** | 16 **100** | 16 **100** | 16 **100** | 16 **100** | 16 **100** | 16 **100** | 16 **100** | 16 **100** |
| 03_6x6_d20 | 3 | 14 | 14 **100** | 14 **100** | 14 **100** | 14 **100** | 14 **100** | 14 **100** | 14 **100** | 14 **100** | 14 **100** | 14 **100** | 14 **100** | 14 **100** |
| 04_6x6_d5 | 2 | 10 | 10 **100** | 10 **100** | 10 **100** | 10 **100** | 10 **100** | 10 **100** | 10 **100** | 10 **100** | 10 **100** | 10 **100** | 10 **100** | 10 **100** |
| 05_8x10_d100 | 17 | 40 | 44 **83** | 48 **69** | 40 **100** | 40 **100** | 40 **100** | 40 **100** | 40 **100** | 40 **100** | 40 **100** | 40 **100** | 40 **100** | 40 **100** |
| 06_8x10_d50 | 13 | 28 | 28 **100** | 28 **100** | 28 **100** | 28 **100** | 28 **100** | 28 **100** | 28 **100** | 28 **100** | 28 **100** | 28 **100** | 28 **100** | 28 **100** |
| 07_8x10_d20 | 6 | 24 | 24 **100** | 24 **100** | 24 **100** | 24 **100** | 24 **100** | 24 **100** | 24 **100** | 24 **100** | 24 **100** | 24 **100** | 24 **100** | 24 **100** |
| 08_8x10_d5 | 2 | 20 | 20 **100** | 20 **100** | 20 **100** | 20 **100** | 20 **100** | 20 **100** | 20 **100** | 20 **100** | 20 **100** | 20 **100** | 20 **100** | 20 **100** |
| 09_12x12_d100 | 29 | 68 | 70 **94** | 76 **80** | 78 **76** | 68 **100** | 68 **100** | 68 **100** | 68 **100** | 68 **100** | 68 **100** | 68 **100** | 68 **100** | 68 **100** |
| 10_12x12_d50 | 15 | 44 | 72 **37** | 74 **35** | 76 (1.00s) **34** | 46 **91** | 44 **100** | 44 **100** | 52 **72** | 44 **100** | 44 **100** | 44 **100** | 46 **91** | 44 **100** |
| 11_12x12_d20 | 4 | 22 | 22 **100** | 22 **100** | 22 **100** | 22 **100** | 22 **100** | 22 **100** | 22 **100** | 22 **100** | 22 **100** | 22 **100** | 22 **100** | 22 **100** |
| 12_12x12_d5 | 4 | 28 | 28 **100** | 28 **100** | 28 **100** | 28 **100** | 28 **100** | 28 **100** | 28 **100** | 28 **100** | 28 **100** | 28 **100** | 28 **100** | 28 **100** |
| 13_20x15_d100 | 66 | 144 | 206 (1.44s) **47** | 210 (3.53s) **38** | 218 (2.66s) **37** | 174 **68** | 152 **90** | 168 **73** | 180 **64** | 166 **75** | 144 **100** | 144 **100** | 154 **87** | 160 **81** |
| 14_20x15_d50 | 23 | 74 | 164 **20** | 106 (1.02s) **49** | 108 (1.18s) **46** | 88 **71** | 84 **78** | 104 **51** | 76 **95** | 76 **95** | 80 **86** | 84 **78** | 86 **74** | 82 **81** |
| 15_20x15_d20 | 8 | 42 | 44 **91** | 44 **91** | 44 **91** | 42 **100** | 42 **100** | 42 **100** | 42 **100** | 42 **100** | 42 **100** | 42 **100** | 42 **100** | 44 **91** |
| 16_20x15_d5 | 4 | 36 | 36 **100** | 36 **100** | 38 **90** | 36 **100** | 36 **100** | 36 **100** | 36 **100** | 36 **100** | 36 **100** | 36 **100** | 36 **100** | 36 **100** |
| 17_25x25_d100 | 94 | 212 | INV (8.0s) **0** | INV (8.2s) **0** | INV (8.1s) **0** | 246 **74** | 232 **84** | 266 **64** | 292 **53** | 258 **68** | 222 **91** | 266 **64** | 224 **90** | 256 **69** |
| 18_25x25_d50 | 36 | 116 | 202 (1.76s) **31** | INV (8.1s) **0** | 148 (1.57s) **58** | 186 **39** | 120 **93** | 162 **51** | 134 **75** | 214 **29** | 116 **100** | 134 **75** | 118 **97** | 120 **93** |
| 19_25x25_d20 | 13 | 78 | 86 **82** | 108 **52** | 94 **69** | 96 **66** | 86 **82** | 82 **90** | 82 **90** | 86 **82** | 82 **90** | 84 **86** | 88 **79** | 80 **95** |
| 20_25x25_d5 | 6 | 58 | 60 **93** | 60 **93** | 62 **88** | 58 **100** | 60 **93** | 58 **100** | 58 **100** | 58 **100** | 58 **100** | 60 **93** | 58 **100** | 58 **100** |
| 21_30x40_d100 | 149 | 362 | INV (8.4s) **0** | INV (8.1s) **0** | INV (8.7s) **0** | 496 **53** | 500 **52** | 532 **46** | 556 **42** | 484 **56** | 424 **73** | 368 **97** | 398 **83** | 362 **100** |
| 22_30x40_d50 | 78 | 248 | INV (8.1s) **0** | INV (8.0s) **0** | INV (8.2s) **0** | 398 **39** | 264 **88** | 282 **77** | 366 **46** | 398 **39** | 280 **78** | 268 **86** | 280 **78** | 262 **90** |
| 23_30x40_d20 | 26 | 146 | 184 **63** | 182 **64** | 166 **77** | 182 **64** | 174 **70** | 196 **55** | 172 **72** | 146 **100** | 158 **85** | 174 **70** | 150 **95** | 166 **77** |
| 24_30x40_d5 | 6 | 70 | 76 **85** | 84 **69** | 80 **77** | 74 **89** | 72 **95** | 74 **89** | 74 **89** | 72 **95** | 72 **95** | 72 **95** | 74 **89** | 74 **89** |
| 25_40x40_d100 | 163 | 412 | INV (8.4s) **0** | INV (10.2s) **0** | INV (8.1s) **0** | 554 **55** | 520 **63** | 480 **74** | 638 **42** | 544 **57** | 578 **51** | 534 **60** | 598 **47** | 476 **75** |
| 26_40x40_d50 | 122 | 402 | INV (8.0s) **0** | INV (9.6s) **0** | INV (9.2s) **0** | 766 **28** | 672 **36** | 628 **41** | 758 **28** | 774 **27** | 536 **56** | 436 **85** | 436 **85** | 422 **91** |
| 27_40x40_d20 | 40 | 228 | INV (8.2s) **0** | INV (8.6s) **0** | INV (8.3s) **0** | 410 **31** | 236 **93** | 204 **100** | 728 (1.06s) **10** | 268 **72** | 404 **32** | 234 **95** | 240 **90** | 228 **100** |
| 28_40x40_d5 | 5 | 58 | 58 **100** | 70 **69** | 70 **69** | 66 **77** | 58 **100** | 58 **100** | 58 **100** | 62 **88** | 62 **88** | 60 **93** | 62 **88** | 58 **100** |
| 29_50x50_d100 | 204 | 558 | INV (8.6s) **0** | INV (11.8s) **0** | INV (8.5s) **0** | 866 **42** | 664 **71** | 900 **38** | 812 **47** | 890 **39** | 998 (1.18s) **31** | 830 **45** | 666 **70** | 636 **77** |
| 30_50x50_d50 | 126 | 484 | INV (8.2s) **0** | INV (8.7s) **0** | INV (8.3s) **0** | 1066 (1.39s) **20** | 878 **30** | 642 **57** | 910 **28** | 1046 **21** | 554 **76** | 484 **100** | 488 **98** | 502 **93** |
| 31_50x50_d20 | 59 | 298 | INV (8.3s) **0** | 368 **66** | INV (8.1s) **0** | 772 **15** | 332 **81** | 752 **16** | 440 **46** | 466 **41** | 298 **100** | 298 **100** | 308 **94** | 312 **91** |
| 32_50x50_d5 | 12 | 114 | 156 **53** | 154 **55** | 140 **66** | 114 **100** | 134 **72** | 114 **100** | 206 **31** | 120 **90** | 118 **93** | 114 **100** | 114 **100** | 116 **97** |
| **AVG33** | | | **60.0** | **58.5** | **59.9** | **76.5** | **87.0** | **82.5** | **76.7** | **81.0** | **88.6** | **91.6** | **92.0** | **93.7** |
| avg tests 0-16 | | | 86.6 | 86.0 | 86.7 | 95.9 | 98.1 | 95.5 | 95.9 | 98.2 | 99.2 | 98.7 | 97.2 | 97.3 |
| avg tests 17-24 | | | 44.3 | 34.9 | 46.1 | 65.6 | 82.3 | 71.7 | 71.0 | 71.1 | 89.1 | 83.2 | 88.8 | 89.2 |
| avg tests 25-32 | | | 19.2 | 23.6 | 16.9 | 45.9 | 68.2 | 65.7 | 41.4 | 54.5 | 65.9 | 84.8 | 84.1 | 90.4 |
| #invalid | | | 9 | 9 | 9 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| max T (s) | | | 8.58 | 11.84 | 9.23 | 1.39 | 0.91 | 0.94 | 1.06 | 0.95 | 1.18 | 0.95 | 0.94 | 0.96 |

Table notes.
- Speed: v1 takes 0.60 s when SA alone succeeds, and about 0.90 s on tests 05, 09, 10 and 14 because the SA polish after a successful DFS adds 0.3 s (`sol_v1.cpp:542`). When v1 needs DFS it takes 1.4-3.5 s (valid, 20x15 d100 and 25x25 d50) or about 8 s (invalid).
- v2 final-code averages over 7 runs of the same code (my 5 reruns plus 2 repo runs): 76.7, 83.0, 81.7, 84.0, 81.0 and 87.0, 82.5 (mean about 82.3). Use about 82 (range 77-87) as the v2 figure, not any single run.
- Lref sources (best known): mostly `out_astra/*` astra outputs (sol_v5/v6/v7/sol_astra) or my reruns (21: 362 from my v5 run; 31: 298 from my v3 run; 30: 484 from my v4 run). Because astra files were changing, treat Lref as "best known at about 19:30".

**Why each failure happened** (see also the single-component runs in the next section).
- v1/b61 invalid on 25x25 d100, 30x40 d100/d50, 40x40 d100/d50/d20, 50x50 d100/d50 (and v1 on 25x25 d50, 30x40 d20 and 50x50 d20, depending on the run). In every case `best` was empty after the 0.6 s SA (the penalised SA never reached zero violations), the DFS ran to its 8.0 s deadline without a result, and `best = start` printed the border rectangle, which breaks any inner pearl (`sol_v1.cpp:536-546`). The checker messages confirm it: "pearl at (r,c) not on loop", "white (0,15) no turn beside".
- DFS-only results (`MODE=dfs`, mine): it finds valid but very long loops only up to 20x15 d100 (L=240 in 7.2 s with just 85 nodes, which is 85 ms per node) and 25x25 d50 (L=412 in 3.7 s). Node counts at 8.5 s timeout on larger grids are only 64-3840. This is because each node calls `probe()` with up to `4*E` full propagation+BFS passes. On 50x50, `E=4900`, so a node costs seconds.
- DFS also overshoots its deadline. The check is every 64 nodes (`:295`), at about 0.1 s per node on big grids, so runs end at 9-11.8 s. Two of my v1 runs exceeded the 10 s TL (25_40x40_d100: 10.23 s; 29_50x50_d100: 11.84 s), under load.
- SA-only (`MODE=sa`, 0.85 s, mine): invalid (violations remain) on all dense tests: 05, 09, 10, 13, 14, 17, 18, 21, 22, 25, 26, 27, 29, 30, 31. It is valid on the small and sparse tests (all d5, and d20 up to 30x40), but also invalid on 40x40 d20 (27) and 50x50 d20 (31).
- Pure SAT (`MODE=sat`, mine): valid on all 33 tests, in 0.01-0.43 s except 50x50 d50 at 1.14 s (22.9k conflicts). The loops are 2-6x longer than optimal because SAT has no objective, e.g. 25x25 d20 L=364 vs Lref 78 and 50x50 d5 L=350 vs 114. So SAT finds validity quickly, but the L it returns is poor and needs LNS.

**v2 later versions: validity.** All v2 final, v3, v4 and v5 runs were valid on all 33 tests in all my reruns and the repo runs (0 invalid). On the 15 harder `tests_astra` tests (mine), v2 final was valid on all 15, but two took long: 50x50_s113_d100 L=2250 in 7.28 s and 50x50_s113_d25 L=1564 in 4.51 s (astra v6 gets 2162/1322 at 2.3/0.78 s). v1 was invalid on 5 of the 15 and took 7-10.6 s on three.

**Where the repo's own numbers agree with mine.** `scores.jsonl` run 2 (v1) failed the same nine tests as my v1 runs, and all five v2/v4 repo runs were valid with T 0.90-0.94 s. v1 is nondeterministic (time-limited SA): across 3 runs the averages are 62.6, 60.0 and 58.9 and the set of failing tests changes (31_50x50_d20: 328, INV, 368).

### Inferences
- v1's score (60) is made almost entirely by the small and sparse tests. Everything from 25x25 d100 upward is invalid or poor.
- The jump v1 to v2 (+17 to +27 points) is entirely validity. Since a failed test scores 0 (plus long T), 9 invalid tests are worth 9/33 = 27 points of average if solved perfectly.
- v2 to v5 (about +10 points) is quality at the sizes 25-50, where LNS efficiency matters.

### Gaps
- Lref is only "best known"; the true optimum on large tests is unknown, so absolute scores on tests 21-31 are pessimistic if there are better loops than the best found.
- No judge scores were recorded for non-astra versions (`p4/notes.md` and `scores.jsonl` contain no `judge` entries).

---

## Q3. Where does time go, and what does the speed penalty cost?

### Takeaway
- Every non-astra version after v1 uses a fixed 0.9 s wall deadline (`tend=0.9`) and finishes at T of about 0.90-0.96 s, which is inside the free first second (speed factor 1.0). v1 uses 0.6 s of SA.
- Time past 1 s only happens when SAT finds the first valid loop late. Such runs lose almost nothing in speed (1.4 s gives 0.96) but lose a lot in quality because LNS gets no time.

### Cited Findings
- Speed factor values: `0.5+0.5*exp(-(T-1)/5)` gives 1.0 at T <= 1, 0.96 at 1.4 s, 0.91 at 2 s, 0.80 at 3.5 s, 0.77 at 4 s, 0.62 at 8 s, and 0.58 at 10 s. Timeout is 0. (Computed from the formula in `p4/statement.md`.)
- v2 final timeline (`sol_v2.cpp:1082-1120`):
  - SA from 0 to 0.3 s.
  - If SA failed, SAT for as long as it needs (typically 0.01-0.5 s; "tsat" in stderr).
  - LNS until 0.9 s.
  - Output. Process overhead gives T of about 0.90-0.96 s.
- The SA 0.3 s phase finds a valid loop only on sparse/small tests (the stderr `satL=-1 saL=<L>` rows). On the rest `saL=1073741824` (never valid), so SAT runs.
- LNS throughput (stderr counters `tried`, mine), steps in the ~0.55 s LNS budget:

| version | 00_sample | 13_20x15_d100 | 21_30x40_d100 | 31_50x50_d20 |
|---|---|---|---|---|
| v2 (Sinz counter) | 695 | 231 | 173 | 138 |
| v3 (native card.) | 4187 | 564 | 554 | 346 |
| v4 | 6749 | 500 | 618 | 330 |
| v5 | 5029 | 148 | 809 | 447 |

  v3's native cardinality gives roughly 2.5-6x more LNS steps than v2.
- Most windows are proven UNSAT ("no strictly shorter loop in this window"): 50-95% of tried windows (e.g. v5 on 21_30x40_d100: 809 tried, 82 improved, 670 UNSAT). That is useful proof of local optimality but wasted time. On small boards (6x6), 100% of windows are UNSAT well before 0.9 s, yet the program keeps running to 0.9 s (harmless, since T<1).
- SAT finish times are highly variable and can eat the whole LNS budget: on 30_50x50_d50, v2 final took `tsat` of 0.44 s in one run, 1.15, 1.11 and 2.18 s in three others (all with the same SA hint recipe, differing only through load and timing). When `tsat` > 0.9 s there are `tried=0` LNS steps, and L stays at the raw SAT length (1188-1284 vs Lref 484). Similar zero-LNS runs: 27_40x40_d20 (L=728, T=1.06 s), 31_50x50_d20 (L=1140, T=1.4 s), 29_50x50_d100 with v3 (L=998, T=1.18 s).
- Dense 50x50 "fill" tests show the SAT cost: v3 `43_50x50_fill_d100` tsat=5.85 s; v2 1.54 s; v4 2.57 s; v5 1.79 s. v5's two restarts reduce but do not remove this (`tried`=24).
- Cost of running longer (mine, v2 final, `TEND` env var, 7 large tests: 13, 17, 21, 25, 26, 29, 30; single runs, load 17): average contest score 50.8 at TEND=0.9, 60.4 at 2 s, 63.2 at 4 s and 51.4 at 8 s. Quality only (ignoring speed) rises 50.9 to 66.6 to 81.9 to 82.6. So the speed penalty, 0.91 at 2 s, 0.77 at 4 s and 0.62 at 8 s, pays for itself up to about 2-4 s on big tests, but not beyond. (L at TEND 0.9/2/4/8 on 30_50x50_d50: 1058/652/486/592; on 29: 890/768/648/646.) This is an estimate from single noisy runs.
- v1 timings: 0.60 s fixed SA when valid. When SA fails and DFS succeeds: 1.4-3.5 s (20x15 d100), 1.8 s (25x25 d50). When DFS also fails: 8.0-8.6 s (output invalid anyway).

### Inferences
- Because T<1 is free, a 0.9 s deadline loses no speed score. About 0.1 s of margin remains for a slower judge. The LNS budget is only about 0.5 s on large instances after the 0.3 s SA and SAT, which is the real limit on quality.
- The SA 0.3 s is a fixed cost: it succeeds only when the instance is sparse. On dense instances it is pure overhead and SAT has to find the loop in the remaining time.

### Gaps
- I did not measure per-component time inside LNS steps (CNF rebuild vs search). From counters, a v2 step costs about 2-4 ms on 30x40 and larger (the `init`, `buildBase` and `isFree` setup are O(E) per step), and v3 about 1 ms.
- No judge-machine timing exists; the repo has no `judge` score entries.

---

## Q4. Which design decisions caused the biggest wins and losses; bugs and weaknesses

### Takeaway
- Biggest win: replacing DFS validity search by a CDCL SAT with in-search subtour cuts (all 33 tests valid in about 1.1 s or less).
- Next: native cardinality in the CDCL (v3, more LNS steps), SA plus local SAT repair instead of global SAT (v4, avoids the slow/variable first solve), restarts and plateau moves (v5).
- Main remaining weaknesses: large dense grids, where the first-solution cost still eats the LNS time.

### Cited Findings
Wins (with numbers).
1. CDCL SAT with in-search cycle hook (`sol_v2.cpp:565-657`) turned 9 invalid tests (v1) into valid ones. In `MODE=sat` all 33 are valid and `iters` (lazy re-solves) is 0-3, so the hook does almost all subtour work. Measured SAT-only times: 0.01-0.43 s, with 1.14 s for 50x50 d50.
2. LNS with SAT windows (`lnsCells`) improves SAT's raw loop several-fold: on 25x25 d20, 364 to 82-96; on 40x40 d20, 768 to 204-728; on 50x50 d5, 350 to 114-206 (ranges across runs). Without LNS (`satL` column) the solution is 2-6x Lref.
3. Phase hint from the SA state (`sol_v2.cpp:807`, `1098`). Evidence is weak but suggestive: 30_50x50_d50 `MODE=sat` (no hint) took 1.14 s, while the hinted default run once had `tsat`=0.44 s. It was not consistent across reruns (1.1-2.2 s).
4. v3 native cardinality: LNS steps 2.5-6x more per second (table above); v2 to v3 average +6.7 points in my single runs (81.0 to 88.6), mostly on tests 17-24.
5. v4 repair (`repairLoop`, `sol_v4.cpp:937-984`) replaces "SAT first solution" in the default: v4 had `repL` valid after 2-12 repair calls and `tsat` of about 0.30-0.33 s on tests where v2 needed 0.35-0.84 s (exception: 29_50x50_d100 needed 10 repair calls and `tsat`=0.93 s, leaving only 10 LNS steps, L=830). Gains are largest on 40x40-50x50: avg tests 25-32 rise from 54.5 (v2 rerun) to 65.9 (v3) to 84.8 (v4) in my runs and 84.1 in the repo run. 50x50_d50: 1046 (v2) / 554 (v3) / 484 (v4). On the fill tests v4 also beats v2/v3 (42_50x50_fill_d70: 1724 vs 1916/1988).
6. v5 (restarts, plateau moves, global window for tiny boards) adds about 2 points in avg33 (93.7) and is best at 25-32 (90.4), but is slightly worse than v4 on some tests (13: 160 vs 144; 15: 44 vs 42; 43_50x50_fill_d100: 2094 vs 2070). Differences between v4 and v5 are within run-to-run noise except tests 25-32.

Losses / bugs / weaknesses.
- Invalid fallback. `if (best.empty()) best = start;` (`sol_v1.cpp:546`, `sol_v2.cpp:1175`) prints a border rectangle that is invalid whenever any pearl is not on the border. `AGENTS.md` ("Always hold a valid answer", "Fallback") requires a valid answer; this one violates it. v1 and `b61` hit it 9 times out of 33. With v2 final and later versions I observed none (they run SAT with an 8 s deadline), but it is still unguarded if SAT times out.
- Time-based nondeterminism. Both SA and LNS stop on wall-clock time, so L varies with CPU load, even though the RNG seed is fixed (`88172645463325252`). Reruns of the final v2 code: 76.7, 83.0, 81.7, 84.0, 81.0 (mine) and 87.0, 82.5 (repo); i.e. 76.7-87.0. Per-test spreads are very large: 30_50x50_d50 L=878..1284; 31_50x50_d20 L=332..1140; 27_40x40_d20 L=204..728; 32_50x50_d5 L=114..206. The cause is SAT-late, zero-LNS runs and the SA at 0.3 s sometimes failing under load. This variance means a single judge run of v2 could land anywhere in that range. The `AGENTS.md` 75%-of-time-limit rule is respected (0.9 s), but the quality is not stable.
- LNS only accepts strictly shorter results and uses small windows (3-8 cells, growing to 14). Large-scale detours that need many windows to change at once are not reachable. After about 0.5 s on 50x50 the loop is still far from Lref, e.g. v2: 29_50x50_d100 L=812-900 vs 558 best known (about 1.5x).
- Cost per LNS step grows with grid size, because `buildBase`/`init` are O(E) per step and CNF is rebuilt each step (v2 only 138 steps for 50x50 d20, 0.55 s). v3 cuts the cardinality part but still rebuilds the CNF.
- First valid loop on very dense big grids is expensive. 50x50 fill d100 needs 1.5-5.9 s in SAT (v3: 5.9 s) and produces a poor L; v5 total T=1.8 s with only 24 LNS steps. In the astra-generated 50x50_s113_d100 test v2 final needs 7.3 s (L=2250). Large sparse grids are handled well by SA plus repair: 50x50 d5 L=114-120 (Lref 114) with v3-v5.
- v1 SA weakness: it starts from the border rectangle and penalises violated pearls with W=20, so dense instances never reach 0 violations in 0.6 s (SA-only invalid on tests 05, 09, 10, 13, 14, 17, 18, 21, 22, 25-27, 29-31). This is why 8x10 d100, 12x12 d50 and so on sometimes come out longer than optimal in v1: valid only after DFS/polish (e.g. 12x12 d50 L=72-76 vs 44).
- v1 DFS: the probe cost per node (`4*E` full propagations) makes it effectively unusable above about 20x15; deadline check every 64 nodes lets it overrun 10 s (observed 10.2 s and 11.8 s under load).
- v2 repository hygiene: in commit `b61a3ac` and `9eef82c` `sol_v2.cpp` includes `"sat_part.h"` (not a one-file submission, contrary to `AGENTS.md`: "One file, no file reads"). `e4648cf` fixed this by inlining. Commit `b61a3ac`'s default driver did not use the SAT at all (only `MODE=sat/combo`), so that commit's measured behaviour equals v1 (59.9 avg).
- Edge: in `lnsCells`, v3+ `curFree < 2` returns false, so no improvement is possible for loops with fewer than 2 free ON edges. Not a validity risk.
- `p4/scores.jsonl` has only runs of non-astra solutions and no `judge` entry, so the actual judge outcome is unknown to the repo.

### Inferences
- Validity is worth about 27 points (9/33 tests) and was solved by v2's SAT. After that, quality at sizes 25-50 is worth another 10-15 points and depends on LNS speed and on not spending the budget on the first SAT solve.
- A reasonable estimate (not measured) is that a 2-3 s deadline would improve v2-v5 on the large tests more than it costs in speed; see the TEND experiment in Q3 for v2 only. I did not run TEND on v4/v5.

### Gaps
- I did not test v3-v5 with larger TEND, nor on the astra `tests_astra` suite (only v1 and v2 final).
- I did not check whether the 50x50 `d100` hidden tests resemble `gen.py`'s tests (8% pearls) or the denser astra tests (`gen_astra.py`), where the SAT start is slow.

---

## Q5. Final status of sol_v2 / the SAT+LNS approach

### Takeaway
- sol_v2 (SA to 0.3 s, SAT only if needed, LNS to 0.9 s) is valid on all 33 local tests and finishes in about 0.92 s, but its quality is erratic on the largest tests. Averages 76.7-87.0 per run (mean about 82).
- v3 to v5 keep the same architecture and are measurably better and more stable on large tests (88.6, 91.6/92.0, 93.7 in single runs).

### Cited Findings
- Final committed sol_v2: `e4648cf` "p4: sol_v2 (SAT+SA+LNS), all valid" (single-file, 1184 lines). Repo runs in `scores.jsonl`: 87.0 avg (run 3) and 82.5 (run 4), both 0 invalid, max T 0.91 and 0.94 s.
- v2 per-size-group scores (my 4 reruns plus the two repo runs): tests 0-16 average about 95.5-98.2; tests 17-24 about 71-82; tests 25-32 about 41-68. The weakest tests are the large ones: 30_50x50_d50 (score 28-57), 31_50x50_d20 (16-81), 26_40x40_d50 (28-41), 27_40x40_d20 (10-100), 29_50x50_d100 (38-71).
- Large dense: tests 25, 26, 29, 30 have L 1.3-2.5x Lref. Large sparse: 32_50x50_d5 L=114-206 and 28_40x40_d5 L=58-76, where SA success within the 0.3 s window decides the outcome (valid SA gives about 116-134, SAT fallback gives about 206).
- Compared with v2, my single runs of v3/v4/v5 (33 tests): 88.6 / 91.6 / 93.7. The repo's own v4 run: 92.0. Per size group avg 25-32: v2 41-68, v3 54.5-65.9, v4 84.8, v5 90.4. Extra tests 40-47 (41 tests in total): my averages are v2 82.4, v3 87.9, v4 92.1, v5 93.9.
- For comparison (same Lref, `scores_astra.jsonl`): astra v5 87.7, astra v6 92.7, astra v7 91.3 avg33. So non-astra v4/v5 are in the same band as astra v6/v7 on these 33 tests.

### Inferences
- The SAT+LNS idea is sound (validity, quality) but v2's quality is limited by (a) the late first SAT solve on big dense grids, (b) a short LNS budget, and (c) 0.3 s of unconditional SA. v4's SA+repair and v5's restarts address (a) and (c) and give the largest gains on big grids.
- Estimate (unmeasured): the remaining gap to Lref on 25-32 tests is mostly LNS time. Extending to about 2-4 s paid off for v2 in my single-run experiment.

### Gaps
- No stable multi-run statistics for v3-v5: I have one run each plus one repo run for v4. The noise level (about +/-5 points) is comparable to v4 vs v5.
- The final status of v5 may still change; `sol_v5.cpp` was modified at 19:27:28 and committed at 19:28:00, and the repo was still being edited when I stopped.

---

## Appendix: reproduction and raw data

- Raw per-run JSON and logs: `/tmp/claude-1000/-home-server-coding-competitions/7aa75855-1aa5-43ba-9bc6-4dfd2d2f50cb/scratchpad/na/res_*.json` (files `res_v1`, `res_v2_b61`, `res_v2_9ee`, `res_v2_final`, `res_repA1`, `res_rep2`, `res_rep3`, `res_s2x`, `res_s3`, `res_s4`, `res_s5`, `res_sat_only`, `res_dfs_only`, `res_sa_only`). Runner: `na/run.py`; table builders: `na/table.py`, `na/final_table.md`. Source snapshots: `scratchpad/snap_v3..v5.cpp`, `scratchpad/sol_v2_final.cpp`, `scratchpad/src_b61/`, `scratchpad/src_9ee/`.
- Pure SAT `MODE=sat` details (v2 final, mine; L / T / conflicts): 26_40x40_d50 972 / 0.43 s / 13,095; 30_50x50_d50 1066 / 1.14 s / 22,881; 29_50x50_d100 902 / 0.40 s / 9,137; 25_40x40_d100 1018 / 0.11 s / 7,086; small tests under 0.03 s. Throughput is about 20-32k conflicts/s.
- SA throughput: 3.5-4.5M face-flip iterations in 0.85 s on 6 tests of different sizes (instrumented v1 copy, load 17).
- Repo's own scoreboard (`p4/scores.jsonl`): run 1 (v1, t=...531): 9 invalid; run 2 (v1): 9 invalid; runs 3-4 (v2): 0 invalid; run 5 (v4): 0 invalid. No judge scores.
- Extra tests 40-47 (`gen.py:extra`) are not in the 33-test scope; the L table is for 33 tests. Extra-test results for v2 to v5 are in Q5.
- I wrote nothing inside the repo except this notes file.
