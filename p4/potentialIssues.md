# P4 Masyu: potential issues and edge cases

Nothing here is implemented. This is a checklist of what could go wrong, grouped by where it bites. Each item says what happens, why, and what to test or guard against.

Sources: `statement.md`, `notes.md`, `notes_astra.md`, `AGENTS.md`, `fastestImplementationReasonAstra`, and the literature notes (Picat/Kissat Masyu timings, lazy sub-cycle blocking, Motzkin growth of frontier DP). Items marked **[repo]** come from reading repo documents. Items marked **[unverified]** are my reasoning, with no measurement behind them yet.

Severity key: **P0** = score 0 on a test, **P1** = large loss, **P2** = small loss or cosmetic.

---

## 1. Score-zero hazards (validity and I/O)

| # | Issue | Why it happens | Guard |
|---|---|---|---|
| 1.1 **P0** | **Invalid fallback output.** If no valid loop is found, a "print something anyway" path emits a loop that breaks pearl rules. | Invalid scores 0, the same as printing nothing, so a bad fallback gains nothing and hides the failure from local tests. **[repo]** The astra doc says the baseline's fallback is a start rectangle that is not generally valid. | Treat "no loop found" as a first-class state. Keep the checker in the loop on every local run and flag fallbacks explicitly. |
| 1.2 **P0** | **Hard timeout.** The judge limit is 10 s and a timeout scores 0. | A search with no deadline check inside the inner loop, such as one long propagation pass or one SAT restart. A slower judge CPU stretches everything. | Check the clock inside every inner loop, not only between phases. Budget against roughly 7 s worst case, not 10 s. Test on a throttled core. |
| 1.3 **P0** | **Truncated `main()` in the statement.** The template stops at `int main()`. | The I/O contract is partly guessed. **[repo]** `notes.md` assumes HackerRank style: a first line giving the row count, then the rows, with output written to `OUTPUT_PATH`. | Write output to `$OUTPUT_PATH` if set, otherwise stdout. Never print debug text to stdout. |
| 1.4 **P0** | **Input format mismatch.** The statement says "R, followed by R rows", but the template has `grid_count`. Possible variants are: R only, R and C, rows with trailing spaces or `\r`, a blank trailing line. | HackerRank harnesses vary. | Read tokens, not lines. Take R from the first token, then read R non-empty tokens. Strip `\r`. Derive C from the first row. Validate that all rows have equal length. |
| 1.5 **P0** | **Wrong glyph convention.** `L` = N+E, `J` = N+W, `F` = S+E, `7` = S+W. A swapped pair gives a loop that looks right and scores 0. | The usual Unicode box-drawing habit is easy to mix up. | Replay the statement's sample (18 cells) through the checker. Add a self-check that every emitted cell's connections are reciprocated by its neighbours. |
| 1.6 **P0** | **Output dimensions or stray characters.** An extra trailing blank line, missing row, `.` vs space, or a debug print on stdout. | Judges compare strictly. | Print exactly R lines of length C. Send all diagnostics to stderr, or remove them at submission. |
| 1.7 **P0** | **Undefined behaviour in C++.** Out-of-bounds neighbour reads at the border, uninitialised arrays, stack overflow from deep recursive DFS or union-find on 2,500 cells. | Border cells have fewer than 4 neighbours, and recursion depth is up to about 2,500 frames times frame size. | Pad the grid with a sentinel border. Use iterative loops for DFS and flood fill. Compile local tests with `-fsanitize=address,undefined`. |
| 1.8 **P0** | **Compiler or standard mismatch on the judge.** `-O2` vs `-O0`, an older GCC, no `<bits/stdc++.h>`, no `__builtin_*`, or a C template where C++17 is expected. | The judge environment is unknown. The statement shows a C template. | Single file, standard headers only. Avoid compiler-specific intrinsics unless guarded. Check that it still compiles with `-std=c++17` and the older `-std=c++14`. A speed plan that assumes `-O2` is **[unverified]** against the judge. |
| 1.9 **P0** | **Memory limit.** Not stated. | Learned-clause databases and the clause-per-pearl-neighbourhood encoding can grow without bound in a CDCL. | Cap the learnt-clause database. Reduce it periodically. Keep total memory well under typical 256 MB. |

---

## 2. Correctness of the loop rules

These are where a solver passes local tests and then fails on a hidden case.

| # | Issue | Detail |
|---|---|---|
| 2.1 **P0** | **Multiple loops.** Degree-2 everywhere does not give one loop. A solver may emit two or more disjoint cycles. | The statement says it directly: "broken or multiple loops" score 0. A search that never checks sub-loops wastes most of its time. |
| 2.2 **P0** | **Premature closure.** A cycle closes before covering all pearls. | The branch is dead: a cycle has degree 2 everywhere, so nothing can attach later. Needs an explicit "cycle must contain every pearl and every selected edge" check. |
| 2.3 **P0** | **White pearl rule misread.** White: straight through the pearl AND at least one of the two adjacent loop cells turns. Mistaking this for "both neighbours turn" removes valid solutions, and "neither needs to turn" admits invalid ones. | Encode the "at least one" as a disjunction conditional on orientation. Test on three whites in a row and on a white at a border. |
| 2.4 **P0** | **Black pearl rule misread.** Black: turns in the pearl cell AND both adjacent loop cells go straight. The straight requirement concerns the neighbour's shape, not the neighbour being pearl-free. | A black pearl beside the border, or beside another black pearl, often has one orientation. A solver that ignores the "straight neighbours" half accepts invalid loops. |
| 2.5 **P0** | **Adjacent pearl interactions.** A black pearl whose straight neighbour is a white pearl forces that white pearl's orientation (it is collinear with the black arm, so the white pearl runs along the arm). Chains of whites in a row force perpendicular crossing. | Easy to mishandle when propagating only per-cell. Cross-check propagation against brute force on tiny grids (see section 8). |
| 2.6 **P1** | **"Adjacent cells along the loop" vs "adjacent on the grid".** The turn conditions refer to neighbours along the loop (the two cells the loop visits immediately before and after), not all four grid neighbours. | Cells that are next to each other on the grid but not connected, such as the statement's "loop cells may sit next to each other without being connected," must not count. |
| 2.7 **P1** | **Pearls on the border.** A white pearl in a corner is impossible (no straight line fits). A white pearl on a border cell must run along the border. A black pearl in a corner is forced. | The statement guarantees at least one valid loop, so corner whites will not appear, but the solver must still not crash on border pearls. |
| 2.8 **P1** | **Corner/degree bookkeeping on non-pearl cells.** Non-pearl cells may be skipped or visited freely, and may be corners or straights. Over-constraining them, for example requiring non-pearl cells to be straight, loses solutions. | Keep non-pearl domains as the full 7 shapes minus out-of-grid ones. |
| 2.9 **P2** | **Minimum loop shape.** The smallest loop is a 2x2 block of 4 cells, and only when its constraints allow it. A solver should not assume loops longer than that, and must not produce 2-cell "loops" (degenerate back-and-forth). | Check that the cycle length is at least 4 and that the loop never reuses an edge. |
| 2.10 **P1** | **Touching loops.** The statement allows loop cells to be adjacent without connecting. A checker or solver that treats adjacency as connection rejects or mis-merges valid loops. | Connectivity must follow the shapes' edges, not grid adjacency. |

---

## 3. Algorithmic risks

| # | Issue | Why it happens | Mitigation to consider |
|---|---|---|---|
| 3.1 **P1** | **Dense large grids: valid loop is hard to find.** The statement warns: "finding a valid loop at all (hard on densely pearled puzzles)." **[repo]** Both the first baseline and early astra versions timed out (about 7-8 s, no output) on several 20x15 to 50x50 dense cases. | Masyu is NP-complete (Friedman). Plain DFS plus propagation hits exponential regions. | Learning (CDCL) and restarts rather than chronological DFS. Strong early propagation. |
| 3.2 **P1** | **Sparse large grids: the first loop found is far too long.** **[repo]** Early astra results show L around 360 on 25x25 sparse puzzles where far shorter loops exist. | A valid-first search happily wanders. Quality is squared, so a loop 25% too long keeps only 64%. | Separate "find valid" from "shorten". Shorten with local moves or windowed re-solves. |
| 3.3 **P1** | **Lazy sub-cycle blocking can need many rounds.** Literature: usually fine, but can need exponentially many solver calls on crafted instances. A Slitherlink experiment averaged about 120 blocking iterations on random 101x101 instances. | Each sub-cycle found adds a clause that forbids only that one cycle. | Add cuts for the cycle's cut-set, not just the cycle's edges (stronger). Cap rounds and keep a fallback. Watch iteration counts per test. |
| 3.4 **P1** | **Level/distance encoding blows up the clause count.** The published best Masyu SAT result uses distance encoding, but it adds bit-vector adders per cell. | Roughly O(N log N) variables at N = 2,500. Fine for an industrial SAT solver, but a hand-written CDCL may be slow. | Estimate variable and clause counts before committing. The 30x30-40x40 timings (1.6-6.2 s with Kissat) are for a real SAT solver, and **[unverified]** whether a hand-written one gets near that. |
| 3.5 **P1** | **Hand-written CDCL is slower than Kissat/CaDiCaL by a large constant factor.** | No inprocessing, simpler heuristics, no preprocessing. | Do not assume the literature timings transfer. Benchmark the hand-written solver on the actual 40x40 and 50x50 tests. |
| 3.6 **P1** | **Frontier DP state explosion.** State count grows roughly like Motzkin numbers, about 3^w for frontier width w. Full-width scanning at w = 50 is infeasible. | Even w = 20 is in the tens of millions of states. | Only use DP in narrow windows, around 8-12 cells in the short dimension, and scan along that. Do not scan along the 50-wide axis. **[unverified]** the exact feasible window height. |
| 3.7 **P1** | **Face-flip local search can get stuck.** A flip must preserve one connected inside region and one connected outside region, and it must also keep every pearl rule. Many flips are illegal near pearls. | The move set is very restricted around dense pearls. | Use it only after a valid loop exists, and combine it with windowed re-solves. Always keep a copy of the best valid loop. |
| 3.8 **P1** | **Local optimum is not global.** Shortening by local moves can plateau 10-30% above Lref. At that level quality is 0.6-0.8. | The statement notes that on a few big cases Lref is only the organisers' best find, with s = 0.9. | Spend remaining time on windowed exact re-solves and restarts, and measure the plateau on the existing tests. |
| 3.9 **P2** | **Non-determinism.** Time-based decisions make runs unrepeatable, so a failure on the judge cannot be reproduced. | Wall-clock-driven restarts or random seeds. | Fixed seed. Log which phase produced the output (stderr). |
| 3.10 **P1** | **Pathological puzzle structures.** Examples: all pearls black and dense; long runs of white pearls; pearls only in two far-apart clusters (the loop must cross the whole grid); a single pearl pair at opposite corners; a pearl-free large region around which the loop must wrap. | Each stresses a different part: propagation, connectivity, or length. | Add these as named generator modes (section 8). |
| 3.11 **P1** | **Pearls that force a very long loop.** If pearls sit at opposite ends, any loop is at least about 2(R + C) long and the minimum is exactly that shape. A search that is biased toward compact loops wastes time on dead ends. | The lower bound is the perimeter of the pearls' bounding box, roughly. | Use the bounding-box bound to recognise when a found loop is already optimal and stop early (saves the speed penalty). |

---

## 4. Time and speed-score traps

The speed multiplier is `0.5 + 0.5 * exp(-max(0, T - 1) / 5)`: free for the first second, then a slow decay, never below 0.5.

| # | Issue | Detail |
|---|---|---|
| 4.1 **P1** | **Fixed deadlines waste points.** **[repo]** An early baseline ran its annealing phase until a fixed 0.85 s on every test, even tiny ones. | At T <= 1 s there is no penalty, so using up to about 1 s is free, but running to 8 s on every test costs about 0.5 + 0.5 e^(-1.4) = 0.62, a 38% loss. Stop as soon as no further gain is likely. |
| 4.2 **P1** | **Early stop on proven optimum.** Many tests have a proven-shortest Lref. If the solver cannot know it has hit it, it keeps spending time. | Use cheap lower bounds to stop early, and accept that most cases cannot be proven. |
| 4.3 **P1** | **Process start-up and I/O time count.** The clock includes reading input, allocating, and printing. Large static tables or `memset` of big arrays at start cost milliseconds. | Negligible against 1 s, but a 100x100x7 table rebuilt every run is not free. Measure total wall time with `/usr/bin/time`, not only the solver's own timer. |
| 4.4 **P1** | **Judge machine slower than this one.** The judge CPU may be 1.5-3x slower, and may be shared across parallel submissions. | Anything tuned to finish at "0.9 s" locally could take 2-3 s there. The cost is gentle (T = 2 s gives 0.91 speed), but a wall of 7 s deadlines becomes 10 s timeouts. Keep a safety margin on the hard cap. |
| 4.5 **P2** | **CPU vs wall clock.** Using `clock()` measures CPU time, which differs from wall time under contention or with threads. | Measure with a monotonic wall clock (`steady_clock`). No threads, since the judge may be single-core. |
| 4.6 **P1** | **Marginal break-even is tiny.** Just past T = 1 s the speed multiplier loses about 10% of its value per extra second (its derivative is -0.1 exp(-(T-1)/5), and it equals 1 at T = 1). Quality is squared, so shortening the loop by x% raises the score by about 2x%. Quality gains must beat that. | Rule of thumb: an extra second is worth it only if it shortens the loop by roughly 5% or more near T = 1. Checked by calculation only, see the quantitative notes for the exact table. |

---

## 5. Test-distribution risks

| # | Issue | Detail |
|---|---|---|
| 5.1 **P1** | **Local tests are not the hidden tests.** The generator builds a loop first and then adds compatible pearls. **[repo]** The astra doc notes the construction length is only an upper bound on the optimum, and the density parameter is the fraction of eligible loop cells, not the fraction of board cells. | Hidden puzzles may be harder (denser or more adversarial), or easier (generated from uniquely solvable Masyu). Do not tune parameters to local densities alone. |
| 5.2 **P1** | **"Increasing size and difficulty" means the hard tests are the high-value ones.** About 100 tests, probably the last third are the large ones. | A solver that is perfect up to 20x20 and times out beyond that loses a large share of the points. Weight local benchmarking toward 30x30 to 50x50. |
| 5.3 **P1** | **Non-square grids.** 6x50 and 50x6 must behave identically. **[repo]** The astra tests already include 6x50 and 50x6. | Bugs from swapping R and C show up only here (for example row-major indexing with the wrong stride). |
| 5.4 **P2** | **Minimum sizes.** 6x6 with 2 pearls: the solver must not rely on a minimum size of 10 or more, and must not crash with only 2 pearls (very sparse). | Include the 2-pearl cases. |
| 5.5 **P1** | **Reference length uncertainty.** On a few large cases Lref is only the organisers' best, with s = 0.9. Beating it by about 10% earns full quality, so sometimes there is headroom, but you cannot tell which tests. | Do not stop early on large cases just because a solution is "good". |
| 5.6 **P2** | **Hidden duplicates of the sample or trivial cases.** The sample is not scored. | No special handling needed. |

---

## 6. Implementation and engineering traps

| # | Issue | Detail |
|---|---|---|
| 6.1 **P1** | **Undo/trail bugs.** Backtracking that fails to restore a domain, queue entry or endpoint link gives a silently corrupted state that then yields invalid output or false "no solution". | Add a debug mode that compares state before and after a decision/undo pair. Assert the final loop with the independent checker logic inside the solver (cheap, O(N)). |
| 6.2 **P1** | **Self-verification missing.** If the solver does not re-validate its answer before printing, any latent bug prints an invalid loop. | Write a compact validator inside the solution, run it on the final answer, and fall back to the best earlier valid answer if it fails. Must be independent of the search's data structures. |
| 6.3 **P1** | **Best-so-far not saved.** Improvements applied in place can leave the state invalid at the deadline. | Keep a separate copy of the best valid grid and only overwrite it after validation. |
| 6.4 **P1** | **Single-file requirement.** Anything that lives in a separate header or file (the repo had `sat_part.h` for a while) will not compile on the judge. | Merge into one file before submitting. A concatenation step can introduce duplicate symbols or name clashes. **[repo]** `sol_v2.cpp` was moved into `dev/` together with its header. Make sure the file that is submitted is the self-contained one. |
| 6.5 **P1** | **Integer overflow and hashing.** Packed state keys or clause activity counters in 32 bits. | Use 64-bit where counts can grow. |
| 6.6 **P2** | **Macro or global name clashes with the template.** The template has `readline`, `ltrim`, `rtrim`, `parse_int` declarations. A solver defining its own functions with the same names conflicts. | Keep template I/O untouched, as AGENTS.md says. Prefix your own helpers. |
| 6.7 **P2** | **Python version.** The judge's Python may be older than 3.8, so only 3.8 syntax is safe. Not an issue for C++. | Only relevant to checker or generator scripts. |
| 6.8 **P1** | **Mixed ownership in the repo.** **[repo]** Files with `_astra` suffix are owned by a separate implementation, and other files belong to the main line. Files are being modified concurrently (new `sol_v6_astra.cpp`, `flips_astra.txt` appeared during this research). | Do not run bulk edits or formatters across p4. Record a version in `scores` only for the exact file submitted. |
| 6.9 **P2** | **Scores log integrity.** `scores.jsonl` must never be edited, per AGENTS.md. | Append only, via the tools. |

---

## 7. Strange input edge cases worth a named test

Each one should be created by hand or by a generator mode and run through the checker.

1. **Two pearls only, adjacent** (for example two blacks side by side, or black next to white). The loop is forced and small.
2. **Two pearls at opposite corners** of a 50x50 grid. Minimum loop is the full bounding rectangle.
3. **Black pearl in every cell of a 6x6.** Unsolvable in general, so the guarantee excludes it, but near-full-black grids stress propagation. Check that the solver does not crash when it proves infeasibility inside a sub-window.
4. **A full border row of whites** (loop must go along the border or cross perpendicular). Three whites in a row force perpendicular crossing.
5. **Checkerboard of blacks.** Many forced orientations at once.
6. **Single long snake path of pearls** (a Hamiltonian-like forced route). Tests sub-cycle handling.
7. **Pearls clustered in one corner of a 50x50.** The loop is small, and the rest of the board must stay empty. A solver that wanders into the empty area produces a long loop.
8. **Pearls on all four borders, none inside.** A big loop around the grid with a pearl-free interior. The shortest loop may not hug the border.
9. **Black pearl next to a border with one legal orientation**, and a white pearl that forces a collinear neighbour into the black pearl's arm.
10. **A pearl that forces a 2x2 loop** (the smallest loop). Output must have exactly 4 non-`.` cells.
11. **Width 6, height 50 and the transpose**, with dense pearls. Checks index strides and any window logic that assumes the short side.
12. **Windows or strips that straddle the border** if any windowed method is used: windows must clip at the grid edge and still produce legal connections to the outside.
13. **Input with Windows line endings** or a trailing space on a row.
14. **Maximum case at full density** (50x50, all eligible loop cells with pearls). Worst case for memory and for time-to-first-loop.
15. **Maximum case at minimum density** (two or three pearls). Worst case for shortening: many equally valid loops, and the solver must not spend 7 s proving what is obvious.

---

## 8. Test and measurement hygiene

| # | Issue | What to do |
|---|---|---|
| 8.1 | **Cross-check propagation against brute force** on small grids (up to about 5x5 or 6x6 with few pearls), enumerating every loop. Verify the solver's loop is among the valid ones and that its length equals the true minimum on those cases. | Gives real proof of correctness for rules, instead of "looks fine". |
| 8.2 | **Independent checker.** AGENTS.md requires `check.py` to share no code with any solver. Keep it that way. The checker's own bugs (for example a wrong white-pearl rule) will pass wrong solutions. | Run the checker against hand-made invalid loops (two loops, wrong glyph, pearl violation) and confirm each is rejected with the right reason. |
| 8.3 | **Measure on cold runs.** First-run effects (page faults, instruction cache) affect small timings. Take the median of several runs. | Report median and worst case. |
| 8.4 | **Score what the judge scores.** Use the real formula with the real Lref, and record both loop length and runtime per test. Averages over local tests hide the large-grid tail. | Report per size class, not only the mean. |
| 8.5 | **Best-known Lref drifts.** **[repo]** Best-known lengths come from the repo's own solutions, so they are an upper bound on the true optimum. Improving the solver changes the reference. | Re-derive best-known after every improvement, and keep a lower bound next to it so you know how much headroom is left. |
| 8.6 | **Overfitting to the 33 local tests.** The set is small, and a parameter tuned to it (deadline, window size, restart interval) may not transfer. | Generate fresh tests with different seeds before trusting a tuning result. |

---

## 9. Priority order if time is short

1. Never print an invalid loop: internal validator plus best-valid fallback (1.1, 6.2, 6.3).
2. Never time out: wall-clock checks inside every inner loop and a hard cap well below 10 s (1.2, 4.4).
3. Exact I/O (1.3 to 1.6).
4. Single-loop correctness (2.1, 2.2).
5. Valid loops on large dense puzzles (3.1), because the zeros there cost the most points.
6. Shorter loops on large sparse puzzles (3.2, 3.8), where the squared-quality penalty applies.
7. Stop early when further improvement is unlikely (4.1, 4.2).

Everything marked **[unverified]** should be turned into a measurement before it drives a design decision.
