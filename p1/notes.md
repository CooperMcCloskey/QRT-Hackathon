# p1 notes: Square packing++

1. Input: list of positive ints (square sides). Exact stdin layout, N limit and
   side limit NOT visible in screenshots -> GUESSED. Parser is tolerant: takes
   every integer on stdin; drops a leading count if line 1 is a lone integer
   equal to the number of remaining integers. Checked on `[1,2,3,4]`,
   `1 2 3 4`, and count + one-per-line.
2. Output: one square per line, `X Y` = top-left corner, in input order.
   X grows downward (row), Y grows right (column). "Wrong number of lines"
   scores 0, so exactly N lines. Separator (`X Y` vs `(X,Y)`) is GUESSED.
3. Validity: integer coords, no overlap (touching is fine), N lines.
4. Objective: minimise bounding area A. Score per test = (L/A)^20 (max 1),
   L = min over W>=m of W*max(m, ceil(S/W)), m = max side, S = sum of squares.
   Very steep: 1% extra area costs ~18% of the score; >70% needs A <= 1.018 L.
5. Time/memory limits: NOT visible. sol_v4 assumes 2 s (stops at 1.5 s),
   sol_v5 assumes 1 s (stops at 0.75 s). Score barely depends on run time.
6. Ambiguous: input layout, output separator, limits, languages allowed.
   Samples are consecutive squares 1..N, so hidden tests may be too.
   Sample 4 (5x7=35 vs L=30) shows L is not always reachable.

## Versions

- sol_v1.py: bottom-left skyline over widths (baseline).
- sol_v2.cpp: lowest-gap decoder (first square of a priority order that fits),
  SA on the order for fixed boxes W x H, H shrinking, several widths.
- sol_v3.cpp: experiment file (env-tunable); same code as v4.
- sol_v4.cpp: v2 + exact-width-fit preference + skyline along the short side
  only (W <= 1.05 sqrt(S)) + rollout beam after each SA slice. 2 s assumed.
- sol_v5.cpp: v4 with 1 s assumed, clock read more often on huge inputs,
  bounded final self-check. Stress: N=200000 finishes in about 1.0 s.

## What was learnt

- SA saturates at about 1.2-1.5% waste: 0.5 s and 10 s give nearly the same
  score, so tuning and extra time do not help; the decoder is the limit.
- Wide, short strips are much worse than narrow, tall ones.
- Bucketing the arms over the width range hurt.
