# Pairing a CDCL loop finder with fast local shortening

**Verdict.** The highest-scoring design is a single-file C++17 hybrid with three stages. (1) A hand-written CDCL SAT solver on edge variables finds a valid loop. It uses lazy or in-search subtour cuts, hedged restarts, and a "pearl-bearing component wins" rule. (2) A cheap O(1)-per-move face-flip shortener shrinks that loop, followed by SAT-window large neighbourhood search (LNS) with native cardinality and plateau moves. (3) A density dispatcher sends sparse puzzles (few pearls) through penalised face-flip SA from the border rectangle, which beat the SAT-first path there. The time policy is a wall-clock deadline of about 0.9 s, never stopping early while length is above a lower bound. It runs on to about 2-3 s only when the loop is provably far from the bound and still improving, and it never prints an invalid grid.

Validity is worth far more than anything else. Every first-generation solver lost 27-36 points per test to invalid output or timeouts, and the fixes took averages from about 38-62 to about 87-95. After that, length on the roughly 15-20 largest tests is the only large remaining loss (about 7 points per test over 48 tests for the best single pipeline). Speed costs under 1 point, because T <= 1 s is free. Literature gives no better off-the-shelf method. Exact DP is infeasible at 50x50, and no published result covers shortest-loop Masyu with a hand-written solver. The repo's own measurements are therefore the main evidence. The best single pipelines score about 92-95 on local tests against best-known Lref. The two best pipelines are complementary: the per-test maximum of v4 and astra v6 is 96.8, versus about 91.5 for either alone. That gap is the main prize and is an estimate for a combined solver, not a measurement.

Caveats for the whole document:
- Lref values come from the repo's own best runs and my CP-SAT runs. They are upper bounds on the true optimum on large tests.
- Timings were measured under load average 14-19 on a 12-core machine. Wall-clock-limited solvers move 10-25% in length with load.
- Where I write "(est.)" the number is derived or modelled, not measured.

## Scoring maths: the first second is free, validity beats everything, length is squared

`score = 100 * min(1, s*Lref/L)^2 * (0.5 + 0.5*exp(-max(0, T-1)/5))`. Three consequences drive the design.

**Time is free up to 1 s, then costs about 9% per extra second at first.** The speed factor is 1.000 for T <= 1, 0.952 at 1.5 s, 0.909 at 2 s, 0.835 at 3 s, 0.725 at 5 s and 0.583 at 10 s. A timeout is 0, so there is no point running to 9.9 s.

**An extra second must buy a shorter loop by a threshold fraction.** Because quality is squared, an extra second at runtime T pays only if L shrinks by the percentage below. This is the exact solution of (1-d)^-2 = speed(T)/speed(T+dT).

| Already at T | +1 s needs L shorter by | +0.5 s needs | +0.1 s needs |
|---|---|---|---|
| 1 s | 4.64% | 2.41% | 0.50% |
| 2 s | 4.17% | 2.17% | 0.45% |
| 3 s | 3.71% | 1.93% | 0.40% |
| 5 s | 2.85% | 1.49% | 0.31% |

Cumulatively, finishing at 2 s instead of 1 s needs L shorter by 4.6%. Finishing at 3 s needs 8.6%, at 5 s needs 14.9%, and at 10 s needs 23.7%. A 1% shorter loop is worth about 2% of the score, which is about 0.2 s of runtime at T = 1.

**A modelled optimal stopping rule gives an operational policy.** Model the excess over optimum as e1*exp(-(T-1)/tau). Stopping at 1 s is optimal when e1 <= 5%, whatever tau is. Extending pays only when excess at 1 s is >= 10-20% and decays fast. For e1 = 20% the optimum is 2.1-2.5 s (+10 to +17 points) for tau of 0.5-1 s. For e1 = 40% it is 2.5-4.5 s. This model is estimated, not measured.

**Validity first.** An invalid output scores 0. A valid loop 100% too long still scores 25 x speed. A valid loop at T = 9 s scores at least 100*q*0.60. The slow path must therefore run to about 8.5 s rather than give up. The repo's own `sol_v2` runs show why: mean 87.0 with all valid, versus 27.3 points lost per test to invalid output in `sol_v1`.

**The cap makes the hidden score ceiling different from local.** On the "few large" tests with s = 0.9, matching Lref scores 81, and 100 needs L <= 0.9*Lref. If the organisers' best-found there is weaker than ours, we gain. On proven-optimal tests the cap is reaching Lref exactly. Note the repo's best-known Lref is not tight on large tests: CP-SAT (40 s, loaded machine) beat the repo best on `21_30x40_d100` (360 vs 370), `35x48_s109_d100_astra` (1354 vs 1362), `50x6_s103_d3_astra` (96 vs 98), `20x20_s107_d100_astra` (352 vs 358, proven) and `20x20_s107_d25_astra` (152 vs 160, proven). On tests >= 30x40 the best-known moved 5-22% between solver generations (`27_40x40_d20` 262 to 204, `30_50x50_d50` 618 to 488). Treat any local score on large tests as optimistic.

## What worked, what failed, with the reason

The test set has 33 generated tests (`gen.py`) and 15 harder tests (`gen_astra.py`) in the repo. Scores below are per-run averages against the best-known Lref available to each note-taker. Absolute values differ between notes because the Lref basis differs (see "Where the notes disagree").

| Method (repo version) | Measured result | Why |
|---|---|---|
| Penalised SA on face flips from the border rectangle, 0.6 s (`sol_v1.cpp`) | Avg about 60 on 33 tests; invalid on all dense tests above 20x15 d100 (9/33 invalid); about 3.5-4.5M flips/s | Violation penalty W=20 never reaches 0 violations on dense boards in 0.6 s. Works on every sparse test and is the best sparse-board method found (50x50 d5: L=114-120 vs 178 for astra v6). |
| Propagation DFS with failed-literal probing (`sol_v1.cpp` DFS, astra v1) | Astra v1 38.6 avg, 12/33 fail; `sol_v1` DFS needs about 0.1 s per node on 50x50 and finds loops only up to 20x15 d100 | No clause learning. Global single-loop failure is found only deep in the tree. Probing costs up to 4*E propagation passes per node. Per-node rate falls as about 1/N (about 29k nodes/s at N=2500, and 1.5M nodes with no loop on 13_20x15_d100). |
| CDCL on edge variables with lazy subtour cuts (non-astra v2, astra v2) | Pure SAT valid on all 33 tests in 0.01-0.43 s (one 1.14 s, 50x50 d50); astra v2 77.8 avg with 1 fail | Clause learning jumps over the sub-loop dead ends that kill DFS. Only 0-3 lazy re-solves are needed once the in-search hook handles most subtours. Loops returned are 2-6x too long (no objective), so shortening is mandatory. |
| Boundary cuts "an edge leaving the cycle must be on" added after each model (astra v3) | Net negative: avg 70.8 vs 77.8, 4 fails vs 1 | Hundreds of literals per clause for big components. Hypothesis only; no clean ablation exists. |
| In-search cycle theory callback on or off (astra v4) | Heavy-tailed: 13_20x15_d100 0.027 s off vs 1.035 s on; 26_40x40_d50 0.98 s off vs 0.018 s on; 29_50x50_d100 0.74 s off vs 0.071 s on | No single setting dominates. Hedge with restarts that alternate theory and variable order; theory on only for N>500 first (astra v5). Fixed 0 fails on 48 tests. |
| Windowed DFS "LNS" with 4-12 windows, 1200 nodes (astra v2-v5) | Left 1030 on `50x50_s113_d3` where face flips reach 378 | Windows can fix local detours only; cannot shrink long slack within 0.12 s. |
| Face-flip descent (4 corners XOR, uphill 0.2%) on the SAT loop (astra v6) | +8.65 avg points over v5 (86.5 to 95.1, 48 tests); examples: `50x50_s113_d3` 1386 to 378 in 22 ms, 9.2 to 83.2 points | Each move is O(1) and removes slack in the long arbitrary SAT loop. It cannot re-route between pearls, so it plateaus (`32_50x50_d5` 178 vs 114-126 best). |
| SA-then-local-SAT repair of violated pearls (non-astra v4) | v3 88.6, v4 91.6-92.0, v5 93.7 (single run, 33 tests); avg on tests 25-32 rose from 54.5 to 84.8 | Removes the late "global SAT to find any loop" on big dense grids. Windows around only the violated pearls avoid solving the whole board. |
| SAT-window LNS with native at-most-K cardinality (non-astra v3) | 2.5-6x more LNS steps per second than the Sinz counter (e.g. 4187 vs 695 on the sample, 554 vs 173 on 30x40 d100) | Sinz counter adds (n-1)*K auxiliary variables per step and rebuilds the CNF each time. |
| Restarts plus plateau moves plus global window on tiny boards (non-astra v5) | +2 points avg at 33 tests (93.7); best on tests 25-32 (90.4) | Plateau moves (equal-length alternative loops) escape local optima. |
| Running to 2 s / 4 s / 8 s on non-astra v2, 7 large tests | Score 50.8 at 0.9 s, 60.4 at 2 s, 63.2 at 4 s, 51.4 at 8 s (quality alone 50.9 / 66.6 / 81.9 / 82.6) | Longer pays up to about 2-4 s only when the loop is far from converged; past that the speed penalty dominates. Single noisy runs. |
| Early stop after 250 failed windows / stale counter (astra v6) | `14_20x15_d50`: stopped at 0.29 s with L=90 vs optimum 74 (32 points lost); `16_20x15_d5`: 40 vs 36 at 0.26 s | Wasted free seconds: 0.6-0.9 s were available. |

Full-grid frontier DP is rejected on counts. Plug-DP state counts grow about 2.65-3x per extra column (Motzkin M(W+1), with the flagged version about 0.21*3^W), measured up to width 12. At width 50 that is about 10^22 states, so it is impossible regardless of language. A window DP is the only DP-style method with a feasible budget (est.). At about 30M transitions/s, a window of 8 rows x 50 columns costs about 0.05 s, 9 rows about 0.18 s, and 10 rows about 0.6 s. Nobody in the repo implemented it, so it remains untested. Generic ILP and CP-SAT cannot be used from standard-library C++, and on large tests they were weaker than the repo's heuristics in the measured run. CP-SAT incumbents were 3-60% worse than the repo best on the large sparse tests (`24_30x40_d5` 98 vs 70, `32_50x50_d5` 162 vs 114, `35x48_s109_d3` 430 vs 182). It is useful only as an offline checker.

Literature offers no stronger option for this objective. The best published Masyu SAT result, a distance (level) encoding solved with Kissat, takes 1.6 s for 30x30 and 6.2 s for 40x40 including translation ([Zhou, Picat reachability constraints](https://ar5iv.labs.arxiv.org/html/2109.08293)), and its instances are uniquely solvable contest puzzles with no length objective. Lazy cycle-blocking is the common practitioner approach and "usually works well" but is exponential in crafted cases ([Hakerh400, Slitherlink via SAT](https://esolangs.org/wiki/User:Hakerh400/How_to_solve_Slitherlink_using_SAT_solver)). On 101x101 Slitherlink it needed an average of 120 re-solve iterations ([Optilog tuning](https://ulog.udl.cat/static/doc/optilog/html/optilog/use-cases/slitherlink/tuning-slitherlink.html)). Masyu is NP-complete ([Friedman](https://mountainvistasoft.com/docs/pearl-puzzles-are-np-complete.htm)), so no exact method is expected to scale. The repo's in-search hook already needs only 0-3 lazy iterations on its tests, which is evidence that a distance encoding is not worth its extra 30k auxiliary variables here. That comparison is inferred, not measured.

## The recommended pipeline, step by step

Language: C++17, single file, standard library only. Python is out because the method's quality depends on iterations per second. The repo's SA does 3.5-4.5M flips/s and its CDCL does 20-32k conflicts/s in C++. I did not measure Python, but the face-flip loop and propagation would be one to two orders of magnitude slower (est.). The repo's own `AGENTS.md` says to move to C++17 once the method is a search whose quality depends on iterations per second.

**Step 0. Parse and bound.** Build edge indexing (E = R(C-1)+(R-1)C, 4900 at 50x50) and pearl configuration lists. Compute LB = max(4, 2*(row span + col span of pearls), exact Held-Karp Manhattan TSP over pearls when <= 15 pearls, #pearls rounded up to even). Why: it is cheap and gives the stop certificate. For sparse puzzles (<= 15 pearls) the best known is within 1.00-1.17 of the TSP/bbox bound (median 1.14; 1.04-1.09 on the large sparse tests), so stopping at L <= 1.03*LB is justified: at that gap 1 more second cannot recoup the 4.6% threshold. For dense puzzles the bound is weak (best/bound 1.2-2.05), so do not use it to stop there.

**Step 1. Dispatch on pearl density.** Alternatives: SAT first for everything (the astra family), SA first for everything (the non-astra family). The measured complement says use both.
- Sparse (about <= 15 pearls or pearl density about <= 1-2% of cells; thresholds are an estimate to tune): penalised face-flip SA from the border rectangle for about 0.15-0.3 s. It reached 114-120 on `32_50x50_d5` (Lref 114) and 58 on `28_40x40_d5` (exact bound 58), where astra v6 reached 178 and 58. If SA has not reached zero violations by then, go to Step 2.
- Everything else: go to Step 2 immediately. SA wastes 0.3 s on dense boards, because it never reaches zero violations there. This is non-astra v2's measured SA-first weakness: the unconditional 0.3 s SA found a valid loop only on sparse boards, so on dense boards it was pure overhead before SAT started.

**Step 2. CDCL SAT to find any valid loop.** Edge variables, per-cell degree clauses (degree 0 or 2, 8 clauses per cell), 3-9 clauses per pearl, 2-watched literals, VSIDS with phase saving, Luby or geometric restarts, learnt-clause database reduction (non-astra v2 reduces at 4000 learnts; astra v2 keeps all, with no memory issue within 7 s). Add the cycle rule: a closed cycle that does not contain all pearls is a conflict, with the clause "some edge leaving this cell set must be on" (non-astra v2 `cycleCheck` hook; handles almost all subtour work with 0-3 lazy re-solves). Accept any component containing all pearls and drop pearl-free extra cycles. Why: SAT solved 32 of 33 (astra v2) or all 33 (non-astra v2, pure SAT in 0.01-0.43 s except 50x50 d50 at 1.14 s) where DFS solved about 21 of 33. First-valid-loop times observed: 0.011-0.1 s on most tests, 0.64 s on `30_50x50_d50`, 0.75 s on `50x50_s113_d3_astra` after 23 cut rounds, and about 80 cut rounds on the 840-pearl `50x50_s113_d100_astra`.

The heavy-tail risk matters. The in-search theory ablation shows 1-second vs 0.02-second swings in both directions per instance. Therefore:
- Do not fix one SAT setting. Run a short first call (about 0.3 s, theory on if N > 500), then restarts with a shuffled edge order and alternating theory, with slices growing from 0.6 s (astra v5 recipe; 0 fails on 48 tests).
- Keep learned clauses across restarts where possible. Astra v6 discards them, which cost five wasted 0.6-1.0 s attempts on the 840-pearl test in a loaded run. This is a known weakness, not a measured fix.
- Seed the SAT phase hint with the SA's final state (non-astra v2, weak evidence).
- On the non-astra side, instead of a global solve, repair the SA's violated pearls with local SAT windows (non-astra v4, tested: faster than global SAT on 40x40-50x50).

**Step 3. Shorten with face flips.** Operate on a cell-shape array. Pick a random 2x2 face, skip masks that could split the loop, toggle the four corners, check pearl legality locally, accept downhill moves always, uphill about 0.2%, reset to incumbent if drift exceeds best+6, and validate fully only on new bests. Why: v5 to v6 gain of +8.65 points came from exactly this, and each move is O(1) (the SA's 3.5-4.5M flips/s). Take it from the SAT loop. A proper temperature schedule was never tested in the astra family, so that is the cheapest unexplored improvement; the non-astra SA uses a geometric schedule 3.0 to 0.3 and works on sparse boards.

**Step 4. SAT-window LNS.** Free all edges in a window around a random loop edge (3-8 growing to 14 cells), fix everything else to the current loop with unit clauses, bound the free ON edges to strictly fewer than the current count with a native cardinality constraint (non-astra v3), 500-conflict limit, phases set to the current solution. Plateau mode for about 20% of steps: accept an equal-length different loop (non-astra v5). Use a global window every 10th step when E < 600. Why: face flips plateau, and windows catch what they miss on mid-size boards (astra v6 lost points on `18_25x25_d50` 130 vs 116, `14_20x15_d50` 90 vs 74 because windows had no time left). Observed cost: about 1 ms per step with native cardinality; 50-95% of windows come back UNSAT (a local-optimality proof, but wasted time), e.g. 670 UNSAT of 809 tried on `21_30x40_d100`.

Open improvement, an estimate and untested: reuse the face-flip phase between LNS rounds, and alternate rather than run them once. The astra v6 order was SAT, face, LNS, face. Non-astra v5 ran SA, repair, SAT, LNS and was roughly equal on a shared test set (v4 91.48 vs astra v6 91.47 on the 32 shared non-sample tests). They differ by class: dense (d100) v4 84.6 vs v6 97.2; mid v4 92.1 vs v6 89.2; sparse v4 97.1 vs v6 90.3. The per-test maximum is 96.83. That is the quantitative reason for combining them.

**Step 5 (optional if time remains in the 2-hour contest). Window DP.** An exact min-length DP over a strip of at most 8-9 rows by 50 columns (est. about 0.05-0.2 s per window) would remove the plateau that both face flips and 1200-node DFS hit on sparse large boards (`32_50x50_d5` 178 vs 114; `50x50_s113_d3` 342 vs 312). It is the only unexplored method with a feasible cost model. Everything above it has measured evidence; this has only a state-count model and about 300-500 lines of implementation (est.). Do it last, and only if the SA dispatch does not already reach about 100% on sparse boards.

**Final stage: validate before printing.** Run an independent full-validity check (reciprocal edges, pearl rules, single cycle containing all pearls) before storing any new best, as all astra versions did. The best valid loop is printed. If nothing valid exists after the slow path, the last fallback must still try SAT without the bounding-box margin. Printing the border rectangle is an invalid answer whenever any pearl is interior, and both `sol_v1.cpp` and `sol_v2.cpp` do exactly that.

## Time policy

Read wall clock inside the SA, SAT and LNS loops (not every 64 DFS nodes, which overshot 10 s at 10.2-11.8 s under load).

| Situation | Policy | Reason |
|---|---|---|
| Normal | Deadline 0.9 s wall, whole process | The first second is free; observed finishing times 0.77-0.96 s leave 0.04-0.1 s for a slower judge. Overshoot costs 2.9% at 1.3 s and 9.1% at 2 s. |
| L <= about 1.03 x LB on sparse boards | Stop immediately | 4.6% per extra second threshold exceeds any remaining gain. |
| Never stop early on a "stale counter" while L is above LB and T < 0.9 s | Use the budget | Astra v6 stopped 14_20x15_d50 at 0.29 s, 21.6% above optimum, and lost 32 points. Small boards (6x6 to 12x12) reach optimum in under 0.35 s in every version and can stop earlier. |
| No valid loop yet | Continue to about 8.5 s | Any valid loop at 8.5 s scores at least 100*q*0.62 vs 0. |
| At 0.9 s: L >= 1.2 x LB (dense/uncertain) or first loop arrived late and LNS is still dropping fast | Extend to 2-3 s | Break-even is 4.6% shorter at 2 s and 8.6% at 3 s. Measured: non-astra v2 on 7 large tests gained 50.8 to 60.4 at 2 s and 63.2 at 4 s. A drop rate above about 5%/s at T = 1 and 4%/s at T = 3 justifies continuing. |

The extension rule is an estimate. It is not tested on the repo's best versions and the repo found it inconsistent: the non-astra v4/v5 TEND experiment was never run. If unsure, stay at 0.9 s: slowness cost only 0.24 points per test for astra v6, and extending can cost up to 9% at 2 s.

## Expected score and remaining risk

**Measured on local tests (Lref = best known).** Astra v6 averages 92.7-95.1 on 48 tests (depends on Lref basis); non-astra v4 91.7 (33 tests) and v5 93.7 (single run, 33 tests, about +/-5 run-to-run noise); the per-test maximum of v4 and astra v6 is 96.8. The recommended pipeline combines both and adds early-stop removal. The expected local score is about 93-96 (est.). Small tests (<= 150 cells) already score 99.3 for every recent solution.

**Estimated hidden score.** If the 100 hidden tests are spread evenly by side length, about 54% have more than 700 cells, and those carry about 75-90% of the lost points. The current best single pipeline would give about 89-91 under that weighting; the combined design should add a few points, giving about 90-94 (est., not measured). Three risks apply:
- The organisers' Lref may be a stronger exact result on mid-size tests, which lowers every score.
- The s = 0.9 cases raise scores (up to 19 points) if their best-found is weak.
- The judge CPU speed is unknown. Since wall-clock deadlines make length vary 10-25%, losses move with it, and the 0.9 s guard protects only time, not quality.

**Largest known holes (candidates for remaining time).**
- Sparse large boards: face flips plateau far above the bound (178 vs 114-126 on 50x50 d5). The SA dispatch handles part of this, and window DP is the untested fix.
- Dense 50x50 first loop: 840 pearls took 2.3 s (loaded: up to 4.5 s) at about 80 cut rounds; keeping learned clauses across restarts is the unmeasured fix.
- First-loop heavy tails: alternating theory and shuffled order is the mitigation.
- Hidden density: the generators differ (pearls 0.3-22% of cells in `gen.py`, 0.8-38% in `gen_astra.py`), and the real set is unknown.

## Where the notes disagree, and how it reconciles

- **Average of astra v6.** `repo_astra.md` reports 95.1 on 48 tests; `quantitative.md` reports 92.7. Both are the same runs. The difference is the Lref basis: the first uses best-known from repo runs only (a snapshot including astra v7), the second adds later runs and CP-SAT incumbents, which lowers some Lrefs and raises the bar. Use about 93 as the working number.
- **Non-astra v2 final.** `repo_non_astra.md` finds a mean of about 82 over seven runs (range 76.7-87.0), while `repo_astra.md` quotes 88.92 for `sol_v2.cpp` from the repo's scoreboard. The latter is one run against a different Lref set. Use 82 (range 77-87).
- **Lref for individual tests differs by note** (`32_50x50_d5`: 126 in the astra notes, 114 in the non-astra and CP-SAT-aware notes; `21_30x40_d100`: 370, 362, 360). Quality percentages for those tests therefore differ by note. I use the lowest known value as "best known".
- **Time-limit usage.** `quantitative.md` finds that early termination is a pure waste and the 1 s budget is free; `repo_non_astra.md` finds that running 2-4 s helps only when the solver is far from converged. These are consistent: extend only conditionally, never stop early while above the bound.
- **SAT first-loop time on 50x50 dense.** Non-astra v2 pure SAT: 0.40 s on `29_50x50_d100`. Astra v6 with theory: 0.058-0.071 s. Same instance, different encoding details and different load; both are under 1 s, so no conflict, but the tail on 840-pearl astra cases (2.3-4.5 s) is a different regime.
- **The astra plan document's claim that a specialised DFS beats generic SAT** was contradicted by measurement: CDCL solved 11 of 12 DFS failures and doubled the mean (77.8 vs 38.6). That document explicitly hedged ("no universal dominance asserted"), which the data bore out.
- **The note-takers' timings and ablations were produced under heavy load,** so absolute runtimes are pessimistic and run-to-run variation (10-25% in L on d20 boards) is large. Differences of 1-3 points between adjacent versions are within noise; the large steps are not.

## Conclusion

Three results are not obvious from the version history alone. First, the tail risk lies in the first valid loop, not in the average: theory on or off flips a 0.02 s solve into a 1 s one on specific instances, so the recommendation is hedged restarts rather than a tuned single setting. Second, the best single pipelines are complementary by density class (dense: SAT-first wins by 12.6 points; sparse: SA-first wins by 6.8), which is why a density dispatch plus both shorteners is worth about 5 points over either, more than any parameter tuning. Third, time is almost entirely a non-issue: slowness costs under 1 point per test while early termination cost up to 32 points on a single test.

The right priorities for the 2-hour contest follow. Get a valid answer everywhere with CDCL plus restarts and a validated fallback. Then run the face-flip shortener and the SAT-window LNS. Then add the sparse-board SA path. Add the window DP last, and only if the sparse boards are still far from the bound. Until it is measured, treat any estimate in this document above the local scoreboard figures as unproven.
