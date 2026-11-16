# Masyu (p4): scoring economics, instance structure, exact-method feasibility, measured losses

Scope: quantitative research only. All "sources" below are the repo files (read-only) and my own scripts, which live in `/tmp/claude-1000/-home-server-coding-competitions/7aa75855-1aa5-43ba-9bc6-4dfd2d2f50cb/scratchpad/quant/` (`p1.py`, `stop.py`, `dp.py`/`dp2.py`, `satexp.py`, `propbench.py`, `probe.py`, `exact.py`, `final.py`, `classes.py`, `worst.py`, `qan.py`). No web research was needed or used. The score files `p4/scores.jsonl` and `p4/scores_astra.jsonl` were being appended to by another agent while I worked (scores_astra.jsonl grew from 6 to 16+ lines; `sol_v6`..`sol_v10`, `sol_astra.cpp` appeared), so I took a snapshot at the end; numbers marked "snapshot" can drift. Estimates are marked **(est.)**. Statement: `p4/statement.md` (read in full).

Statement facts used: `score = 100*min(1, s*Lref/L)^2 * (0.5+0.5*exp(-max(0,T-1)/5))`, s=1 when Lref is proven optimal, s=0.9 on "a few large ones"; 6<=R,C<=50; >=2 pearls; 100 hidden tests "of increasing size and difficulty"; 10 s hard limit (timeout = 0); invalid = 0.

## Q1. Scoring maths, break-even between time and length, stopping policy

### Takeaway
The first second is free (speed = 1.000 for T<=1); after that one extra second at T=1 costs 9.1% of the score (speed 1.000 -> 0.909), so the loop must get 4.64% shorter (quality is squared) to pay for it, falling to 3.7% at T=3 and 2.85% at T=5. Optimal policy: use up to ~0.85-0.95 s wall time, then stop unless the solver can show (or reliably expects) >~5% length improvement per extra second; a valid-but-25%-longer loop (64 pts) still beats an invalid/timeout (0) by a huge margin, so validity first, quality second, speed third.

### Cited Findings
Formulas (derived from `p4/statement.md`; script `quant/p1.py`):
- speed(T) = 0.5 + 0.5*exp(-max(0,T-1)/5); quality q = min(1, s*Lref/L)^2; score = 100*q*speed.
- ln score = 2 ln(s*Lref) - 2 ln L + ln speed(T). Differentiating: d ln score = -2 dL/L + (speed'/speed) dT with speed'(T) = -0.1*exp(-(T-1)/5) for T>1.
- Marginal cost rate of time: r(T) = -d ln speed/dT = 0.1*exp(-(T-1)/5)/speed(T) (per second). Linearised break-even: an extra dT seconds is worth it iff relative length reduction dL/L >= r(T)*dT/2. Exact version for an extra dT: (1-d)^-2 = speed(T)/speed(T+dT), i.e. d = 1 - sqrt(speed(T+dT)/speed(T)).
- Marginal value of length: 1% shorter L = +2.0% of score (if still below the cap q<=1). At T=1, 0.1 s of extra time costs 1.0% of score (speed(1.1)=0.990) = 0.50% of L equivalent; so 1% of L is worth ~0.2 s at T=1.

Speed multiplier table (computed, `p1.py`):

| T (s) | speed | loss |
|---|---|---|
| 0.1 | 1.0000 | 0 |
| 0.5 | 1.0000 | 0 |
| 1 | 1.0000 | 0 |
| 1.5 | 0.9524 | 4.76% |
| 2 | 0.9094 | 9.06% |
| 3 | 0.8352 | 16.48% |
| 5 | 0.7247 | 27.53% |
| 7 | 0.6506 | 34.94% |
| 10 | 0.5826 | 41.74% (floor is 0.5; timeout at 10 s scores 0 anyway) |

Break-even length reduction (fraction of L that must disappear to justify one more second, and to justify 0.5 s, starting at runtime T):

| T | r(T) per s | linear dL/L per +1 s | exact per +1 s | exact per +0.5 s | exact per +0.1 s |
|---|---|---|---|---|---|
| 1 | 0.1000 | 5.00% | 4.64% | 2.41% | 0.496% |
| 2 | 0.0900 | 4.50% | 4.17% | 2.17% | 0.447% |
| 3 | 0.0803 | 4.01% | 3.71% | 1.93% | 0.398% |
| 4 | 0.0709 | 3.54% | 3.26% | 1.70% | 0.351% |
| 5 | 0.0620 | 3.10% | 2.85% | 1.49% | 0.307% |
| 7 | 0.0463 | 2.31% | 2.12% | 1.11% | 0.229% |
| 9 | 0.0336 | 1.68% | 1.53% | 0.80% | 0.166% |

Cumulative: finishing at T instead of 1 s requires L to be shorter by 2.41% (T=1.5), 4.64% (T=2), 8.61% (T=3), 14.87% (T=5), 19.34% (T=7), 23.67% (T=10).

Quality table (s=1): L 2% over Lref -> q 0.961; 5% -> 0.907; 10% -> 0.826; 15% -> 0.756; 20% -> 0.694; 25% -> 0.640; 30% -> 0.592; 50% -> 0.444; 100% -> 0.250. With s=0.9: matching Lref gives 0.81, 5% shorter than Lref gives 0.8975 (q=(0.9/0.95)^2), full 1.0 requires L <= 0.9*Lref. So on "best-found" tests, improving beyond Lref is capped at +19 points (81 -> 100); on proven-optimal tests the cap is reaching Lref exactly.

Overshoot penalties of a solver that targets 1 s but runs late: 1.3 s -> -2.9%; 2 s -> -9.1%; so keep a safety margin (the repo's runs finish at 0.77-0.94 s on a loaded laptop; judge CPU may be slower, so the time guard must be wall-clock based).

Optimal stopping under a model (est.; script `quant/stop.py`): if the excess over optimum decays as e(T) = e1*exp(-(T-1)/tau) (e1 = excess at T=1 s, tau = decay time), score(T) = 100*(1+e(T))^-2*speed(T). Optimal stopping time T*:

| e1 (excess at 1 s) | tau=0.5 | tau=1 | tau=2 | tau=4 | tau=8 |
|---|---|---|---|---|---|
| 2% | 1.0 | 1.0 | 1.0 | 1.0 | 1.0 |
| 5% | 1.35 (+1.3 pts) | 1.0 | 1.0 | 1.0 | 1.0 |
| 10% | 1.7 (+6.4) | 1.7 (+2.2) | 1.0 | 1.0 | 1.0 |
| 20% | 2.1 (+16.8) | 2.5 (+10.3) | 2.55 (+3.2) | 1.0 | 1.0 |
| 40% | 2.45 (+32.7) | 3.3 (+24.4) | 4.45 (+14.4) | 5.4 (+4.1) | 1.0 |

(entries: T* in s, and points gained over stopping at 1 s.)

### Inferences
- Policy: (1) always hold a valid loop (0 vs >=40 points dwarfs everything: a loop 100% longer than optimal still scores 25 pts). (2) Spend T<=~0.9 s improving; the budget is free. (3) Beyond 1 s extend only when expected improvement rate exceeds r(T)/2 per second (~5%/s at T~1, 4.5%/s at T=2, 4%/s at T=3), i.e. when the solver is clearly not converged (excess >= ~10% with fast decay). Observed excess of the repo's 0.8 s solutions (see Q5) is mean ~4.9-5.5%, median 0%, but up to 56% on some large sparse tests, so an adaptive rule (e.g. compare L against a lower bound; Q3) is valuable: stop immediately when L <= 1.05*LB, extend (to ~2-3 s) when L >= 1.2*LB and still dropping.
- The "no valid loop yet" case must use the slow path up to ~8-9 s: any valid loop at T=9 scores >= 100*q*0.60; at 0 it's nothing.
- The cost of the slow path is bounded: even at T=10 you keep 58% of a perfect-length score.

### Gaps
- True per-instance improvement curves L(T) of a given solver (tau, e1) are solver-specific; I only modelled them. The judge machine speed relative to this laptop is unknown (the tests run here under load average ~17 from concurrent agents, so my timings are pessimistic).


## Q2. Hidden test distribution: instance classes (size x density) and likely score shares

### Takeaway
The repo has two generators: `gen.py` (random planted loop, pearls only on cells that the planted loop makes eligible; 8 sizes x 4 densities; pearl density 0.3%-22% of cells, best-known loops cover 3.6%-50% of cells) and `gen_astra.py` (planted loop that fills ~97% of the board with "ears"; pearl density 0.8%-38%; shortest loops cover 10%-93% of cells). If the 100 hidden tests are spread evenly by size, ~54% (linear in side length) to ~73% (linear in cell count) of them have >700 cells, and those large tests account for 75-90% of the points currently lost.

### Cited Findings
- `p4/gen.py`: grows a random simple loop by face flips (`steps = R*C`), then puts a white pearl with probability `dens*(1-bw)*2` on straight loop cells with at least one non-straight loop neighbour and a black pearl with probability `dens*bw*2` on corner cells whose two loop neighbours are straight (`bw=0.5`, so probability = `dens`). Sizes: 6x6, 8x10, 12x12, 20x15, 25x25, 30x40, 40x40, 50x50, densities 1.0/0.5/0.2/0.05; the "dens" parameter is the fraction of eligible cells that get pearls, not the fraction of grid cells.
- `p4/gen_astra.py`: planted loop = board perimeter with random 3-edge "ears"; witness lengths 288 (50x6), 298 (6x50), 390 (20x20), 1634 (35x48), 2420 (50x50), i.e. 96-98% of all cells; pearls on eligible cells with probability 1.0/0.25/0.03. The `.out` witness is far from shortest on sparse files (e.g. 50x50 d3: witness 2420 vs best-known 312).
- Measured pearl statistics (snapshot, `quant/qan.py`; "L/cells" uses best-known loop length):

| generator, dens param | #tests | pearls per cell: mean (min-max) | best-known L / cells (mean) | black share of pearls |
|---|---|---|---|---|
| gen d100 | 8 | 0.154 (0.082-0.220) | 0.371 | 20% |
| gen d50 | 8 | 0.088 (0.050-0.163) | 0.273 | 21% |
| gen d20 | 8 | 0.038 (0.021-0.083) | 0.185 | 20% |
| gen d5 | 8 | 0.018 (0.003-0.056) | 0.134 | 14% |
| astra d100 | 5 | 0.350 (0.329-0.383) | 0.869 | 15% |
| astra d25 | 5 | 0.094 (0.078-0.117) | 0.446 | 17% |
| astra d3 | 5 | 0.013 (0.008-0.027) | 0.190 | 13% |

  Pearl counts range from 2 (6x6 and 8x10 d5) to 840 (50x50 astra d100).
- Statement hints (`p4/statement.md`): "densely pearled puzzles hard to find any loop; sparsely pearled hard to find short loop"; "Lref ... on most test cases proven the shortest; a few large ones only best found (s=0.9)". Statement's sample: 6x6 with 4 pearls, shortest 18 = 50% of cells.
- Hardness of the classes, measured with a pure CP/SAT probe: unit-propagation + failed-literal probing (quant/probe.py, PySAT minisat22) fixes only 6.2% of the 4900 edge variables on `29_50x50_d100`, 0.6% on d50, 0.1% on d20/d5, but 45% on `50x50_s113_d100_astra` (pearls = 34% of cells). So generator-style "dense" cases (8-22% pearls) are far from solved by propagation, while astra-style d100 is propagation-heavy but with huge search space for length.
- Current per-class scores (Lref = best-known over all repo runs plus my CP-SAT results; script `quant/classes.py`, snapshot; excludes `00_sample`), for the two strongest 33/48-test runs: `sol_v4.cpp` (33 gen tests) and `sol_v6_astra.cpp` (48 tests):

| class (cells x density) | n (v4/v6) | sol_v4.cpp avg score | sol_v6_astra.cpp avg score |
|---|---|---|---|
| S <=150 cells, dense | 3 / 3 | 100.0 | 100.0 |
| S, mid | 6 / 6 | 98.6 | 98.6 |
| S, sparse | 3 / 3 | 100.0 | 100.0 |
| M 151-700, dense | 2 / 5 | 88.5 | 98.6 |
| M, mid | 4 / 7 | 87.3 | 90.0 |
| M, sparse | 2 / 5 | 100.0 | 94.5 |
| L >700, dense | 3 / 5 | 66.5 | 93.0 |
| L, mid | 6 / 8 | 88.8 | 84.1 |
| L, sparse | 3 / 5 | 92.3 | 84.8 |
| by size S / M / L | | 99.3 / 90.8 / 84.1 | 99.3 / 93.9 / 86.8 |

  (dense = d100 files, mid = d50/d25/d20, sparse = d5/d3. Note the two columns are on different test sets: v4 only has the 32 gen tests, v6 also the 15 astra tests, so compare them only via Q5's like-for-like numbers.)

### Inferences
- Size weights if tests increase evenly: with side length linear from 6 to 50, cells<=150 (side<=12.2) is ~14 tests, 151-700 ~32, >700 ~54 (est.). If cell count rises linearly 36 -> 2500 then ~5 / 22 / 73 tests (est.). Weighting the class averages above gives a hidden-set estimate for the current best solution (`sol_v6_astra.cpp`) of ~90.8 (linear side) or ~89.0 (linear area), versus ~88.4 / 86.3 for `sol_v4.cpp`; large (>700 cells) tests account for ~78% / ~87% of the lost points (74% / 85% for v4). Small tests (<=12x12) are essentially solved (99.3) by every recent solution, so effort should go to >=25x25.
- Within large tests the loss mechanism differs by density: dense = finding any loop quickly and then shortening (v4 loses 52 points on `25_40x40_d100`, 598 vs 412), sparse = shortening a loop that is easy to find (v6 loses 59 points on `32_50x50_d5`, 178 vs 114 although only 12 pearls). Because sparse puzzles are TSP-like (Q3), a pearl-tour construction should produce near-optimal loops there with almost no search.
- Real hidden puzzles are probably unique-solution style (statement tips about "three whites in a row", black next to border) with pearl density likely higher than gen.py's sparse cases for "dense" ones; I could not verify this (no hidden data), so treat the generator mix as an estimate only.
- During my session other agents added `p4/tests/33..47_*` (fill/black/white/sparse variants, e.g. `42_50x50_fill_d70`, `46_50x50_sparse2`); these are not in the score files and were not analysed.

### Gaps
- Actual hidden density/size distribution, black/white ratio and whether Lref comes from an exact solver (CP-SAT/MIP) are unknowable from the repo.
- Which tests are the s=0.9 ones ("a few large ones") is unknown.


## Q3. Lower bounds and best-known lengths per test; reliability of "best known"

### Takeaway
For sparse puzzles (<=15 pearls) a pearl-tour/bounding-box lower bound is nearly tight: best-known L / max(bbox, TSP) is 1.00-1.17 (median 1.14 over all 21 tests with <=15 pearls, only 1.04-1.09 on the large sparse ones). For dense puzzles geometric bounds are weak (best/bound 1.2-2.05) and only the CP-SAT bound is informative. "Best known" is NOT reliable on large tests: it fell 21-22% on `27_40x40_d20` and `30_50x50_d50` after the first solver wave, and my CP-SAT (40 s) beat the repo best on five tests (`21_30x40_d100`: 360 < 370, `35x48_s109_d100_astra`: 1354 < 1362, `50x6_s103_d3_astra`: 96 < 98, and proven optima `20x20_s107_d100_astra` 352 < 358 and `20x20_s107_d25_astra` 152 < 160).

### Cited Findings
Method. Valid lower bounds: (a) bbox: a closed rectilinear loop through pearls spanning rows r0..r1 and columns c0..c1 has at least 2*((r1-r0)+(c1-c0)) cells; (b) TSP: consecutive pearls on the loop are at least their Manhattan distance apart, so L >= optimal Manhattan TSP tour (exact Held-Karp DP for <=15 pearls; MST weight is a weaker bound for more pearls); (c) trivial: L >= #pearls, L even, L>=4; (d) CP-SAT best bound from `quant/exact.py` (OR-tools 9.15 `AddCircuit` model with optional nodes and pearl constraints, minimise #cells, 12 workers, 40 s per test while the machine was under load ~17, so strength is below what a quiet machine gives). Best-known = min over every valid 'v' in `p4/scores.jsonl` + `p4/scores_astra.jsonl` (snapshot taken at the end) and, where lower, the CP-SAT incumbent.
- CP-SAT sanity check: model reproduces the statement's sample (18) and agrees with the repo solvers on all small tests; proven optimal (status OPTIMAL) in <=40 s on all tests up to 20x15, plus `18_25x25_d50` (116) and the astra tests `20x20_s107_d100` (352), `20x20_s107_d25` (152), `50x6_s103_d100` (256), `6x50_s101_d100` (278): 22 tests proven optimal in total.
- First-wave best-known vs now: `10_12x12_d50` 46 -> 44 (opt, -4.3%), `18_25x25_d50` 120 -> 116 (opt, -3.3%), `27_40x40_d20` 262 -> 204 (-22.1%), `30_50x50_d50` 618 -> 488 (-21.0%), `32_50x50_d5` 126 -> 114 (-9.5%), `31_50x50_d20` 328 -> 308 (-6.1%), `50x50_s113_d25_astra` 1322 -> 1188 (-10.1%), `50x50_s113_d3_astra` 342 -> 312 (-8.8%). "First wave" = runs of sol_v1..v6.
- Exact (CP-SAT) vs best-known vs bound, per test (`quant/final.py`, snapshot; "not finished" = CP-SAT run had not reached it when I stopped; `UNKNOWN` = no feasible solution in 40 s):

```
00_sample                best=    18 cpsat=18.0 (OPTI) bound=18.0 gap_best_vs_bound=0.0% 
01_6x6_d100              best=    14 cpsat=14.0 (OPTI) bound=14.0 gap_best_vs_bound=0.0% 
02_6x6_d50               best=    16 cpsat=16.0 (OPTI) bound=16.0 gap_best_vs_bound=0.0% 
03_6x6_d20               best=    14 cpsat=14.0 (OPTI) bound=14.0 gap_best_vs_bound=0.0% 
04_6x6_d5                best=    10 cpsat=10.0 (OPTI) bound=10.0 gap_best_vs_bound=0.0% 
05_8x10_d100             best=    40 cpsat=40.0 (OPTI) bound=40.0 gap_best_vs_bound=0.0% 
06_8x10_d50              best=    28 cpsat=28.0 (OPTI) bound=28.0 gap_best_vs_bound=0.0% 
07_8x10_d20              best=    24 cpsat=24.0 (OPTI) bound=24.0 gap_best_vs_bound=0.0% 
08_8x10_d5               best=    20 cpsat=20.0 (OPTI) bound=20.0 gap_best_vs_bound=0.0% 
09_12x12_d100            best=    68 cpsat=68.0 (OPTI) bound=68.0 gap_best_vs_bound=0.0% 
10_12x12_d50             best=    44 cpsat=44.0 (OPTI) bound=44.0 gap_best_vs_bound=0.0% 
11_12x12_d20             best=    22 cpsat=22.0 (OPTI) bound=22.0 gap_best_vs_bound=0.0% 
12_12x12_d5              best=    28 cpsat=28.0 (OPTI) bound=28.0 gap_best_vs_bound=0.0% 
13_20x15_d100            best=   144 cpsat=144.0 (OPTI) bound=144.0 gap_best_vs_bound=0.0% 
14_20x15_d50             best=    74 cpsat=74.0 (OPTI) bound=74.0 gap_best_vs_bound=0.0% 
15_20x15_d20             best=    42 cpsat=42.0 (OPTI) bound=42.0 gap_best_vs_bound=0.0% 
16_20x15_d5              best=    36 cpsat=36.0 (OPTI) bound=36.0 gap_best_vs_bound=0.0% 
17_25x25_d100            best=   212 cpsat=226.0 (FEAS) bound=204.0 gap_best_vs_bound=3.9% 
18_25x25_d50             best=   116 cpsat=116.0 (OPTI) bound=116.0 gap_best_vs_bound=0.0% 
19_25x25_d20             best=    78 cpsat=80.0 (FEAS) bound=74.0 gap_best_vs_bound=5.4% 
20_25x25_d5              best=    58 cpsat=58.0 (FEAS) bound=49.0 gap_best_vs_bound=18.4% 
21_30x40_d100            best=   370 cpsat=360.0 (FEAS) bound=342.0 gap_best_vs_bound=5.3% 
22_30x40_d50             best=   248 cpsat=302.0 (FEAS) bound=226.0 gap_best_vs_bound=9.7% 
23_30x40_d20             best=   148 cpsat=158.0 (FEAS) bound=116.0 gap_best_vs_bound=27.6% 
24_30x40_d5              best=    70 cpsat=98.0 (FEAS) bound=46.0 gap_best_vs_bound=52.2% 
25_40x40_d100            best=   412 cpsat=none (UNKNOWN) 40s
26_40x40_d50             best=   402 cpsat=none (UNKNOWN) 40s
27_40x40_d20             best=   204 cpsat=none (UNKNOWN) 40s
28_40x40_d5              best=    58 cpsat=58.0 (FEAS) bound=35.0 gap_best_vs_bound=65.7% 
29_50x50_d100            best=   558 cpsat=none (UNKNOWN) 40s
30_50x50_d50             best=   488 cpsat=none (UNKNOWN) 40s
31_50x50_d20             best=   308 cpsat=none (UNKNOWN) 40s
32_50x50_d5              best=   114 cpsat=162.0 (FEAS) bound=75.0 gap_best_vs_bound=52.0% 
20x20_s107_d100_astra    best=   358 cpsat=352.0 (OPTI) bound=352.0 gap_best_vs_bound=0.0% 
20x20_s107_d25_astra     best=   160 cpsat=152.0 (OPTI) bound=152.0 gap_best_vs_bound=0.0% 
20x20_s107_d3_astra      best=    42 cpsat=42.0 (FEAS) bound=38.0 gap_best_vs_bound=10.5% 
35x48_s109_d100_astra    best=  1362 cpsat=1354.0 (FEAS) bound=1288.0 gap_best_vs_bound=5.1% 
35x48_s109_d25_astra     best=   738 cpsat=none (UNKNOWN) 40s
35x48_s109_d3_astra      best=   182 cpsat=430.0 (FEAS) bound=113.0 gap_best_vs_bound=61.1% 
50x50_s113_d100_astra    best=  2144 cpsat=none (UNKNOWN) 40s
50x50_s113_d25_astra     best=  1188 cpsat=none (UNKNOWN) 40s
50x50_s113_d3_astra      best=   312 cpsat=none (UNKNOWN) 40s
50x6_s103_d100_astra     best=   256 cpsat=256.0 (OPTI) bound=256.0 gap_best_vs_bound=0.0% 
50x6_s103_d25_astra      best=   122 cpsat=122.0 (FEAS) bound=102.0 gap_best_vs_bound=19.6% 
50x6_s103_d3_astra       best=    98 cpsat=96.0 (FEAS) bound=69.0 gap_best_vs_bound=39.1% 
6x50_s101_d100_astra     best=   278 cpsat=278.0 (OPTI) bound=278.0 gap_best_vs_bound=0.0% 
6x50_s101_d25_astra      best=   152 cpsat=152.0 (FEAS) bound=148.0 gap_best_vs_bound=2.7% 
6x50_s101_d3_astra       best=    86 cpsat=86.0 (FEAS) bound=45.0 gap_best_vs_bound=91.1%
```
- Simple geometric bounds vs best-known (tests with <=15 pearls, TSP exact): 

```
test pearls best tspLB best/tsp bbox
00_sample                  4     18    14    14 1.29
01_6x6_d100                5     14    10    10 1.40
02_6x6_d50                 4     16    14    14 1.14
03_6x6_d20                 3     14    10    10 1.40
04_6x6_d5                  2     10     6     6 1.67
06_8x10_d50               13     28    20    20 1.40
07_8x10_d20                6     24    24    24 1.00
08_8x10_d5                 2     20    16    16 1.25
10_12x12_d50              15     44    34    34 1.29
11_12x12_d20               4     22    20    20 1.10
12_12x12_d5                4     28    24    24 1.17
15_20x15_d20               8     42    36    36 1.17
16_20x15_d5                4     36    34    34 1.06
19_25x25_d20              13     78    74    74 1.05
20_25x25_d5                6     58    58    58 1.00
24_30x40_d5                6     70    70    70 1.00
28_40x40_d5                5     58    58    58 1.00
32_50x50_d5               12    114   110   110 1.04
20x20_s107_d3_astra        4     42    40    40 1.05
50x6_s103_d3_astra         8     98    90    90 1.09
6x50_s101_d3_astra         3     86    84    84 1.02
```
  Columns: test, #pearls, best-known L, TSP lower bound, max(TSP,bbox), best/LB. For >15 pearls (dense, 27 tests) best/max(bbox,MST) ranges 1.21-2.05 (median 1.69), so those bounds are useless for dense puzzles; pearl-cell constraints (black needs 5 cells in an L plus straight arms, white needs 3 collinear cells) dominate.
- Loop structure facts: the minimum possible loop is 4 (a 2x2 block) but any pearl forces more: a white pearl needs 3 collinear cells and one turning neighbour -> L >= 6; a black pearl needs a corner with 2-cell straight arms, i.e. 5 cells in an L (L >= 8, because the loop must close). The statement's sample (4 pearls) has L = 18, the 2-pearl 6x6 sparse test has L = 10 (= 6+4), the 3-pearl test L = 14.

### Inferences
- Reliability estimate: of the 22 tests where CP-SAT proved optimality (all <=25x25 or 6-wide strips), the final repo best-known equals the optimum on 20, and is off on `20x20_s107_d100_astra` (358 vs 352, +1.7%) and `20x20_s107_d25_astra` (160 vs 152, +5.3%); the first-wave solvers (v1..v6) were also off on `10_12x12_d50` (+4.5%) and `18_25x25_d50` (+3.4%). On tests >=30x40 the best-known has been moving by 5-22% between solver generations, so for large tests treat best-known as an upper bound on Lref and expect the organisers' own Lref (s=0.9 cases) to be similarly loose. Score estimates against a stale best-known overstate quality.
- For the sparse class, the TSP/bbox bound is the practical certificate: stop immediately if L <= ~1.03*LB (the 5%/s break-even from Q1 means it is not worth even 1 s more); for d>=20% classes use the structure (forced orientation) instead.
- CP-SAT (generic) is a good checker for <=20x15 but not competitive on >=30x40 (its incumbents are 3-60% worse than the repo's best on large sparse tests: e.g. `24_30x40_d5` 98 vs 70, `32_50x50_d5` 162 vs 114, `35x48_s109_d3` 430 vs 182) and its LP/CP bound is far weaker than the TSP/bbox bound on sparse tests (e.g. `6x50_s101_d3_astra`: CP-SAT bound 45, TSP/bbox bound 84, best 86) - consistent with Q4's conclusion that exact methods do not scale and LNS/constructive heuristics win at 2500 cells.

### Gaps
- True optimum for >=30x40 tests is unknown (CP-SAT bound gaps 5-66%). Proven bounds from a stronger MIP (e.g. with lazy subtour cuts and long runs) were not attempted within the time.


## Q4. Feasibility of exact methods (frontier DP, CDCL SAT, propagation speed)

### Takeaway
Full-grid frontier DP is impossible: states grow ~2.65-3x per extra column (measured to width 12), reaching ~1e22 at width 50 (Motzkin number M_51 = 8.3e21). A DP over a strip of height h costs ~0.05-0.2 s for h=8-9 and 0.5-2 s for h=10-11 across a 50-wide window (est.), so LNS windows should be at most ~8-10 rows x full width (or ~10x10..12x12 squares), not larger. Edge-variable SAT is small (4900 vars, 20-25k clauses for 50x50) and lazy subtour cuts needed 6-630 rounds to get *a* valid loop (not a short one); raw unit propagation runs ~6M literals/s through PySAT/minisat22 incl. Python overhead, so a full propagation pass over 4900 edge variables takes <1 ms.

### Cited Findings
(a) Frontier / plug DP (script `quant/dp.py`, own implementation: cell-by-cell broken profile, plug labels 0/open/close for single-loop connectivity plus per-plug flags "neighbour straight" and "next cell must be straight/turn" that encode black/white pearl rules; pearl layouts from `gen.py` `make()`, grid height W+3; max reachable states over the sweep; flags are stored uncompressed so the pearl numbers are an upper-ish estimate):
- Motzkin bound on pure connectivity states for width W is M_{W+1}: W=6: 127; 8: 835; 10: 5,798; 12: 41,835; 14: 310,572; 16: 2,356,779; 18: 18,199,284; 20: 142,547,559; 24: 9.04e9; 30: 4.86e12; 40: 1.92e17; 50: 8.27e21.
- Measured max states per row sweep (W = number of columns):

| W | M(W+1) | no pearls | gen d100 | gen d20 | gen d5 |
|---|---|---|---|---|---|
| 6 | 127 | 102 | 291 | 215 | 100 |
| 8 | 835 | 646 | 3,168 | 1,408 | 1,709 |
| 9 | 2,188 | 1,670 | 6,310 | 4,317 | 4,929 |
| 10 | 5,798 | 4,376 | 15,007 | 12,148 | 10,380 |
| 11 | 15,511 | 11,596 | 37,080 | 34,365 | 23,621 |
| 12 | 41,835 | 31,022 | 69,072 | 110,885 | 58,484 |

- Growth ratio per extra column from W=9 to 12: no pearls 2.65, d100 2.22, d20 2.95, d5 2.28 (asymptote 3). The pearl-free count is ~0.74*M(W+1); with flags 1.4-2.7*M(W+1) at W=12. Pearl constraints did not reduce reachable states on these random layouts (they prune loops but the extra flag information adds states); on d100 W=7 the count did drop below the empty case (170 vs 254).
- Fit: S(W) ~ 0.21*3^W (flagged, d20) or ~0.058*3^W (plain). At W=50 -> ~1.5e23 (flagged) / ~6e21 (plain). Not feasible in 1 s (or ever). Python timing of my DP is not representative; a C++ DP does ~20-50M transitions/s (est.).
- LNS window cost model (est.): transitions = S(h)*h*w*3 for an h x w window processed with frontier h+1; at 30M transitions/s: h=8, w=50: S~1.3k flagged (0.4k plain) -> 1.6M trans -> 0.05 s; h=9: 5.3M -> 0.18 s; h=10: 17.7M -> 0.59 s; h=11: 58M -> 1.9 s; h=12: 191M -> 6.4 s; h=14: 2e9 -> 67 s. With a 0.1-0.3 s window budget: h <= 8-9 at width 50, or a 10x10-ish block (S(10)=11.8k flagged x 10x10x3 = 3.5M trans = 0.12 s). A beam-pruned DP (keep best K states per cell) over the full grid costs K*N*3 transitions: K=2000 -> 15M trans ~0.5 s (est.), but loses optimality guarantees.

(b) CDCL SAT on an edge-variable encoding (script `quant/satexp.py`, PySAT minisat22; variables = grid edges; per cell: for each edge e: e -> OR(other edges) (4 clauses of size <=4) and no-three-edges (4 clauses of size 3); pearls add 3-9 clauses each; black: exactly one horizontal and one vertical edge + neighbour continues straight; white: both opposite edges equal + "straight on this axis -> some perpendicular edge at one of the two neighbours"):
- Size: vars E = 2RC - R - C (4900 for 50x50, 2330 for 30x40, 1200 for 25x25). Clauses measured: 19,297 (50x50, 12 pearls), 20,142 (126 pearls), 20,691 (204 pearls), 25,207 (840 pearls, astra d100) i.e. ~7.7 per cell + ~5-6 per pearl (4900 vars, 20-25k clauses). A cell-shape encoding (7 shape vars per cell = 17,500 vars for N=2500; one-hot 22 clauses + ~30 adjacency clauses per cell) is ~52 clauses/cell = ~130k clauses (est., not built), 5x larger, with no propagation advantage.
- Lazy subtour cuts (blocking clause per extra cycle, no length bound, first valid loop; loops found are long, ~80% of cells): rounds / time / conflicts / propagations (python-driven, cadical not used):

| test | vars | clauses | rounds | cuts | time s | conflicts | propagations |
|---|---|---|---|---|---|---|---|
| 13_20x15_d100 | 565 | 2,612 | 42 | 142 | 0.06 | 509 | 37,669 |
| 21_30x40_d100 | 2,330 | 10,129 | 23 | 241 | 0.17 | 2,172 | 182,637 |
| 25_40x40_d100 | 3,120 | 13,358 | 85 | 1,643 | 1.35 | 38,224 | 1.74M |
| 27_40x40_d20 | 3,120 | 12,472 | 144 | 2,979 | 2.44 | 94,653 | 3.54M |
| 29_50x50_d100 | 4,900 | 20,691 | 293 | 7,505 | 9.13 | 273,137 | 13.4M |
| 30_50x50_d50 | 4,900 | 20,142 | 630 | 16,915 | 20.2 | 565,346 | 29.1M |
| 31_50x50_d20 | 4,900 | 19,635 | 118 | 2,840 | 3.37 | 105,277 | 5.1M |
| 32_50x50_d5 | 4,900 | 19,297 | 6 | 15 | 0.01 | 17 | 30,571 |
| 50x6_s103_d25_astra | 544 | 2,132 | 838 | 1,930 | 0.84 | 2,599 | 500,501 |

  Over the repo tests (excluding the sample): rounds 1-838, median ~29, always solvable. Rounds scale with sparsity for narrow grids (50x6: 800+). A length bound (needed for short loops) makes the cuts weaker and rounds higher (not measured; cardinality constraint on 4900 edge vars is itself large: sequential counter ~ n*k = 4900*600 ~ 3M clauses (est.)). Time was dominated by Python overhead and conflicts, so a C++ CDCL will be much faster per round; a stronger cut (for a cycle missing at least one pearl: some edge must leave its cell set) would reduce the number of rounds (not measured).
- Root-level strength: unit propagation + failed-literal probing (all 9,800 literals, up to 5 passes) forces only 306/4900 (6.2%) edges on `29_50x50_d100`, 29 (0.6%) on d50, 3 on d20 and d5, 2203 (45%) on `50x50_s113_d100_astra` (script `quant/probe.py`): propagation alone cannot solve gen-style puzzles.

(c) Propagation speed (script `quant/propbench.py`): PySAT/minisat22 `propagate()` with 200 random assumptions taken from a valid solution on `29_50x50_d100` (4900 vars, 20,691 clauses): 452 literals propagated per call, 0.076 ms per call = 5.93M literals/s; on `31_50x50_d20`: 255 lits/call, 0.042 ms/call = 6.03M lits/s. These include Python call overhead and assumption handling, so a hand-written C++ CDCL at 5-20M props/s (the typical range given in the assignment; I did not benchmark the repo's own CDCL) is consistent.
- Probing speed: 9,800 literal probes took 0.8 s on 50x50 dense = 22 us/probe including Python (`probe.py`).

### Inferences
- Propagation-only pass: a complete assignment of all 4900 edge literals is 4900 propagations = 0.25 ms at 20M/s, 0.5 ms at 10M/s, 1 ms at 5M/s; even 1000 full passes cost only ~0.25-1 s. So propagation is cheap; the cost of SAT/CDCL is conflicts (273k-565k on the hard 50x50 feasibility runs = 9-20 s at python speeds, ~1-3 s plausibly in C++, est.), i.e. exact SAT for shortest loops on 50x50 is borderline even for validity and not feasible for optimisation inside 1 s.
- Conclusion on algorithm family: (1) full-grid DP: infeasible (1e22 states); (2) windowed DP / SAT-in-window LNS: feasible at ~8-10 rows x 50 columns per ~0.1-0.5 s, so ~2-8 windows per second budget; combine with random restarts or sparse-start constructive heuristics; (3) CDCL with lazy cuts as a *validity engine* for dense puzzles (6-630 rounds, seconds) is plausible in C++ within the 10 s slow path but not within 1 s on 50x50 dense; (4) for sparse puzzles use pearl-tour constructive + local search.

### Gaps
- No C++ implementation of the DP/CDCL was benchmarked (the repo's `dev/sat_part.h` and `cdcl_astra.txt` were not run by me because the files are being edited by another agent); the LNS window times are model-based estimates from the measured state counts, not measurements.
- Lazy-cut round counts are for feasibility without a length bound; rounds for an optimisation (length-bounded) search were not measured.


## Q5. Contest score per solution from the repo's measured runs, oracle comparison, where points are lost

### Takeaway
Under the statement's formula and with Lref = best-known per test (min over all repo runs plus my CP-SAT results, s=1), the strongest measured runs are `sol_v6_astra.cpp` (92.7 average on 48 tests; 7.06 points lost to length, 0.24 to slowness, 0 to invalid) and `sol_v4.cpp` (91.7 on 33 tests; 8.26 length, 0 slowness, 0 invalid). The earlier first-generation solutions lose mostly to invalid/timeouts (`sol_v1.cpp` and `sol_v1_astra.cpp`: 27-36 points) and the latest `sol_astra.cpp` runs (86-87 on 48 tests) lose 11-12.5 points to length and 0.8-1.0 to slowness (one run also 2.1 to an invalid output). Across all valid runs length excess is the dominant loss; slowness is <=1.2 points on average because nearly all runs finish by ~0.95 s.

### Cited Findings
Method (`quant/final.py`, `quant/p5.py`; data = copies of `p4/scores.jsonl` (5 runs) and `p4/scores_astra.jsonl` (16 runs) taken at the end; each line is a run with per-test `v` (length), `ok`, `s` (seconds)): per-test score = 100 * min(1, Lref/v)^2 * speed(s), 0 if not ok. Loss attribution per test (sequential): length loss = 100*(1-q); slowness loss = 100*q*(1-speed); invalid loss = 100 if not ok. They sum exactly to 100 minus the score. Runs are restricted to tests that exist as files in `p4/tests` (00..32) and `p4/tests_astra` (15 tests); `sol_astra.cpp` runs on 63 tests include 15 extra `caseN_transformK_astra` tests not present in the repo, which I excluded. "Oracle" = best-known L at T<=1 s: scores 100 on every test by construction (its speed factor is 1.0 for T<=1); measured against *proven optimum* it scores 100 on every test with proven optimum except `20x20_s107_d100_astra` (358 vs 352 -> 96.7), i.e. the oracle's expected gap to a true-optimum oracle is ~0.1 points per test on small/mid tests and unknown (est. 3-15% in L, i.e. 6-25 points) on the ~15 largest tests.

Per run (`quant/final.py`, snapshot; "ts" = last 5 digits of the run's epoch time, n = tests counted, loss columns = average points lost per test; Lref includes my CP-SAT incumbents):

```
idx sol ts n | avg | len-loss slow-loss invalid-loss | avg on core33(incl sample) | maxT | #invalid
 0 sol_v1.cpp         t=55531 n=33 |  62.22 |  10.30   0.21  27.27 |  62.22 | 8.49 | 9
 1 sol_v1.cpp         t=56456 n=33 |  60.06 |  12.54   0.13  27.27 |  60.06 | 8.58 | 9
 2 sol_v2.cpp         t=56456 n=33 |  86.67 |  13.33   0.00   0.00 |  86.67 | 0.91 | 0
 3 sol_v2.cpp         t=56876 n=33 |  82.63 |  17.37   0.00   0.00 |  82.63 | 0.94 | 0
 4 sol_v4.cpp         t=56876 n=33 |  91.74 |   8.26   0.00   0.00 |  91.74 | 0.94 | 0
 5 sol_v1_astra.cpp   t=56010 n=33 |  38.58 |  24.98   0.07  36.36 |  38.58 | 7.32 | 12
 6 sol_v2_astra.cpp   t=56120 n=33 |  76.70 |  20.05   0.22   3.03 |  76.70 | 7.17 | 1
 7 sol_v3_astra.cpp   t=56187 n=33 |  70.48 |  17.27   0.13  12.12 |  70.48 | 7.18 | 4
 8 sol_v4_astra.cpp   t=56289 n=33 |  67.58 |  29.33   0.06   3.03 |  67.58 | 7.21 | 1
 9 sol_v5_astra.cpp   t=56337 n=33 |  87.00 |  13.00   0.00   0.00 |  87.00 | 0.85 | 0
10 sol_v5_astra.cpp   t=56354 n=15 |  79.14 |  20.09   0.77   0.00 |    nan | 2.47 | 0
11 sol_v5_astra.cpp   t=56429 n=48 |  84.62 |  15.15   0.23   0.00 |    nan | 2.37 | 0
12 sol_v6_astra.cpp   t=56429 n=48 |  92.73 |   7.03   0.23   0.00 |    nan | 2.29 | 0
13 sol_v6_astra.cpp   t=56487 n=48 |  92.70 |   7.06   0.24   0.00 |    nan | 2.34 | 0
14 sol_v7_astra.cpp   t=56487 n=48 |  91.34 |   8.42   0.24   0.00 |    nan | 2.32 | 0
15 sol_astra.cpp      t=56584 n=15 |  91.96 |   4.71   3.33   0.00 |    nan | 4.54 | 0
18 sol_astra.cpp      t=56790 n=48 |  85.95 |  11.19   0.77   2.08 |    nan | 7.17 | 1
19 sol_v10_astra.cpp  t=56845 n= 3 |  65.88 |  25.18   8.94   0.00 |    nan | 4.87 | 0
20 sol_astra.cpp      t=56924 n=48 |  86.45 |  12.52   1.03   0.00 |    nan | 5.17 | 0
```
Invalid/timeout details: `sol_v1.cpp` run 0 is invalid (ok=False, 8.0-8.5 s, i.e. no valid loop) on 9 of 33 tests (`13_20x15_d100`, `21..23_30x40_d100/50/20`, `25..27_40x40`, `29,30_50x50`); `sol_v1_astra.cpp` on 12 of 33 (large and dense); `sol_v2_astra` 1; `sol_v3_astra` 4; `sol_v4_astra` 1 (`13_20x15_d100`, a 300-cell dense test); `sol_v5_astra` onwards 0 invalid on repo tests; `sol_astra.cpp` run idx 18 had 1 invalid (`50x50_s113_d25_astra`, 7.17 s), run idx 20 none.

Where the points go for the two strongest runs (`quant/worst.py`, top losses, loss points / L vs Lref / T):
- `sol_v6_astra.cpp`: `32_50x50_d5` 59.0 (178 vs 114, 0.77 s); `27_40x40_d20` 39.4 (262 vs 204); `30_50x50_d50` 37.6 (618 vs 488); `14_20x15_d50` 32.4 (90 vs 74, stopped at 0.30 s although optimum 74 exists - early termination); `31_50x50_d20` 20.8; `18_25x25_d50` 20.4; `50x50_s113_d25_astra` 19.2; `16_20x15_d5` 19.0 (40 vs 36, T=0.27 s); `50x50_s113_d3_astra` 16.8. Mean excess length vs best-known on non-sample tests: 4.9% (median 0%, max 56%, 11 of 47 tests >5% over).
- `sol_v4.cpp`: `25_40x40_d100` 52.5 (598 vs 412); `29_50x50_d100` 29.8 (666 vs 558); `27_40x40_d20` 27.8 (240 vs 204); `14_20x15_d50` 26.0 (86 vs 74); `22_30x40_d50` 21.6; `19_25x25_d20` 21.4; `21_30x40_d100` 18.2 (398 vs 360). Mean excess 5.5%, median 0%, 12 of 32 tests >5% over.
- Like-for-like on the 32 shared non-sample gen tests (`00_`..`32_`; runs 4 and 13): `sol_v4.cpp` 91.48 vs `sol_v6_astra.cpp` 91.47 (equal), but by class they are complementary: dense (d100) v4 84.6 vs v6 97.2; mid v4 92.1 vs v6 89.2; sparse v4 97.1 vs v6 90.3; by size S 99.3/99.3, M 90.8/90.0, L 84.1/84.7. The per-test max of the two scores is 96.83 (dense 97.2, mid 95.1, sparse 100.0; M 95.3, L 95.4), i.e. +5.4 points over either, which is what a combined/portfolio solver (run both, print the shorter, or dispatch by density) could reach within the same Lref.

### Inferences
- Speed is essentially free in the current implementations: all recent runs finish at 0.77-0.94 s on the large tests, so slowness is ~0-0.25 points average; even `sol_astra.cpp` which uses up to 5-7 s only loses ~1 point average to speed (but it loses more through length where it stops short or invalid on one test). Cutting runtime further than ~0.9 s gains nothing (speed=1.0 up to 1 s).
- The remaining lever is length on the ~15-20 largest/hardest tests (>=25x25): ~7 average points per test over 48 tests (v6) concentrated in ~10 tests each losing 17-59 points. Early termination (`14_20x15_d50` stopped at 0.30 s with 21.6% excess; `16_20x15_d5` at 0.27 s with 11%) is a pure waste: those tests could use the free second.
- Invalid/timeouts were the dominant loss in the first generation (27-36 points per test average), now eliminated on repo tests, but slow paths still exist in other agents' extra tests (e.g. `case4_transform1_astra` took 5.79 s in `sol_astra.cpp`); keep the validity-first fallback.
- Estimate for the hidden set: ~89-91 points per test for `sol_v6_astra.cpp` (Q2: 90.8 / 89.0) if hidden tests resemble repo tests and Lref ~ best-known; if the organisers' Lref comes from a stronger exact solver on mid-size tests, expect lower; if their s=0.9 "best found" on large tests is weaker than ours, the cap (quality 1.0 once L <= 0.9*Lref) would raise scores on those.

### Gaps
- After my snapshot (first 5 lines of `scores.jsonl`, first 16 of `scores_astra.jsonl`) two more runs were appended to `p4/scores.jsonl` (`sol_v4.cpp` on 41 tests incl. new tests 33-40: 93.5 by the other agent's own Lref basis, `sol_v5.cpp` on 41 tests: 92.1; these use tests that were not in my snapshot, so they are not in my tables and the numbers quoted here are from my own best-known recomputation only roughly comparable).
- The files were being appended while I read them; later runs (`sol_v8..v10`, `sol_astra.cpp` variants) may have changed the picture. Per-test T values come from a heavily loaded machine (load ~17), so absolute runtimes on the judge may differ.
- Best-known per test is not final for large tests (Q3); older runs are penalised more than they would be against the organisers' Lref.

