# Independent adversarial cases

- `00_four_color_pairs`: known witness `AB / CD`. Every required pair is an edge or diagonal, and no equal letters are adjacent. The v1 shelf layout locks conflicting placements and fails despite feasibility.
- `01_only_one_letter`: a 5×5 all-A witness; checks allowed-alphabet reduction.
- `02_forbidden_three_wraps`: place ABC, DEF, GHI on separated rows of an 8×8 all-Z background. Long forbidden words test periodic continuation beyond N.
- `03_reverse_substrings`: place ABCDEFG in a row of an 8×8 all-H background, leaving a final H separator. Checks duplicate/reversal removal and long cyclic forbidden words.
- `04_empty_four_colors`: known witness `AB / CD`; every 1×1 grid is forbidden, so empty required lists still need construction.

All inputs are deliberately feasible. Judge constraints are unknown. Score these only through `../tools/run_tests.py --tl 2 --tests 'edge_tests/*.in' SOLVER.py`.
