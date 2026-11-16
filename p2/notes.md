# Step 0 — Word Search Generation
Source: statement.md transcribed from p2/image.png and the user's scoring screenshot.
Input: generate_grid(required_words: list[str], forbidden_words: list[str]); word counts/length bounds and full judge wrapper are not supplied.
Output: list[str] of N rows, each of length N; rows top-to-bottom, letters left-to-right; no coordinates or index base are output.
Cells must be uppercase ASCII A–Z, and N must be at least the longest required word's length.
Every required word must occur; no forbidden word may occur, including across either boundary.
Each occurrence follows one fixed (dr, dc) in {-1, 0, 1}² excluding (0, 0), with row/column coordinates reduced modulo N at each step.
Objective: minimise N; scoring maximises 100 * min(1, S / (1.7 * N²)), where S is the total length of the supplied required words; invalid grids score 0.
Sample: required [ACE, FIG, OAK], forbidden [] (shown as '-'), output [ACE, FIG, OAK]; all three words are horizontal in a 3×3 square.
Scoring ambiguity: prose says two words per cell, but the explicit formula and numerical examples support a density threshold of 1.7; use the formula.
Time/memory limits and maximum input/grid sizes are unknown; use a provisional local 2-second limit with a 1.5-second optimisation budget, not a claimed judge limit.
User authorises proceeding without more information: assume nonempty uppercase words and feasible inputs; preserve duplicate lengths in S; use N=1 for an empty required list; allow repeated wrapping for long forbidden words.
Submission interface is the supplied function; JSON stdin and newline-separated rows are local testing conventions only; impossible inputs raise ValueError because no valid return convention is supplied.
