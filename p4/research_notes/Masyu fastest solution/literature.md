# Fast valid + short single-loop Masyu solutions (grids up to 50x50, ~1 s): literature on algorithm families

Method note. About 15 web tool calls. The search backend returned model-written summaries, so only facts that I could tie to a fetched page or a search-result snippet are listed under "Cited Findings". Several PDFs (Radboud thesis, TU Berlin theses, Minato et al. in Algorithms) could not be text-extracted or returned 403/404. Their numbers are NOT here and are listed as gaps. Statements marked "Inference" come from my own reasoning or general knowledge, not from a source. Implementability estimates for single-file C++17 are all inference.

---

## 1. SAT/CDCL encodings of Masyu / Slitherlink / Hamiltonian-cycle puzzles: how is the single-cycle constraint handled, and what is fastest?

### Takeaway
Two routes exist for the single-loop constraint: (a) encode it in CNF with a distance/ordering (level) encoding, which is what the best published Masyu SAT result uses (Picat+Kissat, 30x30 to 40x40 in 1.6 to 6.2 s total), or (b) leave it out and block every sub-cycle lazily (CEGAR), which usually works but has an exponential worst case. The Hakerh400 write-up reports the distance encoding scales better on large grids than the cellular-automaton encoding. No source gives numbers for a hand-written CDCL on a 50x50 Masyu.

### Cited Findings
- Picat HCP/`hcp_grid` encoding: each vertex has exactly one in-edge and one out-edge. Sub-cycle elimination uses a distance (ordering/level) encoding. One start vertex is chosen, each vertex gets a distinct distance, and if edge v_i -> v_j exists then dist(v_j) = succ(dist(v_i)) unless v_j is the start. The successor is encoded with a binary adder, which the paper says did best in prior work. The paper gives no clause-count or complexity analysis. — [Zhou, Picat reachability constraints, via ar5iv](https://ar5iv.labs.arxiv.org/html/2109.08293); [arXiv PDF](https://arxiv.org/pdf/2109.08293)
- Masyu in that model: a Boolean grid has circle cells fixed to 1 and `hcp_grid` forces the 1-cells into a loop. A white circle is modelled by 8 path shapes (straight through, plus a turn in an adjacent cell), and a black circle by 8 turn-plus-straight-arms shapes. `constrain_paths` requires that at least one valid shape appears in the loop. Shapes that fall outside the grid are discarded. Edge variables sit in a map for O(1) lookup. — [ar5iv 2109.08293](https://ar5iv.labs.arxiv.org/html/2109.08293)
- Reported Masyu times (translation + solving, seconds), from that paper (as returned by the fetch tool): Masyu3 30x30: Picat+Kissat 1.0.3 = 1.62, Picat+Maple = 6.94, Clingo 5.5.0 = 22.37. Masyu4 35x35: 3.95 / 9.03 / 48.84. Masyu5 40x40: 6.21 / 11.72 / 96.78. Kissat is roughly 12 to 16x faster than Clingo here. Hardware is not stated in what I read. These instances come from the LP/CP programming contest 2020 and may be pearl-sparse or pearl-dense. I did not confirm which. — [ar5iv 2109.08293](https://ar5iv.labs.arxiv.org/html/2109.08293)
- For strongly-connected-component problems the same paper uses a tree encoding (root, parent per vertex, distance = parent distance + 1). — [ar5iv 2109.08293](https://ar5iv.labs.arxiv.org/html/2109.08293)
- Slitherlink SAT write-up describes two direct encodings. Method 1 is a cellular automaton: it starts from one black line, spreads to adjacent black lines each generation, and the last generation must cover exactly the black lines. It is fast in practice, but its variable and clause counts grow rapidly with grid size, which makes large grids intractable. Method 2 is path distance: each line gets an n-bit number, exactly one black line has value 0, and every other black line needs an adjacent black line with value one lower. It is slower on small grids but has far fewer auxiliary variables and is faster on large grids. — [Hakerh400, esolangs wiki](https://esolangs.org/wiki/User:Hakerh400/How_to_solve_Slitherlink_using_SAT_solver)
- The same page describes iterative cycle blocking (Douglas Brown's solver). Call SAT. If the model has several loops, add a clause forbidding each loop, then re-solve. It "usually works well" but deliberately crafted puzzles can force an exponential number of solver calls. It also says Simon Tatham's solver is much slower than modern SAT solvers. — [esolangs wiki](https://esolangs.org/wiki/User:Hakerh400/How_to_solve_Slitherlink_using_SAT_solver)
- Optilog documentation does the lazy version with CaDiCaL. The CNF has per-cell degree constraints, with no connectivity. After each SAT call it decodes the cycles and adds, for each blocked cycle, a clause negating all its edges. It stops when exactly one constrained cycle remains. — [Optilog Slitherlink solving](https://ulog.udl.cat/static/doc/optilog/html/optilog/use-cases/slitherlink/solving-slitherlink.html)
- Scale of the lazy approach: 100 random 101x101 Slitherlink instances averaged about 149,937 Boolean variables, 308,721 clauses in the first formula, 395,253 in the last, and 120 iterations on average. The tuning run used a 300 s limit per run and a 6 GB limit. No solve times are given. — [Optilog tuning](https://ulog.udl.cat/static/doc/optilog/html/optilog/use-cases/slitherlink/tuning-slitherlink.html)
- A 2026 TU Berlin bachelor thesis compared three single-loop methods in CP-SAT for Slitherlink. According to the search summary (I could not read the PDF), the lazy approach was the fastest and most consistent, and check-after-solving scaled worse. — [search result for TU Berlin thesis](https://doc.neuro.tu-berlin.de/bachelor/2026-BA-OmarAlkhatib.pdf) (unverified details)
- ASP study: connectedness and single-loop rules need transitive closure, so naive encodings are non-tight and generate very many loop formulas. — [Solving challenging grid puzzles with ASP](https://www.academia.edu/2791103/Solving_challenging_grid_puzzles_with_answer_set_programming) (snippet only)
- A Radboud bachelor thesis encodes a graph-connectivity constraint directly in SMT (Z3) and applies it to Masyu and other loop puzzles. — [van der Knijff thesis](https://www.cs.ru.nl/bachelors-theses/2021/Gerhard_van_der_Knijff___1006946___Solving_and_generating_puzzles_with_a_connectivity_constraint.pdf) (could not read the content)
- Knuth's DLX Masyu program does not enforce the single loop. It enumerates exact-cover solutions and filters out disconnected ones afterwards. The search snippet said the filter removes most of them early. — [Knuth masyu-dlx.w](https://www-cs-staff.stanford.edu/~knuth/programs/masyu-dlx.w) (page fetch failed with a certificate error, so only the snippet is available)

### Inferences
- For a hand-written CDCL, a distance encoding on a 50x50 grid has about 2500 cells x ceil(log2 2500) = 12 bits, about 30k auxiliary variables, plus adder clauses per edge. That is large but tractable. The binary-adder encoding is the only one with published support (Picat) and is the natural choice if a full CDCL is written.
- A pure "degree + pearl shape" CNF plus lazy sub-cycle blocking is the simplest to implement. The weakness is that Masyu has few clues in sparse instances, so the SAT solver tends to produce many small loops that all need blocking. The Optilog average of 120 iterations on 101x101 Slitherlink suggests it is manageable when clues are dense. Masyu clues are sparser, so expect more iterations. This extrapolation is not tested by any source.
- In this task the objective is a short valid loop, not a unique solution. Pearls are only a subset of the constraints, and the free cells are unconstrained. So the CNF solution space is huge, a first solution is found fast, and the cost is dominated by connectivity and by optimisation, not by satisfiability.
- A hand-written CDCL with 2-watched literals, VSIDS, restarts and phase saving is about 300 to 500 lines of C++17. The pearl and degree clauses add about 100 lines. Lazy blocking fits easily in the 1000-line budget. A level encoding with binary adders adds roughly 150 lines.

### Gaps
- No source gives Kissat/CaDiCaL/MiniSat/Glucose numbers on any Masyu or Slitherlink instance up to 50x50 other than the Picat 30x30 to 40x40 table (and it is for a uniquely solvable contest puzzle, not a shortest-loop objective).
- I could not read the Radboud thesis or the TU Berlin theses (binary PDFs), so their encodings and timings are missing.
- The hardware used for the Picat numbers is not known to me.

---

## 2. MaxSAT / ILP (CP-SAT, Gurobi/CBC) / CP approaches to the shortest loop, and how they scale

### Takeaway
I found no published benchmark of MaxSAT/ILP for shortest Masyu loops. The only concrete hooks are CP-SAT's `AddCircuit` with self-loops for optional nodes (handles single-cycle natively) and the TU Berlin result that lazy constraints beat other single-loop encodings in CP-SAT for Slitherlink. None of this can be used directly in a no-library C++ solution.

### Cited Findings
- In CP-SAT, `AddCircuit` requires the non-self-loop arcs to form a single circuit. Optional nodes are modelled with a self-loop: if node k is active only when literal `lit` holds, add arc (k, k, lit.Not()). — [OR-tools CP-SAT reference](https://developers.google.com/optimization/reference/python/sat/python/cp_model); [activimetrics circuit worked example](https://activimetrics.com/blog/ortools/cp_sat/two_circuit_constraints/)
- `AddMultipleCircuit` also supports optional vertices through self-loops. — [CP-SAT primer](https://d-krupke.github.io/cpsat-primer/advanced_modelling.html)
- Slitherlink in CP-SAT: lazy constraints were the fastest of three methods tested. — [TU Berlin 2026 thesis](https://doc.neuro.tu-berlin.de/bachelor/2026-BA-OmarAlkhatib.pdf) (summary only)

### Inferences
- Shortest loop = minimise the number of used edges subject to degree-2 on pearls, pearl shapes, and single-cycle. A MaxSAT/ILP model for 2500 cells has about 5000 edge variables and 2500 node variables, which is within reach of CP-SAT but beyond any hand-written solver in ~1 s. A hand-written implementation would use cardinality constraints in a stochastic or LNS setting (see section 4), not a complete optimiser.
- Complete optimal methods are not realistic inside 1 s for 50x50 and are not implementable in 1000 lines.

### Gaps
- No numbers for MaxSAT/ILP/CP on Masyu optimisation were found (searches for them returned no relevant papers).
- I did not find CP-SAT or Gurobi solve times for large grid loop puzzles.

---

## 3. Frontier / plug / broken-profile DP, Simpath/ZDD, state-count growth, and DP windows in LNS

### Takeaway
Frontier DP is exact and its state space is bounded by Motzkin-like counts, O(3^w) for frontier width w. That is too large for a full 50-wide grid but fine for narrow windows (width up to about 10 to 12). No source describes a Masyu-specific frontier DP or a frontier-DP-based LNS.

### Cited Findings
- Knuth's Simpath modification enumerates all single cycles of a graph as a ZDD. Being a single cycle is a necessary condition for a Slitherlink solution, and Minato et al. build Slitherlink and Numberlink solvers on this. — [Minato et al., Algorithms 2012](https://mdpi.com/1999-4893/5/2/176) (page returned 403 on fetch; abstract and snippets from search only); [portal.mardi4nfdi](https://portal.mardi4nfdi.de/wiki/Item:Q1736507)
- One paper bounds the states per layer by the number of Motzkin paths, O(3^L) for a frontier of width L. Another says the valid frontier states are closely related to Motzkin numbers. — [search snippets from Hokkaido TCS reports](https://www-alg.ist.hokudai.ac.jp/~thomas/TCSTR/tcstr_13_64/tcstr_13_64.pdf) (not fetched)
- Hamiltonian cycles in m x n grids: there is a bijection between automaton states and Motzkin-language words of length m+1. Generating functions for fixed width were computed by the Stoyan-Strehl method up to width 10 in Maple. — [arXiv 2603.24315](https://arxiv.org/abs/2603.24315)
- Search found no source for a broken-profile or frontier-DP Masyu solver. A Carleton Slitherlink project uses simple rules and guessing instead of frontier DP. — [Carleton comps](https://cs.carleton.edu/cs_comps/1516/slither/final-results/abstract.html)

### Inferences
- Motzkin growth: number of plug-state patterns with pairing for a frontier of w+1 plugs is the Motzkin number M(w+1), roughly 3^w times a polynomial factor (M(11) = 835, M(13) = 3,629, M(16) is on the order of 10^5). These values are my own recollection of the standard Motzkin sequence, not from the sources above. A window of 8 to 10 rows by 50 columns scanned in column-major order (frontier = window height) therefore has at most a few thousand to tens of thousands of states per cell, and 50 columns x 10 rows = 500 cell steps. That is about 10^6 to 10^7 state-transitions per DP, i.e. tens of ms. The same window scanned along the 50-wide dimension would be infeasible (M(51) is astronomical).
- For shortest-loop LNS: fix the loop outside a window, with boundary crossings fixed, and run a min-cost DP over the window (cost = number of edges). This must also keep the global single-cycle property, which the plug DP tracks via the pairing of plugs. Connectivity through the outside of the window must be tracked by pre-pairing the boundary crossings according to how the outside connects them. This is implementable in about 300 to 500 lines. I have no source that has done it for Masyu.
- Pearl constraints are local, so they are easy to add to the DP transitions (each cell looks at its own turn/straight state and a few neighbours' plug states, with a one-cell look-ahead for black pearl arms).

### Gaps
- No measured state counts or times for frontier DP on Masyu or Slitherlink grids were retrieved (Minato et al. experiment tables not readable).
- No literature on a DP window used inside LNS for loop puzzles.

---

## 4. Local search for the shortest loop: face-flip / SA, 2-opt, LNS with SAT or DP windows

### Takeaway
I found no direct source on shortest-Masyu-loop local search. The closest primary-source ideas are: (a) the boundary of a simply connected cell region is automatically one closed loop (single-cycle guaranteed by construction, no connectivity check on the loop itself), and (b) SAT-oracle LNS ("Local Search with a SAT Oracle") and MaxSAT improvement of heuristic solutions (Shadoks, CG:SHOP 2026, about 10% improvement).

### Cited Findings
- Region-based approach: a puzzle can be built from a region mask, and the boundary of a simply connected region forms one closed loop. — [search result summary of Carleton and other generator write-ups](https://cs.carleton.edu/cs_comps/1516/slither/final-results/abstract.html) (second-hand)
- Carleton Slitherlink generator: create a random loop first; a greedy algorithm can produce valid loops if two rules are followed. — [Carleton comps](https://cs.carleton.edu/cs_comps/1516/slither/final-results/abstract.html)
- The sen-ltd Masyu generator grows a solved loop by bumping straight segments outward, then reads legal pearl positions from the finished loop. — [sen-ltd/masyu](https://github.com/sen-ltd/masyu)
- Shadoks (CG:SHOP 2026): heuristics that need no external solver are important for initial solutions on large instances. SAT and MaxSAT then improve them by roughly 10%. The problem is triangulation reconfiguration, not loops. — [arXiv 2603.21293](https://arxiv.org/pdf/2603.21293)
- Cohen, Nadel, Ryvchin: local search where a SAT solver searches large complex neighbourhoods. — [Local Search with a SAT Oracle](https://link.springer.com/content/pdf/10.1007/978-3-030-72013-1_5) (snippet)

### Inferences
- Since a Masyu loop must pass through all pearls, a region-flip representation is incompatible with some pearls (a pearl cell must be on the loop). In the cell-region view the loop is the boundary between inside and outside cells, so "pearl on loop" means the vertices (pearl points) are on the boundary. The standard formulation is: loop = boundary of a simply connected, hole-free set of faces on the dual lattice (vertices at the Masyu cell centres). Pearls then become constraints on the four faces around a pearl vertex (a vertex is on the boundary iff the four surrounding faces are not all equal; degree-2 means exactly a 1-3 or 2-2-diagonal-excluded pattern). Moves flip one face at a time and are legal iff connectivity of inside and outside is preserved (a local 8-neighbourhood test, as in the simple-point test from digital topology, general knowledge, not from the sources). I believe this is a good SA move set, but I found no source confirming results.
- Cost = number of boundary edges = loop length. Shrinking the loop requires many flips that each change length by at most +/-2 and are accepted/rejected in O(1). SA at 10^7 to 10^8 flips/s in C++ is plausible, but there is no evidence in the sources.
- Because the loop passes through every pearl, a loop with L about the minimum is close to a rectilinear-Steiner-like cycle through the pearls. LNS windows (DP over strips, section 3) are best at removing local slack once SA has converged.

### Gaps
- No source evaluates SA/2-opt/LNS for shortest loops, nor gives achievable ratios vs optimal.
- No hard comparison of face-flip SA vs DP-LNS.

---

## 5. Constraint-propagation rules specific to Masyu and how much they prune

### Takeaway
Standard rules are local patterns on edge states plus one non-local guard (a closed loop must contain all pearls and all ON edges). These are adequate for puzzle solving but no source quantifies how much they prune. For a shortest-loop task with no uniqueness requirement, only the forced-edge rules around pearls matter.

### Cited Findings
- sen-ltd/masyu edge model: edge states UNKNOWN/ON/OFF. Every cell has degree 0 or 2, pearl cells exactly 2. A white pearl can only use its two collinear edges; a black pearl only one horizontal plus one vertical edge. When all surviving patterns agree on an edge it is forced. If a black pearl's arm is ON, the next cell beyond must continue straight (the outer edge is forced ON, or the arm is set OFF if impossible). Closure guard: when ON edges form a finished loop, that loop must already contain every pearl and every ON edge, otherwise the branch is dead. Search branches on an edge that extends an existing fragment, ON before OFF, with propagation after each. — [dev.to / sen-ltd/masyu README](https://github.com/sen-ltd/masyu) (language TypeScript; no performance numbers given)
- CacheSleuth solver: each cell degree 0 or 2, a connection tracker, and a link that would close a loop is accepted only if that loop already satisfies every pearl. — [cachesleuth](https://www.cachesleuth.com/tools/masyu/) (from search snippet; the page returned 404 when fetched)
- Typical human deductions: black pearl with only one possible turn direction forces the two turning edges and their straight extensions; a white pearl with only one possible straight direction forces the opposite edges. Loop-degree logic is also used. — [search summary of Masyu solver pages](https://www.puzzles.wiki/wiki/Masyu)
- Rule of thumb: adjacent black pearls, black pearl next to the border, and similar patterns give early forced edges; closing a small loop early is a dead end unless it contains every pearl. — [puzzles.wiki Masyu](https://www.puzzles.wiki/wiki/Masyu); [KrazyDad tutorial](https://derp.krazydad.com/masyu/tutorial/)
- Knuth's DLX approach shows the loop filter eliminates most disconnected solutions early. — [Knuth masyu-dlx.w](https://www-cs-staff.stanford.edu/~knuth/programs/masyu-dlx.w) (snippet)

### Inferences
- Pearl rules (black: turn plus 2 straight arms, so no black pearl within 1 of the border in the turn direction; white: straight plus a turn at one end) are pure local clauses: with 8 shapes each they are O(1) per pearl. They are the cheapest source of forced edges and are easy to encode in a CDCL or in a custom propagation search. A union-find "closure guard" is about 40 lines.
- Border rule (derived from the shapes, not cited): a black pearl cannot use an arm whose neighbour cell is off-grid or whose cell beyond is off-grid (arm needs two cells in line), so a black pearl one cell from the border has a forced turn direction.
- None of this gives a pruning ratio. I have no cited numbers.

### Gaps
- No quantitative pruning data for Masyu rules. No source on rules that pay off for sparse pearls on 50x50.

---

## 6. Constructive approaches: pearls as required nodes (Steiner / TSP-like), merging cycles by local patches

### Takeaway
Masyu is NP-complete (reduction from Hamiltonian circuit in cubic planar graphs), so there is no polynomial exact method, but a valid short loop (not unique, not necessarily optimal) only needs a loop through all pearls obeying local shapes. A 2-factor-then-merge pipeline is documented for grid Hamiltonian cycles. No source applies it to Masyu with pearl constraints.

### Cited Findings
- Masyu/Pearl is NP-complete: NP-hardness by reduction from the Hamiltonian circuit problem on cubic planar graphs, using white-pearl wall gadgets, one-wide passageways, vertex rooms with a single isolated white pearl. Verification is O(n^2) for the rules, O(n^4) for self-intersection by naive pair comparison. A planar graph with n vertices has an O(n^2) grid drawing, so the construction is polynomial. — [Friedman, Pearl Puzzles are NP-complete](https://mountainvistasoft.com/docs/pearl-puzzles-are-np-complete.htm); [original PDF](https://erich-friedman.github.io/papers/pearl.pdf)
- Framework paper for loop and path puzzle hardness (Masyu included). — [arXiv 2202.02046](https://arxiv.org/pdf/2202.02046); [arXiv 2004.12849](https://arxiv.org/pdf/2004.12849)
- Cycle merging in grids: start from a 2-factor (disjoint cycles covering all vertices); if neighbouring cycles share a unit square ("separant", two parallel edges on a 2x2 square from different cycles) they can be spliced into one cycle by swapping those two edges for the other two. Repeating this gives a Hamiltonian cycle. — [arXiv 2501.11832](https://arxiv.org/pdf/2501.11832); [Grid Hamiltonicity slides](https://www.csd.uoc.gr/~hy583/2010_presentations/Grid_Hamiltonicity.pdf)
- Spanning-tree construction: the outline of a spanning tree drawn at double resolution traces a Hamiltonian cycle; it does not produce all cycles. An n x m grid with n and m both odd has no Hamiltonian cycle (bipartite parity). — [Medium, generating Hamiltonian cycles in grids](https://medium.com/@pascal.sommer.ch/generating-hamiltonian-cycles-in-rectangular-grid-graphs-316c94ecefe0); [arXiv 2501.11832](https://arxiv.org/pdf/2501.11832)
- Solid grid graphs have a Hamiltonian cycle iff a valid "checkering" admits a spanning tree. — [arXiv 1805.03192](https://arxiv.org/pdf/1805.03192)

### Inferences
- A Masyu loop need not visit all cells, so the full-Hamiltonian results are only an analogy. The relevant pipeline (my own proposal, no source): (1) build a cheap 2-regular structure that satisfies the pearl shapes, e.g. local pearl patterns plus small cycles (like 2x2 squares or short rectangles) around the free cells touching pearls; (2) merge cycles with 2x2 face splices, which preserve degrees and pearl constraints if the spliced edges are not adjacent to a pearl-turn requirement; (3) shorten with SA/LNS. The merge splice changes length by 0 (swap of two edges for two edges on a unit square), which is cheap but can break pearl shapes, so it needs the local checks from section 5.
- Spanning-tree boundary loops (cycle = outline of a thickened tree on the dual grid) are a more natural "valid loop by construction" family: with tree edges forced along pearl-required straights and turns, any tree whose outline passes through pearl points gives one cycle. Again unsupported by sources.
- Treating pearls as TSP/Steiner nodes: shortest rectilinear cycle through all pearl points under shape constraints is a TSP-like problem with local turn constraints. Quadratic-time heuristics (nearest neighbour + 2-opt/Or-opt on the pearl order, then routing) are standard for TSP but the local pearl constraints (black pearl turn, straight arms; white straight) make it a constrained TSP. No source tests this.

### Gaps
- No source reports success rates or lengths of the "2-factor merge" or "Steiner cycle" pipelines on Masyu.
- No source on how often pearl shapes survive splicing.

---

## 7. Practical advice from competitions / solver writeups

### Takeaway
I found no AtCoder, Codeforces, ICFP or Hash Code writeup about Masyu or any shortest-loop variant, so practical advice only comes from solver authors and the LP/CP programming contests (Picat). The consistent advice there is: use SAT with a native single-loop encoding or lazy sub-cycle blocking, and keep propagation of pearl patterns local and cheap.

### Cited Findings
- The Picat paper selects Masyu from the 2020 LP/CP Programming Contest and credits "cutting-edge SAT solvers empowered with effective encodings" for solving efficiency. Contest repositories: lpcp-contest-2019 and 2020 on GitHub. — [arXiv 2109.08293](https://arxiv.org/pdf/2109.08293)
- Simon Tatham's "pearl" puzzle is the open-source C reference implementation (generator and solver). The documentation says the "Allow unsoluble" option skips the uniqueness check and allows much larger grids; the generator then shows every possible clue and a solution is still guaranteed. It gives no solver strategy or grid-size limit. — [sgt-puzzles Pearl docs](https://www.chiark.greenend.org.uk/~sgtatham/puzzles/doc/pearl.html)
- Cycle-blocking CEGAR is the common practitioner approach, with worst-case exponential iteration counts on crafted inputs. — [Hakerh400, esolangs](https://esolangs.org/wiki/User:Hakerh400/How_to_solve_Slitherlink_using_SAT_solver)
- One search returned only generic Masyu background for contests (no contest editorials). — [Wikipedia Masyu](https://en.wikipedia.org/wiki/Masyu)

### Inferences
- Given the score formula (quadratic in length, mild time penalty), the practical recipe suggested by the combined evidence is: fast valid construction first (propagation + backtracking/CDCL with closure guard, or constructive merging), then spend the remaining time on local improvement. The literature gives no evidence on which gets closest to Lref.
- Puzzle generators (sen-ltd, Tatham) produce loops by "bumping" segments outward from a small loop. The reverse (retracting bumps while keeping pearls on the loop) is the natural analogue of face-flip shortening.

### Gaps
- No contest writeups found. I searched AtCoder/Codeforces/ICFP/Hash Code terms only once, with no hits; a differently worded search (e.g. specific contest names, "loop puzzle heuristic contest", Kaggle/Advent of Code) was not done.
- No benchmark of a custom (non-SAT) Masyu solver at 50x50 (sen-ltd/masyu README gives no numbers).

---

## Source list (URLs consulted)
- https://ar5iv.labs.arxiv.org/html/2109.08293 (Picat reachability, Masyu table; read via fetch)
- https://arxiv.org/pdf/2109.08293
- https://esolangs.org/wiki/User:Hakerh400/How_to_solve_Slitherlink_using_SAT_solver (read)
- https://ulog.udl.cat/static/doc/optilog/html/optilog/use-cases/slitherlink/solving-slitherlink.html (read)
- https://ulog.udl.cat/static/doc/optilog/html/optilog/use-cases/slitherlink/tuning-slitherlink.html (read)
- https://github.com/sen-ltd/masyu (README read)
- https://mountainvistasoft.com/docs/pearl-puzzles-are-np-complete.htm and https://erich-friedman.github.io/papers/pearl.pdf (Friedman)
- https://arxiv.org/abs/2603.24315 (abstract read)
- https://www.chiark.greenend.org.uk/~sgtatham/puzzles/doc/pearl.html (read)
- https://mdpi.com/1999-4893/5/2/176 (403, snippets only); https://www-cs-staff.stanford.edu/~knuth/programs/masyu-dlx.w (cert error, snippet only)
- https://doc.neuro.tu-berlin.de/bachelor/2026-BA-OmarAlkhatib.pdf and https://www.cs.ru.nl/bachelors-theses/2021/Gerhard_van_der_Knijff___1006946___Solving_and_generating_puzzles_with_a_connectivity_constraint.pdf (binary, unreadable via tools)
- https://arxiv.org/pdf/2501.11832, https://arxiv.org/pdf/1805.03192, https://www.csd.uoc.gr/~hy583/2010_presentations/Grid_Hamiltonicity.pdf, https://medium.com/@pascal.sommer.ch/generating-hamiltonian-cycles-in-rectangular-grid-graphs-316c94ecefe0 (snippets)
- https://arxiv.org/pdf/2603.21293, https://link.springer.com/content/pdf/10.1007/978-3-030-72013-1_5 (snippets)
- https://developers.google.com/optimization/reference/python/sat/python/cp_model, https://activimetrics.com/blog/ortools/cp_sat/two_circuit_constraints/, https://d-krupke.github.io/cpsat-primer/advanced_modelling.html (snippets)
- https://www.puzzles.wiki/wiki/Masyu, https://derp.krazydad.com/masyu/tutorial/, https://cs.carleton.edu/cs_comps/1516/slither/final-results/abstract.html (snippets)
