# Problem statement

Paste the full statement, code template and samples here, word for word.

Masyu (ましゅ) is a Nikoli logic puzzle. You are given a rectangular grid of cells, some of which contain white or black “pearls” (circles). Your task is to draw a single closed loop that visits every pearl, passes through cell centres, moving only horizontally or vertically (every turn is 90°), never crosses itself and never visits a cell twice, and obeys the pearl rules: White pearl — the loop passes straight through the cell, AND turns in at least one of the two adjacent cells along the loop. Black pearl — the loop turns in the cell, AND passes straight through the cells immediately before and after. Cells without a pearl may be visited or skipped freely. Loop cells may sit next to each other without being connected. Puzzles may have more than one valid loop. Any valid loop is accepted, but shorter loops score more. Input format: Plain text on stdin: R, followed by R rows of equal length C, characters from {'.', 'W', 'B'}, where `.` means empty cell, `W` means white pearl, and `B` means black pearl. Example input (rows separated here by semicolons): `6; ......; .W....; ....B.; ......; .B....; ....W.` Output format: R rows of length C. Each cell is one of: `.` (period) — not on the loop; `-` (dash) — straight segment, horizontal (connects E and W); `|` (vertical bar) — straight segment, vertical (connects N and S); `L` — corner connecting N and E; `J` — corner connecting N and W; `F` — corner connecting S and E; `7` — corner connecting S and W. A valid (and shortest, length 18) solution to the example (rows separated here by semicolons): `......; F-7...; L7L-7.; .|..|.; .L-7L7; ...L-J`. The loop length is the number of characters other than `.`: 18 here. Scoring: Each test case scores between 0 and 100: `score = 100 × quality × speed`. Validity: An invalid output (wrong dimensions, illegal character, broken or multiple loops, a pearl rule violated) scores 0. Quality: `quality = min(1, s × L_ref / L)²`, where L is your loop length and L_ref is the shortest loop length known to the organisers. On most test cases L_ref is proven to be the shortest possible and s = 1. On a few large ones it is only the best loop the organisers found, and s = 0.9: matching it earns 81%, and beating it by about 10% earns full quality. Because quality is squared, a loop 10% longer than L_ref keeps about 83% and one 25% longer about 64%. Speed: `speed = 0.5 + 0.5 × exp(−max(0, T − 1) / 5)`, where T is your program’s runtime in seconds on that test case. The first second is free; after that, slower answers lose up to half their score, never more. Your total is the sum over all hidden test cases. The sample test cases are not scored. Constraints: 6 ≤ R, C ≤ 50 (grid is rectangular, not necessarily square); at least 2 pearls; every puzzle has at least one valid loop; 100 hidden test cases of increasing size and difficulty; time limit: 10 seconds per test case (a timeout scores 0). Tips: Two strategies are scored: finding a valid loop at all (hard on densely pearled puzzles), and finding a short one (hard on sparsely pearled puzzles, where many loops are valid). A black pearl next to the border, or next to another black pearl, often has only one possible orientation. Three white pearls in a row force the loop to cross all three perpendicular to that row. The single-loop rule is global: a search that never checks for prematurely closed sub-loops wastes most of its time. Runtime counts. You can trade time for loop quality, but past the first second each extra second costs score; plan for both fast and slow paths.


Example Code
#include <assert.h>
#include <ctype.h>
#include <limits.h>
#include <math.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char* readline();
char* ltrim(char*);
char* rtrim(char*);

int parse_int(char*);



/*
 * Complete the 'solve' function below.
 *
 * The function is expected to return a STRING_ARRAY.
 * The function accepts STRING_ARRAY grid as parameter.
 */

/*
 * To return the string array from the function, you should:
 *     - Store the size of the array to be returned in the result_count variable
 *     - Allocate the array statically or dynamically
 *
 * For example,
 * char** return_string_array_using_static_allocation(int* result_count) {
 *     *result_count = 5;
 *
 *     static char* a[5] = {"static", "allocation", "of", "string", "array"};
 *
 *     return a;
 * }
 *
 * char** return_string_array_using_dynamic_allocation(int* result_count) {
 *     *result_count = 5;
 *
 *     char** a = malloc(5 * sizeof(char*));
 *
 *     for (int i = 0; i < 5; i++) {
 *         *(a + i) = malloc(20 * sizeof(char));
 *     }
 *
 *     *(a + 0) = "dynamic";
 *     *(a + 1) = "allocation";
 *     *(a + 2) = "of";
 *     *(a + 3) = "string";
 *     *(a + 4) = "array";
 *
 *     return a;
 * }
 *
 */
char** solve(int grid_count, char** grid, int* result_count) {

}

int main()