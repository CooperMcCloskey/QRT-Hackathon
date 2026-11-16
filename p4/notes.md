# P4 Masyu — notes
1. Input (stdin): R, then R rows of length C over {. W B}; 6<=R,C<=50, >=2 pearls, solvable.
2. Output: R rows of length C over {. - | L J F 7}; L=N+E, J=N+W, F=S+E, 7=S+W. (HackerRank C template: write to $OUTPUT_PATH; we write there if set, else stdout.)
3. Validity: dims/chars; every connection reciprocated and inside grid; one single closed loop (all non-'.' cells one cycle);
   every pearl on loop; W: straight + at least one loop-neighbour turns; B: turns + both loop-neighbours straight.
4. Objective: minimise L = #non-'.' cells. score = 100*min(1,s*Lref/L)^2 * (0.5+0.5*exp(-max(0,T-1)/5)).
5. TL 10 s/test (timeout=0). Runtime past 1 s costs score -> target ~0.85 s, slow path only if no valid loop yet.
6. Main() of template truncated in statement; assume HackerRank style (grid_count line, then rows; output to OUTPUT_PATH).
