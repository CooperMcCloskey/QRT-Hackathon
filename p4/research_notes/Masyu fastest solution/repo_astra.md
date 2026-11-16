# Masyu p4: the `_astra` solver family (sol_v1..v6_astra), measurements, claims and weaknesses

Scope and provenance. Everything below comes from files in `/home/server/coding/competitions/QRT-Hackathon/p4` (cited as linked file:line references) plus a few re-runs/ablations I did from private copies in the scratchpad. No web search was used. The repo was changing while I worked (other agents added `sol_v7..v10_astra.cpp`, `sol_astra.cpp`, `README_astra.md`; the txt files `cdcl_astra.txt`, `sat_methods_astra.txt`, `flips_astra.txt`, `shortcuts_astra.txt` were deleted from the working tree after I had read them). `sol_v1..v6_astra.cpp` were verified byte-identical between my read and my build (`cmp`). All score numbers below use a **snapshot** I took of `scores.jsonl` (3 runs) and `scores_astra.jsonl` (10 runs, ending with v7) at 19:16:24 UTC; the live file has since grown to 16 runs.

Scoring formula used everywhere: `score = 100 * min(1, Lref/L)^2 * (0.5 + 0.5*exp(-max(0,T-1)/5))`, s = 1 ([statement.md: Scoring](/home/server/coding/competitions/QRT-Hackathon/p4/statement.md)). Lref per test = smallest valid L over every run in both jsonl snapshots (which includes the non-astra `sol_v1.cpp`, `sol_v2.cpp` and astra v7). Lref is a best-known value, not a proven optimum, so every score below is optimistic. T is the wall time recorded by `run_tests_astra.py` ([run_tests_astra.py:42-60](/home/server/coding/competitions/QRT-Hackathon/p4/run_tests_astra.py) `run_one`), i.e. whole-process time.

## 1. What algorithm does each version v1..v6 use, and what changed and why

### Takeaway
v1 is a pure constraint-propagation DFS over 7-state cell domains. v2 replaces first-solution search by an edge-variable CDCL SAT solver with lazy subtour cuts (and keeps the DFS only as a windowed "LNS" improver). v3, v4 are two experiments on cut strength/timing (v3 boundary cuts, v4 in-search cycle "theory" propagator); v5 makes the SAT stage robust (theory only for N>500 first, then restarts with shuffled variable order) and v6 adds a cheap face-flip local search on the first SAT loop. Each step fixed a measured failure of the previous step.

### Cited Findings

**Common skeleton (all versions).**
- Cell state = 7-value domain bitmask: shape index 0 = empty, 1 = `-`, 2 = `|`, 3..6 = `L J F 7`; masks `shape[7]={0,10,5,3,9,6,12}` with bits N=1,E=2,S=4,W=8 ([sol_v1_astra.cpp:18](/home/server/coding/competitions/QRT-Hackathon/p4/sol_v1_astra.cpp)). Initial domain: white 6 (`-`,`|`), black 120 (four corners), empty 127, border cells lose shapes pointing off-board ([sol_v1_astra.cpp:243-244](/home/server/coding/competitions/QRT-Hackathon/p4/sol_v1_astra.cpp)).
- `validate()` is a full independent check (reciprocal edges, pearl rules, single cycle walk) run before any candidate is stored as `best` ([sol_v1_astra.cpp:94-118](/home/server/coding/competitions/QRT-Hackathon/p4/sol_v1_astra.cpp)); output goes to `OUTPUT_PATH` if set, else stdout ([sol_v1_astra.cpp:274](/home/server/coding/competitions/QRT-Hackathon/p4/sol_v1_astra.cpp)). If nothing was found the program prints an all-`.` grid, which is invalid, so a failure scores 0 ([sol_v1_astra.cpp:262-263](/home/server/coding/competitions/QRT-Hackathon/p4/sol_v1_astra.cpp)).
- `lower = max(4, 2*(row span + col span of pearls))` is the only global lower bound used as stop criterion ([sol_v1_astra.cpp:247](/home/server/coding/competitions/QRT-Hackathon/p4/sol_v1_astra.cpp)); inside the DFS the pruning bound is `max(forcedCount, 2*max(colourCount[0], colourCount[1])) >= limit` (|F| bound and checkerboard-parity bound from the plan doc) ([sol_v1_astra.cpp:177](/home/server/coding/competitions/QRT-Hackathon/p4/sol_v1_astra.cpp), [sol_v1_astra.cpp:179](/home/server/coding/competitions/QRT-Hackathon/p4/sol_v1_astra.cpp)).
- Hard search deadline 7.3 s ([notes_astra.md:8](/home/server/coding/competitions/QRT-Hackathon/p4/notes_astra.md)); target for improvement phases ~0.85 s.

**v1: propagation + DFS ([sol_v1_astra.cpp:1-276](/home/server/coding/competitions/QRT-Hackathon/p4/sol_v1_astra.cpp))**
- `narrow()` shrinks a domain, pushes cell and 4 neighbours on a queue, and records a trail entry; `undo(mark)` restores ([sol_v1_astra.cpp:39-54](/home/server/coding/competitions/QRT-Hackathon/p4/sol_v1_astra.cpp)).
- `propagate()` ([sol_v1_astra.cpp:55-92](/home/server/coding/competitions/QRT-Hackathon/p4/sol_v1_astra.cpp)): table `edgeSupport[d][mask]` (built at [sol_v1_astra.cpp:232-238](/home/server/coding/competitions/QRT-Hackathon/p4/sol_v1_astra.cpp)) enforces reciprocal edges; black pearl rules as binary constraints on neighbours (straight along the axis, [sol_v1_astra.cpp:64-74](/home/server/coding/competitions/QRT-Hackathon/p4/sol_v1_astra.cpp)); white pearl: a straight orientation is viable only if at least one of the two neighbours may be a corner (`dom&120`), and if one is already straight the other is forced to a corner ([sol_v1_astra.cpp:77-89](/home/server/coding/competitions/QRT-Hackathon/p4/sol_v1_astra.cpp)).
- `connectivity()` ([sol_v1_astra.cpp:120-171](/home/server/coding/competitions/QRT-Hackathon/p4/sol_v1_astra.cpp)) rebuilds a union-find over **all N cells at every search node** (O(N alpha) per node, not the O(1) endpoint bookkeeping the plan doc recommends), detects a closed cycle, accepts it only if it contains all forced-occupied cells and passes `validate()` ([sol_v1_astra.cpp:134-144](/home/server/coding/competitions/QRT-Hackathon/p4/sol_v1_astra.cpp)), forbids premature closure edges between two endpoints of the same open path ([sol_v1_astra.cpp:145-155](/home/server/coding/competitions/QRT-Hackathon/p4/sol_v1_astra.cpp)), and every 16th node or depth<4 runs a possible-edge flood fill to check all required cells are mutually reachable ([sol_v1_astra.cpp:156-169](/home/server/coding/competitions/QRT-Hackathon/p4/sol_v1_astra.cpp), called with `depth<4||(nodes&15)==0` at [sol_v1_astra.cpp:180](/home/server/coding/competitions/QRT-Hackathon/p4/sol_v1_astra.cpp)).
- `search()` ([sol_v1_astra.cpp:172-220](/home/server/coding/competitions/QRT-Hackathon/p4/sol_v1_astra.cpp)): branch variable = highest score of `10000/domainSize + 30*activity + 700 (pearl) + 1200 (open endpoint)` (+random in restart mode), O(N) scan per node ([sol_v1_astra.cpp:187-194](/home/server/coding/competitions/QRT-Hackathon/p4/sol_v1_astra.cpp)); values ordered by a cost preferring already-occupied neighbours and open endpoints ([sol_v1_astra.cpp:200-211](/home/server/coding/competitions/QRT-Hackathon/p4/sol_v1_astra.cpp)); failed variables get `activity++` ([sol_v1_astra.cpp:218](/home/server/coding/competitions/QRT-Hackathon/p4/sol_v1_astra.cpp)).
- `solve()` ([sol_v1_astra.cpp:249-266](/home/server/coding/competitions/QRT-Hackathon/p4/sol_v1_astra.cpp)): randomised restarts, node limit `min(2e6, 2000*2^min(round,9))`, time slice 0.18 s for rounds 0-2 then 0.65 s, until 7.2 s ([sol_v1_astra.cpp:252-257](/home/server/coding/competitions/QRT-Hackathon/p4/sol_v1_astra.cpp)); only if a solution exists and `elapsed<0.8`, one branch-and-bound DFS (`limit=bestLen`, deadline 0.85) tries to shorten ([sol_v1_astra.cpp:258-261](/home/server/coding/competitions/QRT-Hackathon/p4/sol_v1_astra.cpp)).

**v2: CDCL SAT with lazy subtour cuts + DFS-window LNS ([sol_v2_astra.cpp:16-99](/home/server/coding/competitions/QRT-Hackathon/p4/sol_v2_astra.cpp), [sol_v2_astra.cpp:313-365](/home/server/coding/competitions/QRT-Hackathon/p4/sol_v2_astra.cpp), [sol_v2_astra.cpp:366-379](/home/server/coding/competitions/QRT-Hackathon/p4/sol_v2_astra.cpp), [sol_v2_astra.cpp:403-414](/home/server/coding/competitions/QRT-Hackathon/p4/sol_v2_astra.cpp))**
- `class CDCL`: two-watched-literal propagation ([sol_v2_astra.cpp:29-45](/home/server/coding/competitions/QRT-Hackathon/p4/sol_v2_astra.cpp)), first-UIP `analyze` ([sol_v2_astra.cpp:51-66](/home/server/coding/competitions/QRT-Hackathon/p4/sol_v2_astra.cpp)), VSIDS-style activity with lazy heap and decay factor 1/0.95 ([sol_v2_astra.cpp:27](/home/server/coding/competitions/QRT-Hackathon/p4/sol_v2_astra.cpp), [sol_v2_astra.cpp:86](/home/server/coding/competitions/QRT-Hackathon/p4/sol_v2_astra.cpp)), phase saving ([sol_v2_astra.cpp:26](/home/server/coding/competitions/QRT-Hackathon/p4/sol_v2_astra.cpp)), restarts every 128 conflicts growing by `128+n/3` ([sol_v2_astra.cpp:88](/home/server/coding/competitions/QRT-Hackathon/p4/sol_v2_astra.cpp)), deadline check every 32 conflicts ([sol_v2_astra.cpp:81](/home/server/coding/competitions/QRT-Hackathon/p4/sol_v2_astra.cpp)). No clause deletion (learned clauses are only appended, [sol_v2_astra.cpp:28](/home/server/coding/competitions/QRT-Hackathon/p4/sol_v2_astra.cpp)). All learned clauses die with the object when `satSearch` returns.
- `satSearch(end, margin)`: one boolean per grid edge (E = R(C-1)+(R-1)C) plus a constant-false variable for off-board edges ([sol_v2_astra.cpp:314-319](/home/server/coding/competitions/QRT-Hackathon/p4/sol_v2_astra.cpp)). Per cell: "an used edge implies another used edge" (4 clauses) and "no 3 of 4" (4 clauses) = degree 0 or 2 ([sol_v2_astra.cpp:322-323](/home/server/coding/competitions/QRT-Hackathon/p4/sol_v2_astra.cpp)); white and black clauses encode straightness/turning and the neighbour conditions ([sol_v2_astra.cpp:324-331](/home/server/coding/competitions/QRT-Hackathon/p4/sol_v2_astra.cpp)). `margin>=0` bans every edge outside the pearl bounding box expanded by `margin` ([sol_v2_astra.cpp:333-337](/home/server/coding/competitions/QRT-Hackathon/p4/sol_v2_astra.cpp)).
- Lazy subtour elimination: after each SAT model, extract connected components; a component containing all P pearls that passes `validate()` and beats `bestLen` is accepted (other pearl-free cycles are simply dropped, a nice property of the formulation) ([sol_v2_astra.cpp:341-356](/home/server/coding/competitions/QRT-Hackathon/p4/sol_v2_astra.cpp)); otherwise **for every component** add a clause "not all of these edges together" ([sol_v2_astra.cpp:357-362](/home/server/coding/competitions/QRT-Hackathon/p4/sol_v2_astra.cpp)) and re-solve.
- `solve()`: `satSearch(0.30, margin 2)`, if still nothing `satSearch(7.15, -1)` ([sol_v2_astra.cpp:404-405](/home/server/coding/competitions/QRT-Hackathon/p4/sol_v2_astra.cpp)); if N<=100 one DFS branch-and-bound with deadline 0.40 ([sol_v2_astra.cpp:407](/home/server/coding/competitions/QRT-Hackathon/p4/sol_v2_astra.cpp)); then `localImprove` ([sol_v2_astra.cpp:408](/home/server/coding/competitions/QRT-Hackathon/p4/sol_v2_astra.cpp)): pick a random loop cell, free a random window of 4-12 x 4-12 cells (all other cells fixed to the incumbent shape), run the v1 DFS with `limit=bestLen` and `nodeLimit=1200`, stop after 250 consecutive non-improving windows or at `end=min(7.3,max(0.85,elapsed+0.12))` ([sol_v2_astra.cpp:366-379](/home/server/coding/competitions/QRT-Hackathon/p4/sol_v2_astra.cpp)). This is the "LNS" of the repo; it is windowed DFS, not SAT.

**v3: + boundary (connectivity) cuts ([sol_v3_astra.cpp:362-368](/home/server/coding/competitions/QRT-Hackathon/p4/sol_v3_astra.cpp))**
- For a component with 0 < pearls < P add the positive clause "at least one grid edge leaving the component is used" (all boundary edges), in addition to the v2 negative cut. Everything else equals v2.

**v4: + in-search cycle "theory" ([sol_v4_astra.cpp:20](/home/server/coding/competitions/QRT-Hackathon/p4/sol_v4_astra.cpp), [sol_v4_astra.cpp:85-90](/home/server/coding/competitions/QRT-Hackathon/p4/sol_v4_astra.cpp), [sol_v4_astra.cpp:103](/home/server/coding/competitions/QRT-Hackathon/p4/sol_v4_astra.cpp), [sol_v4_astra.cpp:346-362](/home/server/coding/competitions/QRT-Hackathon/p4/sol_v4_astra.cpp))**
- `CDCL::theory` callback invoked at every 8th conflict-free propagation fixpoint: union-find over the currently TRUE edges; if a cycle that lacks pearls (`pcnt != P`) exists, return its "not all these edges" clause, store it as a conflict, back-jump to the highest decision level in it and analyse ([sol_v4_astra.cpp:85-90](/home/server/coding/competitions/QRT-Hackathon/p4/sol_v4_astra.cpp)). The post-model boundary cuts of v3 are removed (the diff v3->v4 deletes [sol_v3_astra.cpp:362-368](/home/server/coding/competitions/QRT-Hackathon/p4/sol_v3_astra.cpp)); post-model per-component negative cuts remain.

**v5: hedged SAT stage ([sol_v5_astra.cpp:320-329](/home/server/coding/competitions/QRT-Hackathon/p4/sol_v5_astra.cpp), [sol_v5_astra.cpp:351](/home/server/coding/competitions/QRT-Hackathon/p4/sol_v5_astra.cpp), [sol_v5_astra.cpp:433-445](/home/server/coding/competitions/QRT-Hackathon/p4/sol_v5_astra.cpp))**
- `satSearch(end, margin, useTheory, variant)` (signature at [sol_v5_astra.cpp:320](/home/server/coding/competitions/QRT-Hackathon/p4/sol_v5_astra.cpp)): `variant=1` randomly permutes the edge numbering before building the CDCL, i.e. a different tie-break order ([sol_v5_astra.cpp:325-329](/home/server/coding/competitions/QRT-Hackathon/p4/sol_v5_astra.cpp)).
- `solve()`: first call `satSearch(0.30, 2, N>500)` (theory ON only for big boards), second call `satSearch(+0.7 s, margin 2, N<=500)` (theory ON only for small boards), then a loop of randomised restarts with growing slices `0.6+0.2*attempt`, margin `-1` every third attempt, theory alternating ([sol_v5_astra.cpp:433-440](/home/server/coding/competitions/QRT-Hackathon/p4/sol_v5_astra.cpp)) until a loop exists (<7.1 s). Improvement stage identical to v2/v4 (DFS if N<=100, then `localImprove`).

**v6: + face-flip local search ([sol_v6_astra.cpp:396-438](/home/server/coding/competitions/QRT-Hackathon/p4/sol_v6_astra.cpp), [sol_v6_astra.cpp:484-490](/home/server/coding/competitions/QRT-Hackathon/p4/sol_v6_astra.cpp))**
- `faceImprove(end)`: works on the current shape-mask array. Picks a random 2x2 face (cells a, a+1, a+C+1, a+C); computes which of its 4 sides are on the loop; skips if 0, 4, or the two opposite sides (which could split the loop); `delta = 4 - 2*count` of used sides is the length change; uphill moves (delta>0) are accepted with probability 2/1000; toggles the four corner cells by XOR masks `{6,12,9,3}`, requires every new cell to be a valid shape and every touched pearl cell (and its neighbours) to still satisfy white/black rules (`legalPearl`), and only when the running length beats `bestLen` runs the full `validate()` and stores the improvement ([sol_v6_astra.cpp:411-433](/home/server/coding/competitions/QRT-Hackathon/p4/sol_v6_astra.cpp)). It resets to the incumbent when drift exceeds `bestLen+6` ([sol_v6_astra.cpp:434](/home/server/coding/competitions/QRT-Hackathon/p4/sol_v6_astra.cpp)). Early exit when `stale > 20000+20*N` consecutive iterations without a new best ([sol_v6_astra.cpp:409](/home/server/coding/competitions/QRT-Hackathon/p4/sol_v6_astra.cpp), loop at [sol_v6_astra.cpp:408-435](/home/server/coding/competitions/QRT-Hackathon/p4/sol_v6_astra.cpp)).
- Order in `solve()`: SAT stage; DFS if N<=100; then `end = min(7.3, max(0.85, elapsed+0.12))`; `faceImprove` for at most 0.12 s; `localImprove` until `end-0.08`; `faceImprove` to `end` ([sol_v6_astra.cpp:484-490](/home/server/coding/competitions/QRT-Hackathon/p4/sol_v6_astra.cpp)). The only code diff v5->v6 is this block plus the function.
- Same face-flip idea existed in the non-astra `sol_v1.cpp` as simulated annealing (SA from the border rectangle, [sol_v1.cpp:532-542](/home/server/coding/competitions/QRT-Hackathon/p4/sol_v1.cpp)); the astra variant is a near-greedy version (0.2 % uphill) seeded from a SAT loop instead of a rectangle.

**What the SAT stage looks like in practice (instrumented private copy of v6, single runs, machine under load average 14-18, so times are pessimistic).** Format: first valid loop time, length -> after first faceImprove -> after LNS = final.
- 13_20x15_d100: 0.011 s after 33 model/cut rounds, 272 -> 216 -> 144 (final 144, 0.30 s).
- 23_30x40_d20: 0.101 s (first model valid, 0 cut rounds), 492 -> 278 -> 154.
- 26_40x40_d50: 0.039 s, 644 -> 580 -> 478 (final 470).
- 29_50x50_d100: 0.058 s (2 cut rounds, theory on), 826 -> 666 -> 582.
- 32_50x50_d5: 0.031 s, 250 -> 194 -> 180. 28_40x40_d5: 0.033 s, 136 -> 82 -> 58.
- 30_50x50_d50: first call hit its 0.30 s deadline unsolved (status -1), second call solved at 0.638 s after 39 cut rounds (2-32 components per model), 1062 -> 724 (11 ms) -> 700 (final 698).
- 50x50_s113_d3_astra: solved at 0.754 s after 23 cut rounds with 1386 cells, faceImprove shrinks it to 378 in 22 ms, final 368 (repo run of v6: 342).
- 35x48_s109_d25_astra: solved at 1.195 s after 19 cut rounds (len 1278) -> 846 -> 842, final 840 (repo run: 738, T=0.78 s; v5 repo run: 1150).
- 50x50_s113_d100_astra (840 pearls): about 80 model/cut rounds; SAT attempts timed out at 0.30, 1.02, 1.63 (variant 1), 2.46 s; solved at 4.47 s in my loaded run (repo run: total 2.29 s).
So the SAT loop is long and arbitrary (it only has to cover the pearls) and the whole later quality comes from shrinking it.

### Inferences
- The progression is "make first-solution finding reliable" (v1->v2, v4->v5) and then "make the first loop short" (v5->v6); the loop-shortening phase was the missing piece for N>700.
- The astra DFS in v1 is not the incremental design promised by the plan doc (O(N) connectivity and O(N) variable scan per node), which explains its low node rate (section 3).

### Gaps
- I did not trace v7+ (out of scope, still changing).
- No per-phase timing logs exist in the repo; the phase numbers above come from my own instrumented copy (not the repo's binaries).

## 2. Per-test results (L, runtime T, contest score) and per-version averages

### Takeaway
Averages with Lref = best-known over all repo runs (snapshot): v1 38.6 (33 tests), v2 77.8, v3 70.8, v4 68.2, v5 88.8 (33) / 86.5 (48), **v6 94.2 (33) / 97.1 (15 astra) / 95.1 (48)**. v1 fails 12/33 (and 10/15 on astra tests in my re-run), v2 1, v3 4, v4 1, v5 and v6 0 of 48. The big jump v4 -> v5 in the repo table is partly measurement noise (see caveats).

### Cited Findings
Data: [scores_astra.jsonl](/home/server/coding/competitions/QRT-Hackathon/p4/scores_astra.jsonl) (v1 line 1, v2 line 2, v3 line 3, v4 line 4, v5 lines 5-7, v6 lines 8-9) and [scores.jsonl](/home/server/coding/competitions/QRT-Hackathon/p4/scores.jsonl) for non-astra context. Cell format `L/T/score`; `FAIL` = invalid/timeout (output empty grid after the 7.3 s deadline; all FAIL rows show T = 7.2-7.3 s). v1..v4 were never run on the 15 astra tests in the repo (`n/a`). v5 = jsonl line 7 (48 tests), v6 = line 8. "Lref" column: best-known L and which solution first achieved it ((A) = astra solution file version, (N) = non-astra `sol_v1.cpp`/`sol_v2.cpp`, e.g. `sol_v7(A)`).

| test | pearls | Lref (first source) | v1 | v2 | v3 | v4 | v5 | v6 |
|---|---|---|---|---|---|---|---|---|
| 00_sample | 4 | 18 (sol_v1(N)) | 18/0.00s/100.0 | 18/0.01s/100.0 | 18/0.01s/100.0 | 18/0.10s/100.0 | 18/0.01s/100.0 | 18/0.01s/100.0 |
| 01_6x6_d100 | 5 | 14 (sol_v1(N)) | 14/0.00s/100.0 | 14/0.00s/100.0 | 14/0.01s/100.0 | 14/0.07s/100.0 | 14/0.00s/100.0 | 14/0.00s/100.0 |
| 02_6x6_d50 | 4 | 16 (sol_v1(N)) | 16/0.00s/100.0 | 16/0.00s/100.0 | 16/0.01s/100.0 | 16/0.03s/100.0 | 16/0.00s/100.0 | 16/0.01s/100.0 |
| 03_6x6_d20 | 3 | 14 (sol_v1(N)) | 14/0.00s/100.0 | 14/0.01s/100.0 | 14/0.01s/100.0 | 14/0.14s/100.0 | 14/0.01s/100.0 | 14/0.01s/100.0 |
| 04_6x6_d5 | 2 | 10 (sol_v1(N)) | 10/0.00s/100.0 | 10/0.01s/100.0 | 10/0.01s/100.0 | 10/0.08s/100.0 | 10/0.01s/100.0 | 10/0.01s/100.0 |
| 05_8x10_d100 | 17 | 40 (sol_v1(N)) | 40/0.01s/100.0 | 40/0.01s/100.0 | 40/0.02s/100.0 | 40/0.30s/100.0 | 40/0.01s/100.0 | 40/0.01s/100.0 |
| 06_8x10_d50 | 13 | 28 (sol_v1(N)) | 28/0.00s/100.0 | 28/0.03s/100.0 | 28/0.03s/100.0 | 28/0.27s/100.0 | 28/0.02s/100.0 | 28/0.02s/100.0 |
| 07_8x10_d20 | 6 | 24 (sol_v1(N)) | 24/0.01s/100.0 | 24/0.01s/100.0 | 24/0.03s/100.0 | 24/0.04s/100.0 | 24/0.03s/100.0 | 24/0.03s/100.0 |
| 08_8x10_d5 | 2 | 20 (sol_v1(N)) | 20/0.60s/100.0 | 20/0.33s/100.0 | 20/0.33s/100.0 | 20/0.87s/100.0 | 20/0.30s/100.0 | 20/0.31s/100.0 |
| 09_12x12_d100 | 29 | 68 (sol_v1(N)) | 68/0.12s/100.0 | 68/0.10s/100.0 | 68/0.10s/100.0 | 68/0.71s/100.0 | 68/0.09s/100.0 | 68/0.09s/100.0 |
| 10_12x12_d50 | 15 | 44 (sol_v2(N)) | 86/0.85s/26.2 | 46/0.11s/91.5 | 46/0.11s/91.5 | 46/0.89s/91.5 | 46/0.12s/91.5 | 46/0.11s/91.5 |
| 11_12x12_d20 | 4 | 22 (sol_v1(N)) | 32/0.85s/47.3 | 22/0.12s/100.0 | 22/0.15s/100.0 | 22/0.88s/100.0 | 22/0.11s/100.0 | 22/0.14s/100.0 |
| 12_12x12_d5 | 4 | 28 (sol_v1(N)) | 56/0.85s/25.0 | 28/0.10s/100.0 | 28/0.15s/100.0 | 28/0.89s/100.0 | 28/0.11s/100.0 | 28/0.13s/100.0 |
| 13_20x15_d100 | 66 | 144 (sol_v3(A)) | FAIL/7.23s/0.0 | 152/0.16s/89.8 | 144/0.21s/100.0 | FAIL/7.21s/0.0 | 144/0.18s/100.0 | 144/0.19s/100.0 |
| 14_20x15_d50 | 23 | 74 (sol_v4(A)) | 128/0.85s/33.4 | 84/0.26s/77.6 | 76/0.26s/94.8 | 74/0.88s/100.0 | 88/0.23s/70.7 | 90/0.29s/67.6 |
| 15_20x15_d20 | 8 | 42 (sol_v2(N)) | 86/0.85s/23.9 | 42/0.22s/100.0 | 42/0.29s/100.0 | 42/0.87s/100.0 | 42/0.24s/100.0 | 42/0.31s/100.0 |
| 16_20x15_d5 | 4 | 36 (sol_v1(N)) | 56/0.85s/41.3 | 36/0.41s/100.0 | 36/0.27s/100.0 | 36/0.89s/100.0 | 36/0.25s/100.0 | 40/0.26s/81.0 |
| 17_25x25_d100 | 94 | 212 (sol_v2(A)) | 366/0.85s/33.6 | 212/0.85s/100.0 | 222/0.35s/91.2 | 302/0.88s/49.3 | 214/0.52s/98.1 | 216/0.77s/96.3 |
| 18_25x25_d50 | 36 | 116 (sol_v7(A)) | 366/3.07s/8.3 | 120/0.58s/93.4 | 124/0.74s/87.5 | 138/0.86s/70.7 | 120/0.51s/93.4 | 130/0.77s/79.6 |
| 19_25x25_d20 | 13 | 78 (sol_v5(A)) | 358/0.85s/4.7 | 96/0.85s/66.0 | 94/0.70s/68.9 | 126/0.88s/38.3 | 78/0.85s/100.0 | 80/0.43s/95.1 |
| 20_25x25_d5 | 6 | 58 (sol_v6(A)) | 368/5.48s/1.7 | 62/0.44s/87.5 | 60/0.71s/93.4 | 74/0.89s/61.4 | 60/0.35s/93.4 | 58/0.28s/100.0 |
| 21_30x40_d100 | 149 | 370 (sol_v6(A)) | 682/0.85s/29.4 | 506/0.85s/53.5 | 730/1.88s/23.6 | 472/0.87s/61.4 | 374/0.85s/97.9 | 370/0.77s/100.0 |
| 22_30x40_d50 | 78 | 248 (sol_v6(A)) | FAIL/7.30s/0.0 | 268/0.85s/85.6 | 308/0.85s/64.8 | 398/0.88s/38.8 | 276/0.85s/80.7 | 250/0.77s/98.4 |
| 23_30x40_d20 | 26 | 148 (sol_v6(A)) | FAIL/7.30s/0.0 | 636/1.24s/5.3 | 682/1.47s/4.5 | 454/0.87s/10.6 | 172/0.85s/74.0 | 148/0.77s/100.0 |
| 24_30x40_d5 | 6 | 70 (sol_v6(A)) | FAIL/7.30s/0.0 | 88/0.85s/63.3 | 98/0.85s/51.0 | 164/0.87s/18.2 | 100/0.85s/49.0 | 70/0.66s/100.0 |
| 25_40x40_d100 | 163 | 412 (sol_v5(A)) | FAIL/7.32s/0.0 | 528/0.85s/60.9 | 922/2.20s/17.8 | 502/0.89s/67.4 | 416/0.85s/98.1 | 438/0.77s/88.5 |
| 26_40x40_d50 | 122 | 402 (sol_v5(A)) | FAIL/7.32s/0.0 | FAIL/7.17s/0.0 | FAIL/7.18s/0.0 | 610/0.89s/43.4 | 402/0.85s/100.0 | 420/0.77s/91.6 |
| 27_40x40_d20 | 40 | 236 (sol_v2(N)) | FAIL/7.30s/0.0 | 496/0.85s/22.6 | FAIL/7.18s/0.0 | 986/0.88s/5.7 | 326/0.85s/52.4 | 262/0.78s/81.1 |
| 28_40x40_d5 | 5 | 58 (sol_v1(N)) | FAIL/7.30s/0.0 | 62/0.85s/87.5 | 62/0.78s/87.5 | 70/0.87s/68.7 | 64/0.54s/82.1 | 58/0.20s/100.0 |
| 29_50x50_d100 | 204 | 558 (sol_v6(A)) | FAIL/7.30s/0.0 | 876/1.66s/38.1 | FAIL/7.18s/0.0 | 764/0.89s/53.3 | 588/0.85s/90.1 | 560/0.78s/99.3 |
| 30_50x50_d50 | 126 | 616 (sol_v7(A)) | FAIL/7.30s/0.0 | 1274/6.05s/15.9 | FAIL/7.18s/0.0 | 1274/2.49s/20.4 | 712/0.85s/74.9 | 618/0.78s/99.4 |
| 31_50x50_d20 | 59 | 328 (sol_v1(N)) | FAIL/7.30s/0.0 | 614/0.85s/28.5 | 488/0.85s/45.2 | 698/0.86s/22.1 | 476/0.85s/47.5 | 346/0.78s/89.9 |
| 32_50x50_d5 | 12 | 126 (sol_v2(A)) | FAIL/7.30s/0.0 | 126/0.85s/100.0 | 348/0.85s/13.1 | 234/0.89s/29.0 | 204/0.85s/38.1 | 178/0.77s/50.1 |
| 20x20_s107_d100_astra | 143 | 358 (sol_v5(A)) | n/a | n/a | n/a | n/a | 358/0.05s/100.0 | 358/0.09s/100.0 |
| 20x20_s107_d25_astra | 44 | 160 (sol_v7(A)) | n/a | n/a | n/a | n/a | 162/0.60s/97.5 | 162/0.77s/97.5 |
| 20x20_s107_d3_astra | 4 | 42 (sol_v5(A)) | n/a | n/a | n/a | n/a | 42/0.30s/100.0 | 42/0.29s/100.0 |
| 35x48_s109_d100_astra | 553 | 1362 (sol_v5(A)) | n/a | n/a | n/a | n/a | 1362/0.85s/100.0 | 1374/0.78s/98.3 |
| 35x48_s109_d25_astra | 140 | 738 (sol_v6(A)) | n/a | n/a | n/a | n/a | 1150/0.85s/41.2 | 738/0.78s/100.0 |
| 35x48_s109_d3_astra | 16 | 182 (sol_v6(A)) | n/a | n/a | n/a | n/a | 328/0.85s/30.8 | 182/0.78s/100.0 |
| 50x50_s113_d100_astra | 840 | 2160 (sol_v7(A)) | n/a | n/a | n/a | n/a | 2258/2.37s/80.5 | 2162/2.29s/88.4 |
| 50x50_s113_d25_astra | 194 | 1274 (sol_v7(A)) | n/a | n/a | n/a | n/a | 1432/0.85s/79.2 | 1322/0.78s/92.9 |
| 50x50_s113_d3_astra | 21 | 312 (sol_v7(A)) | n/a | n/a | n/a | n/a | 1030/0.85s/9.2 | 342/0.78s/83.2 |
| 50x6_s103_d100_astra | 103 | 256 (sol_v5(A)) | n/a | n/a | n/a | n/a | 256/0.05s/100.0 | 256/0.04s/100.0 |
| 50x6_s103_d25_astra | 25 | 122 (sol_v5(A)) | n/a | n/a | n/a | n/a | 122/0.43s/100.0 | 122/0.36s/100.0 |
| 50x6_s103_d3_astra | 8 | 98 (sol_v6(A)) | n/a | n/a | n/a | n/a | 102/0.41s/92.3 | 98/0.35s/100.0 |
| 6x50_s101_d100_astra | 115 | 278 (sol_v5(A)) | n/a | n/a | n/a | n/a | 278/0.01s/100.0 | 278/0.03s/100.0 |
| 6x50_s101_d25_astra | 35 | 152 (sol_v5(A)) | n/a | n/a | n/a | n/a | 152/0.54s/100.0 | 152/0.31s/100.0 |
| 6x50_s101_d3_astra | 3 | 86 (sol_v7(A)) | n/a | n/a | n/a | n/a | 92/0.30s/87.4 | 88/0.33s/95.5 |
| **mean 33 tests/** | | | 38.63 | 77.79 | 70.76 | 68.19 | 88.85 | 94.22 |
| **mean 15 astra** | | | n/a | n/a | n/a | n/a | 81.21 | 97.06 |
| **mean all 48** | | | n/a | n/a | n/a | n/a | 86.46 | 95.11 |

Averages from the same data: v1 38.63, v2 77.79, v3 70.76, v4 68.19, v5 88.75 (jsonl line 5, 33 tests) / 88.85 (line 7) / 81.09 (line 6, the 15 astra tests alone) / 86.46 (48 tests), v6 94.22 (33) / 97.06 (15) / 95.11 (48) (second v6 run, jsonl line 9: 95.07). Non-astra for reference: `sol_v1.cpp` 63.11 / 60.42, `sol_v2.cpp` 88.92 (33 tests). If v7's results are excluded from Lref: v5 86.91, v6 95.88 (48 tests). The v7 (out of scope) average is 93.76 against the same Lref.

**Which tests failed and why (jsonl, T shown is the deadline kill, all after about 7.2-7.3 s).**
- v1 (12/33): 13_20x15_d100, and every 30x40, 40x40, 50x50 test except 21_30x40_d100 (22, 23, 24, 25..32). In my re-run (binary rebuilt from the repo file, loaded machine) v1 additionally failed 10 of 15 astra tests: 35x48 x3, 50x50 x3, 50x6 d25/d3, 6x50 d25/d3. Node counters from stderr: 13_20x15_d100 1.52 M nodes in 7.3 s with no loop (about 208 k nodes/s, N=300); 50x50_d50: 213 k nodes in 7.3 s (about 29 k nodes/s, N=2500); 6x50_d3: 1.86 M nodes (about 255 k/s) and still no loop. So it is search failure, not slowness, and the per-node rate scales ~1/N.
- v2 (1/33): 26_40x40_d50 (7.165 s). Other slow ones: 30_50x50_d50 6.05 s (L=1274, score 15.9), 23_30x40_d20 1.24 s.
- v3 (4/33): 26_40x40_d50, 27_40x40_d20, 29_50x50_d100, 30_50x50_d50. v3 also slow where solved: 21_30x40_d100 1.88 s, 25_40x40_d100 2.20 s.
- v4 (1/33): 13_20x15_d100 (7.208 s). Confirmed in my re-run (v4 prints `length=0 nodes=0 time=7.19`), and v2/v3/v5/v6 solve that test in 0.16/0.21/0.17/0.19 s.
- v5, v6 (0/48).
- v1 failed because DFS cannot satisfy the global single-loop constraint on N >= ~1200 or dense N=300 (section 3).

**Re-run (private, same repo sources, each version run once sequentially per version but 6 versions in parallel on a 12-core box with load average 14-18 from other agents). Treat as noisy.** Cell = `L/T/score`.

| test | Lref | v1 | v2 | v3 | v4 | v5 | v6 |
|---|---|---|---|---|---|---|---|
| 20x20_s107_d100_astra | 358 | 354/0.9s/100.0 | 358/0.0s/100.0 | 352/0.1s/100.0 | 358/0.0s/100.0 | 358/0.0s/100.0 | 358/0.1s/100.0 |
| 20x20_s107_d25_astra | 160 | 278/2.9s/27.9 | 162/0.6s/97.5 | 160/0.9s/100.0 | 158/0.6s/100.0 | 162/0.5s/97.5 | 162/0.8s/97.5 |
| 20x20_s107_d3_astra | 42 | 180/2.6s/4.7 | 42/0.4s/100.0 | 48/0.7s/76.6 | 44/0.3s/91.1 | 42/0.3s/100.0 | 42/0.3s/100.0 |
| 35x48_s109_d100_astra | 1362 | FAIL/7.3s/0.0 | 1348/0.9s/100.0 | 1486/1.2s/82.8 | 1362/0.9s/100.0 | 1370/0.9s/98.8 | 1368/0.8s/99.1 |
| 35x48_s109_d25_astra | 738 | FAIL/7.3s/0.0 | 1222/1.0s/36.5 | FAIL/7.2s/0.0 | 1274/1.2s/33.0 | 1094/0.9s/45.5 | 784/0.8s/88.6 |
| 35x48_s109_d3_astra | 182 | FAIL/7.3s/0.0 | 412/0.9s/19.5 | 1074/6.9s/1.9 | 418/0.9s/19.0 | 320/0.9s/32.3 | 182/0.8s/100.0 |
| 50x50_s113_d100_astra | 2160 | FAIL/7.3s/0.0 | FAIL/7.2s/0.0 | FAIL/7.2s/0.0 | FAIL/7.2s/0.0 | 2236/2.3s/82.3 | 2148/2.4s/87.8 |
| 50x50_s113_d25_astra | 1274 | FAIL/7.3s/0.0 | 2016/1.7s/37.2 | FAIL/7.2s/0.0 | 1576/0.9s/65.3 | 1374/0.9s/86.0 | 1306/0.8s/95.2 |
| 50x50_s113_d3_astra | 312 | FAIL/7.3s/0.0 | 1556/1.3s/3.9 | FAIL/7.2s/0.0 | 990/0.9s/9.9 | 954/0.9s/10.7 | 342/0.8s/83.2 |
| 50x6_s103_d100_astra | 256 | 256/0.9s/100.0 | 256/0.1s/100.0 | 256/0.1s/100.0 | 256/0.1s/100.0 | 256/0.1s/100.0 | 256/0.1s/100.0 |
| 50x6_s103_d25_astra | 122 | FAIL/7.3s/0.0 | 122/0.9s/100.0 | FAIL/7.2s/0.0 | 122/0.5s/100.0 | 122/0.6s/100.0 | 122/0.3s/100.0 |
| 50x6_s103_d3_astra | 98 | FAIL/7.3s/0.0 | 102/0.9s/92.3 | 118/1.7s/64.3 | 98/0.6s/100.0 | 102/0.9s/92.3 | 98/0.3s/100.0 |
| 6x50_s101_d100_astra | 278 | 278/0.0s/100.0 | 278/0.0s/100.0 | 278/0.0s/100.0 | 278/0.0s/100.0 | 278/0.0s/100.0 | 278/0.0s/100.0 |
| 6x50_s101_d25_astra | 152 | FAIL/7.3s/0.0 | 152/0.9s/100.0 | 156/1.0s/94.5 | 156/1.7s/88.6 | 152/0.9s/100.0 | 152/0.4s/100.0 |
| 6x50_s101_d3_astra | 86 | FAIL/7.3s/0.0 | 92/0.6s/87.4 | 92/0.6s/87.4 | 92/0.6s/87.4 | 92/0.5s/87.4 | 88/0.4s/95.5 |
| **mean 15** | | 22.17 | 71.62 | 53.83 | 72.95 | 82.19 | 96.46 |

Re-run means on all 48 tests: v1 33.51, v2 81.09, v3 65.27, v4 80.46, v5 84.74, v6 94.64 (33-test parts: 38.67, 85.40, 70.47, 83.87, 85.90, 93.82; fails over 48 tests: v1 22, v2 1, v3 9, v4 2, v5 0, v6 0).

**Caveats on the repo measurements (important for interpretation).**
- All improvement phases are bounded by wall-clock deadlines, so results depend on CPU contention. The repo's v4 row shows T = 0.87-0.89 s even for 12x12 tests (v2/v3 need 0.1 s there) and L values worse than my re-run (e.g. 17_25x25_d100: 302 in jsonl vs 214 in re-run; 30_50x50_d50 1274 vs 998; 27_40x40_d20 986 vs 404). It was almost certainly measured while another process was loading the machine (the 250-failure cap ends `localImprove` at about 0.1-0.4 s on an idle box). Consequently the repo's apparent v4 -> v5 jump (68.2 -> 88.8) is inflated; my re-run gives 83.9 -> 85.9 for the 33 tests. The real v4 -> v5 gain is mainly fixing 13_20x15_d100 (0 -> 100 points).
- Same binary, repeated runs on my loaded machine (3 runs each): v5 27_40x40_d20 L = 516/436/434 (jsonl 326), v5 19_25x25_d20 102/102/116 (jsonl 78 twice), v5 31_50x50_d20 646/718/718 (jsonl 476), v6 27_40x40_d20 338/262/320 (jsonl 262), v6 31_50x50_d20 366/376/366 (jsonl 346), v6 50x50_s113_d3 376/370/368 (jsonl 342). So L can move by 10-25 % with load: the judge's CPU speed matters for these solvers.
- `run_tests_astra.py` writes `scores_astra.jsonl` with whole-process time ([run_tests_astra.py:42-60](/home/server/coding/competitions/QRT-Hackathon/p4/run_tests_astra.py), [run_tests_astra.py:161-174](/home/server/coding/competitions/QRT-Hackathon/p4/run_tests_astra.py)); its own "rel" column is a linear best-ratio and not the contest score ([run_tests_astra.py:13](/home/server/coding/competitions/QRT-Hackathon/p4/run_tests_astra.py) docstring and [fastestImplementationReasonAstra:171-192](/home/server/coding/competitions/QRT-Hackathon/p4/fastestImplementationReasonAstra)).

### Inferences
- The v6 total of about 95 is with Lref = best known; it will be lower against true optima. Cross-check from another agent's CP-SAT run in the shared scratchpad (not part of the repo, unverified by me, `scratchpad/exact_out.jsonl`): proven optimal 00-16 and 18 (10_12x12_d50 = 44, 13 = 144, 14 = 74, 15 = 42, 16 = 36, 18 = 116), and for 17 only 226 (bound 204), 19 80 (bound 74), 20 58 (49), 21_30x40_d100 **360** (bound 342; the repo-best is 370, so Lref there is not tight), 22 302 (226), 23 158 (116), 24 98 (46), 28 58 (35). The 33 `tests/` Lref values are therefore exact for 18 tests and plausibly within a few percent for several others, but not for the large sparse ones.

### Gaps
- No repo measurement of v1..v4 on the 15 astra tests; only my loaded re-run.
- Lref is best-known; true optimum unknown for the big boards and statement says hidden Lref may use s=0.9.

## 3. What worked, what failed, and why (with numbers)

### Takeaway
Plain propagation DFS fails on any board with N >= ~1200 cells or on dense/narrow boards because it does not learn from failed global-connectivity choices; CDCL with lazy subtour cuts solves almost everything and fast, but yields long arbitrary loops; short loops on big sparse boards come from the face-flip rubber-band shrinking (+8.65 average points v5->v6), while windowed DFS "LNS" helps mainly mid-size boards. Stronger/earlier cuts (v3, v4) are instance-dependent, with no consistent gain.

### Cited Findings
**Plain DFS (v1) failed.**
- 12/33 invalid, mean score 38.6 ([scores_astra.jsonl](/home/server/coding/competitions/QRT-Hackathon/p4/scores_astra.jsonl) line 1); the 21 valid ones still have poor quality: the four 25x25 tests got L = 366, 366, 358, 368 although best-known are 212, 116, 78, 58 (scores 33.6, 8.3, 4.7, 1.7); 12x12 d50: 86 vs 44, d5: 56 vs 28. The first loop found is arbitrary and the single follow-up branch-and-bound only runs if `elapsed<0.8` and ends at 0.85 s ([sol_v1_astra.cpp:258-261](/home/server/coding/competitions/QRT-Hackathon/p4/sol_v1_astra.cpp)).
- Cause: no learning (only an `activity` counter), full O(N) connectivity and variable scan per node ([sol_v1_astra.cpp:120-171](/home/server/coding/competitions/QRT-Hackathon/p4/sol_v1_astra.cpp), [sol_v1_astra.cpp:188-194](/home/server/coding/competitions/QRT-Hackathon/p4/sol_v1_astra.cpp)), and the connectivity pruning only fires after the cycle forms (premature-closure rule [sol_v1_astra.cpp:145-155](/home/server/coding/competitions/QRT-Hackathon/p4/sol_v1_astra.cpp) plus reachability flood fill every 16 nodes). On 13_20x15_d100 (66 pearls, density 22 %) SAT solves it in 0.011 s with 33 cut rounds, DFS never in 1.52 M nodes. Narrow boards with sparse pearls also kill it (6x50_d25: 1.53 M nodes, no loop; but the dense 6x50_d100 with a nearly forced loop took 155 nodes, 0.0015 s).
- Where v1's DFS does work: tiny and forced instances (all 6x6, 8x10, 12x12_d100: 0.0-0.6 s, optimal), e.g. d100 tests where pearls force the loop.

**CDCL + lazy subtour cuts (v2) worked for feasibility.**
- 32/33 solved (only 26_40x40_d50 fails); 13_20x15_d100 in 0.16 s total; mean 77.8 vs 38.6. Failed/long cases are the large mid-density ones (30_50x50_d50 6.05 s).
- Quality problem: first loop is arbitrary, e.g. 23_30x40_d20 L=636 (best 148), 27_40x40_d20 496 (236), 30_50x50_d50 1274 (616), 31_50x50_d20 614 (328), 25_40x40_d100 528 (412). Cause: the SAT model has no length objective and the windowed DFS (4-12 sized windows, 1200 nodes each) can only fix local detours; on 50x50_s113_d3 v5 gets 1030 vs 342 after face flips.
- The margin-2 bounding-box restriction ([sol_v2_astra.cpp:333-337](/home/server/coding/competitions/QRT-Hackathon/p4/sol_v2_astra.cpp)) is a heuristic that speeds the first call (0.30 s) but not complete; the fall-back call without margin is the only safe one.

**v3 boundary cuts: net negative.** 4 failures vs 1 and mean 70.8 vs 77.8 (re-run 70.5 vs 85.4). v3 is better on a few mid-size tests (14_20x15_d50: 76 vs 84; 13_20x15_d100 144 vs 152) but much worse on 26/27/29/30 (fail) and slower (21_30x40_d100 1.88 s, 25_40x40_d100 2.20 s). Mechanism (hypothesis, not measured separately): the boundary clause has hundreds of literals for big components and the cut is added for all partial components, increasing watch-list work.

**v4 in-search theory: helps some instances, kills others.** v4 fails 13_20x15_d100 (7.2 s) that v2/v3 solve in 0.2 s. My ablation on the v6 code (single run each, theory forced on or off for both SAT calls; loaded machine; seconds to first valid loop):

| test | theory OFF | theory ON |
|---|---|---|
| 13_20x15_d100 (N=300) | 0.027 | 1.035 (two calls timed out at 0.30 s first) |
| 17_25x25_d100 | 0.042 | 0.044 |
| 21_30x40_d100 | 0.164 | 0.026 |
| 26_40x40_d50 | 0.980 | 0.018 |
| 29_50x50_d100 | 0.742 (first call timed out at 0.30 s) | 0.071 |
| 30_50x50_d50 | 0.264 | 1.086 (two timeouts) |

Heavy-tailed, instance-specific behaviour. This is why v5 uses the small/large split plus restarts (changing both theory and variable order) instead of one setting; v5 then solved all 48 with first valid loop usually < 0.8 s.

**v5 randomised restarts.** Fixes feasibility (0 failures over 48 tests; both repo runs and my rerun), costs nothing when the first call succeeds. The restart loop discards learned clauses each time (a fresh CDCL per `satSearch`), visible in 50x50_s113_d100 where 5 attempts of 0.6-1.0 s each were thrown away in my loaded run.

**v6 face flips: the largest single gain.** v5 -> v6, same SAT stage, mean 86.46 -> 95.11 (+8.65) over 48 tests; per test (repo jsonl, v5 vs v6 score): 50x50_s113_d3 9.2 -> 83.2, 35x48_s109_d3 30.8 -> 100, 35x48_s109_d25 41.2 -> 100, 24_30x40_d5 49.0 -> 100, 23_30x40_d20 74.0 -> 100, 27_40x40_d20 52.4 -> 81.1, 31_50x50_d20 47.5 -> 89.9, 29_50x50_d100 90.1 -> 99.3, 30_50x50_d50 74.9 -> 99.4, 50x50_d25 79.2 -> 92.9, 32_50x50_d5 38.1 -> 50.1. Mechanism: the SAT loop contains a lot of slack; a few thousand face moves each cost O(1) and shrink it 20-72 % in 4-22 ms (examples in section 1, e.g. 1386 -> 378, 250 -> 194, 492 -> 278). LNS windows could not do this within 0.12 s and 1200-node windows (v5 left 1030 on the same test).
- It is not a clear win everywhere: 14_20x15_d50 70.7 -> 67.6 (L 88 -> 90 vs optimum 74), 16_20x15_d5 100 -> 81.0 (36 -> 40), 18_25x25_d50 93.4 -> 79.6 (120 -> 130), 25_40x40_d100 98.1 -> 88.5 (416 -> 438), 26_40x40_d50 100 -> 91.6, 17_25x25_d100 98.1 -> 96.3. Plausible cause (hypothesis, consistent with ordering in [sol_v6_astra.cpp:484-490](/home/server/coding/competitions/QRT-Hackathon/p4/sol_v6_astra.cpp)): the greedy face phase drives the loop into a local optimum that the 4-12 window DFS cannot escape within the remaining time, whereas in v5 the LNS started from the raw SAT loop. Not tested separately.
- The face phase is very cheap: the `stale>20000+20N` rule ends it after a few ms ([sol_v6_astra.cpp:409](/home/server/coding/competitions/QRT-Hackathon/p4/sol_v6_astra.cpp)); the total v6 time is about 0.77 s on big boards, 0.2-0.3 s on mid-size ones.

**DFS inside LNS for small boards.** `if N<=100` v2+ run a bounded B&B DFS (limit 0.40 s). All N<=100 tests reach the optimum (6x6: 14/16/14/10; 8x10: 40/28/24/20) in <= 0.33 s in every version v2..v6.

### Inferences
- Why DFS fails: with sparse pearls the loop has huge freedom, so DFS commits to cell shapes far from any pearl and discovers a sub-loop/unreachable pearl only deep in the tree; with dense pearls on moderate boards the propagation is not strong enough to see that a closed subloop is forced. CDCL learns the subtour cuts (19-39 cut rounds in 10-600 ms on the hard cases) and thereby jumps over these dead-ends.
- Why SAT loops are long: no objective; any loop through all pearls is fine. Why face flips fix that: shrinking by local bypasses of used faces is exactly the loop-shortening move, and on sparse boards the slack is mostly empty detours.
- Why face flips plateau: they cannot re-route between pearls (opposite-side faces are excluded, pearl legality is checked locally) so the macro topology of the SAT loop stays; compare 32_50x50_d5 final 178 vs 126 reached by the non-astra `sol_v2.cpp`, and 50x50_s113_d3 342 vs 312 by v7.

### Gaps
- No clean ablation of boundary cuts and theory under idle-machine conditions in the repo; my ablation is single-run on a loaded box.
- "SA face-flip" in the astra family is only the nearly-greedy variant of v6; I found no astra experiment with a proper temperature schedule (the plan doc leaves SA vs LNS as an open empirical question).

## 4. What do cdcl_astra.txt, sat_methods_astra.txt, fastestImplementationReasonAstra claim, measured vs speculative

### Takeaway
The three `.txt` files are code excerpts with no prose claims: `cdcl_astra.txt` = the `CDCL` class (v2's class minus the `ready` flag, 84 lines), `sat_methods_astra.txt` = `satSearch(end, margin)` (v2 form), `flips_astra.txt` = `localImprove` (v2) + `faceImprove` (v6); they were later removed from the working tree. `fastestImplementationReasonAstra` is an explicitly unmeasured plan: its mathematical statements check out, its architectural recommendation (specialised DFS, no general SAT) was contradicted by the repo's own measurements.

### Cited Findings
Claims in [fastestImplementationReasonAstra:1-202](/home/server/coding/competitions/QRT-Hackathon/p4/fastestImplementationReasonAstra) and their status:
- It "is NOT a proven fastest solver"; no benchmark numbers are claimed ([fastestImplementationReasonAstra:7](/home/server/coding/competitions/QRT-Hackathon/p4/fastestImplementationReasonAstra), [fastestImplementationReasonAstra:196-202](/home/server/coding/competitions/QRT-Hackathon/p4/fastestImplementationReasonAstra)). Status: honest; consistent with data.
- Describes `sol_v1.cpp` (non-astra): initial DFS 0.3 s, annealing to 0.85 s, later feasibility search to 8.5 s, may output its starting rectangle ([fastestImplementationReasonAstra:17](/home/server/coding/competitions/QRT-Hackathon/p4/fastestImplementationReasonAstra)). Verified by grep in `sol_v1.cpp`: `TE` default 0.3 (line 518), `dfs::solve(8.5)` (524), `sa::run(... 0.85 ...)` (529), "SA from the border rectangle" (532). Measured ([scores.jsonl](/home/server/coding/competitions/QRT-Hackathon/p4/scores.jsonl) runs 1-2): that file fails 9/33 (13_20x15_d100, 21-23, 25-27, 29, 30; mean 63.1 / 60.4 over 33 incl. fails).
- Generator facts: the 33 inputs are 1 sample + 8 sizes x 4 density settings, `d` is a sampling probability on eligible loop cells, not board density ([fastestImplementationReasonAstra:21](/home/server/coding/competitions/QRT-Hackathon/p4/fastestImplementationReasonAstra), [fastestImplementationReasonAstra:188](/home/server/coding/competitions/QRT-Hackathon/p4/fastestImplementationReasonAstra)). Verified against `gen.py` ([gen.py:91-107](/home/server/coding/competitions/QRT-Hackathon/p4/gen.py)): actual pearl fraction of cells is 0.3 %-22 % in `tests/` (e.g. 40x40_d100 has 163 pearls = 10 %, 50x50_d100 204 pearls = 8 %).
- Seven-state domains + explicit pearl propagation + trail undo ([fastestImplementationReasonAstra:23-46](/home/server/coding/competitions/QRT-Hackathon/p4/fastestImplementationReasonAstra)): implemented in v1 ([sol_v1_astra.cpp:39-92](/home/server/coding/competitions/QRT-Hackathon/p4/sol_v1_astra.cpp)). Whether domains beat an edge representation was left as "must earn their place by ablation" ([fastestImplementationReasonAstra:19](/home/server/coding/competitions/QRT-Hackathon/p4/fastestImplementationReasonAstra)): never ablated; instead v2 moved to edge variables in SAT.
- O(1) endpoint merging / path summaries instead of full connectivity ([fastestImplementationReasonAstra:48-56](/home/server/coding/competitions/QRT-Hackathon/p4/fastestImplementationReasonAstra)): **not implemented** in v1; `connectivity()` rebuilds a union-find over all cells per node ([sol_v1_astra.cpp:121-133](/home/server/coding/competitions/QRT-Hackathon/p4/sol_v1_astra.cpp)). Unmeasured whether it would have helped.
- Periodic possible-edge flood fill, "run selectively" ([fastestImplementationReasonAstra:58](/home/server/coding/competitions/QRT-Hackathon/p4/fastestImplementationReasonAstra)): implemented every 16th node ([sol_v1_astra.cpp:156-169](/home/server/coding/competitions/QRT-Hackathon/p4/sol_v1_astra.cpp), [sol_v1_astra.cpp:180](/home/server/coding/competitions/QRT-Hackathon/p4/sol_v1_astra.cpp)).
- Variable selection via incremental candidate sets, avoid scanning all 2500 cells ([fastestImplementationReasonAstra:62](/home/server/coding/competitions/QRT-Hackathon/p4/fastestImplementationReasonAstra)): v1 scans all N cells each node ([sol_v1_astra.cpp:188](/home/server/coding/competitions/QRT-Hackathon/p4/sol_v1_astra.cpp)).
- Lower bounds (|F|, 2*max colour count, 2*(row span + col span)) are safe to prune ([fastestImplementationReasonAstra:76-90](/home/server/coding/competitions/QRT-Hackathon/p4/fastestImplementationReasonAstra)): proofs are correct (checkerboard parity and bounding-box walk arguments); implemented in v1 ([sol_v1_astra.cpp:177-179](/home/server/coding/competitions/QRT-Hackathon/p4/sol_v1_astra.cpp), [sol_v1_astra.cpp:247](/home/server/coding/competitions/QRT-Hackathon/p4/sol_v1_astra.cpp)). In practice they only trigger usefully on small boards because the search never reaches an incumbent on big ones.
- Do not spend time proving optimality; stop by the formula ([fastestImplementationReasonAstra:92](/home/server/coding/competitions/QRT-Hackathon/p4/fastestImplementationReasonAstra), [fastestImplementationReasonAstra:120-156](/home/server/coding/competitions/QRT-Hackathon/p4/fastestImplementationReasonAstra)): numbers verified by recomputation: v(2)=0.909365, v(3)=0.835160, v(5)=0.724664, v(7.5)=0.636266; breakeven length reductions 4.64 %, 8.61 %, 14.87 %, 20.23 %; v(T)>0.7 iff T<5.581 s; quality table 1.1 -> 0.826, 1.2 -> 0.694, 1.25 -> 0.64, 1.5 -> 0.444. All correct. Consistent with the astra practice: improvement ends at 0.77-0.85 s (<1 s free window) except when the first loop is late.
- "Local rerouting (windows) vs annealing is empirical; cheap valid face flips may be retained if better" ([fastestImplementationReasonAstra:96](/home/server/coding/competitions/QRT-Hackathon/p4/fastestImplementationReasonAstra)): measured: windowed DFS (v2-v5) < face flips on big sparse boards (+8.65 mean points in v6) but windows catch things flips miss on mid boards (section 3). The doc's hedge was right.
- Table claiming a specialised DFS beats generic SAT/IP ("Mature clause learning may outperform this DFS; no universal dominance asserted", [fastestImplementationReasonAstra:165](/home/server/coding/competitions/QRT-Hackathon/p4/fastestImplementationReasonAstra)): contradicted by measurement: CDCL (v2) solves 11 of the 12 instances DFS (v1) could not, mean 77.8 vs 38.6; v1 is the implementation of the recommended design. The doc's caution turned out to be the true outcome.
- "An unbounded learned-clause store ... is not part of this plan" ([fastestImplementationReasonAstra:114](/home/server/coding/competitions/QRT-Hackathon/p4/fastestImplementationReasonAstra)): v2+ do store learned clauses without deletion ([sol_v2_astra.cpp:28](/home/server/coding/competitions/QRT-Hackathon/p4/sol_v2_astra.cpp), [sol_v2_astra.cpp:86](/home/server/coding/competitions/QRT-Hackathon/p4/sol_v2_astra.cpp)). No measured memory problem within <=7 s.
- Row-frontier DP ([fastestImplementationReasonAstra:166](/home/server/coding/competitions/QRT-Hackathon/p4/fastestImplementationReasonAstra)): never tried in the astra family. Unmeasured.
- Measurement protocol advice ([fastestImplementationReasonAstra:171-192](/home/server/coding/competitions/QRT-Hackathon/p4/fastestImplementationReasonAstra)): repeated trials, held-out tests, include whole-process runtime. The repo satisfied some of it (astra test set, own runner), but each version was run once per test, and timing was contaminated by other processes (section 2).
- Arithmetic facts in section 6 ([fastestImplementationReasonAstra:98-116](/home/server/coding/competitions/QRT-Hackathon/p4/fastestImplementationReasonAstra)): E = 4900 grid edges at 50x50 = 50*49+49*50, 7 x 2500 = 17500 removals, 2^100 = 1.27e30, entries 2500 vs 25. All correct.

### Inferences
- The document is a plan written before the first astra solver (file mtime 18:55 vs `sol_v1_astra.cpp` 19:04); every performance statement in it is conditional, and the only measured conclusions (SAT > DFS) went against its main recommendation.

### Gaps
- No `.txt` file contains a numeric claim to check; the code excerpts match v2/v6 source (I diffed mentally against [sol_v2_astra.cpp:16-99](/home/server/coding/competitions/QRT-Hackathon/p4/sol_v2_astra.cpp), [sol_v2_astra.cpp:313-365](/home/server/coding/competitions/QRT-Hackathon/p4/sol_v2_astra.cpp), [sol_v6_astra.cpp:396-438](/home/server/coding/competitions/QRT-Hackathon/p4/sol_v6_astra.cpp); the removed files can still be recovered from git history if committed).

## 5. Remaining weaknesses: where do the points go (v6, snapshot Lref)

### Takeaway
v6 loses 234.8 of 4800 possible points (4.9 %/test) over 48 tests: 223.4 from quality, 11.4 from runtime. Biggest holes: large sparse boards (32_50x50_d5 loses 49.9 points), mid-size 20x15/25x25 boards where early stopping leaves 0.5-0.7 s free time unused, and large medium-density boards.

### Cited Findings
Top losses for v6 ([scores_astra.jsonl](/home/server/coding/competitions/QRT-Hackathon/p4/scores_astra.jsonl) line 8):
| test | v6 L (T) | Lref | quality | speed | points lost |
|---|---|---|---|---|---|
| 32_50x50_d5 (12 pearls) | 178 (0.77 s) | 126 | 50.1 | 1.00 | 49.9 |
| 14_20x15_d50 (23 pearls) | 90 (0.29 s) | 74 (proven optimal) | 67.6 | 1.00 | 32.4 |
| 18_25x25_d50 | 130 (0.77 s) | 116 (proven optimal) | 79.6 | 1.00 | 20.4 |
| 16_20x15_d5 | 40 (0.26 s) | 36 (proven optimal) | 81.0 | 1.00 | 19.0 |
| 27_40x40_d20 | 262 (0.78 s) | 236 | 81.1 | 1.00 | 18.9 |
| 50x50_s113_d3 | 342 (0.78 s) | 312 | 83.2 | 1.00 | 16.8 |
| 50x50_s113_d100 (840 pearls) | 2162 (2.29 s) | 2160 | 99.8 | 0.886 | 11.6 (all but 0.2 from time) |
| 25_40x40_d100 | 438 (0.77 s) | 412 | 88.5 | 1.00 | 11.5 |
| 31_50x50_d20 | 346 (0.78 s) | 328 | 89.9 | 1.00 | 10.1 |
| 10_12x12_d50 | 46 (0.11 s) | 44 (optimal) | 91.5 | 1.00 | 8.5 |

Mean v6 score by group (48 tests minus the sample = 47): by size N=R*C: N<=150 (12 tests) 99.3; 151-700 (17) 94.9; 701-1600 (8) 95.0; >1600 (10) **90.1** (v5: 59.1 on this group). By generator density parameter (family A = `tests/`, family B = `tests_astra/`): A d100 98.0, A d50 **91.0**, A d20 95.8, A d5 **91.4**; B d100 97.3, B d25 98.1, B d3 95.7. So the weakest cells of the grid are sparse-to-medium (few pearls, 12-100) boards of 20x15 to 50x50 and the largest boards; dense boards are essentially solved (loop is almost forced).
- Time: only 50x50_d100 exceeds 1 s (2.29 s, speed 0.886) in v6; every other test is at or below 0.78 s, so the free 1-second window is not fully used: the `localImprove` 250-failure cap ends many mid boards at 0.26-0.4 s (14_20x15_d50 stops at 0.29 s with L=90 vs optimum 74; 10_12x12_d50 at 0.11 s with 46 vs 44), i.e. about 0.6-0.9 s of free search is thrown away exactly where L is still 10-25 % above optimum. Speed multiplier costs for comparison: 1.5 s 0.952, 2 s 0.909, 3 s 0.835.
- Dense, huge: 50x50_s113_d100 needs 2.3 s (loaded: up to 4.5 s) because the SAT stage needs about 80 model/cut rounds with the unshared clause database being discarded between restarts ([sol_v5_astra.cpp:433-440](/home/server/coding/competitions/QRT-Hackathon/p4/sol_v5_astra.cpp)); the 35x48_s109_d25 case needs 1.2-1.3 s in my loaded runs.
- Run-to-run variance (section 2) is 10-25 % on d20 boards, so on the judge's unknown CPU these sparse/medium results will move.
- Compare with other implementations in the repo (same Lref): non-astra `sol_v2.cpp` has L = 126 on 32_50x50_d5 and 44 on 10_12x12_d50; astra v7 has 312 on 50x50_s113_d3 and 116 on 18_25x25_d50 — the weak cells are known to be fixable by other techniques in the repo (shortcut moves in v7, SA from a border rectangle in `sol_v1/v2.cpp`).
- The hidden set has 100 tests of "increasing size and difficulty" ([statement.md: Constraints](/home/server/coding/competitions/QRT-Hackathon/p4/statement.md)), probably with s=0.9 on the biggest; the astra local tests where v6 is weakest (>=2000 cells) are the likely bulk of the late hidden tests.

### Inferences
- Remaining headroom for v6 is mostly (a) sparse large boards (macro re-routing between far pearls; face flips cannot do it), (b) not cutting the improvement phase at 250 failures/0.77 s when free time up to 1.0 s remains, (c) reducing SAT time for dense 50x50 (keep clauses across restarts), and (d) reducing sensitivity to CPU speed.
- Estimated potential (estimate, not measured): if the top-6 rows reached Lref, v6 would gain about 157 points / 48 tests = +3.3 mean.

### Gaps
- True optimal lengths for the big boards (unknown) and the hidden-test mix; results may differ materially on the judge.
- I did not run v7+ (they changed during my work).
