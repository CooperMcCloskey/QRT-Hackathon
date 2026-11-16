# Word Search Generation

## Problem statement

You are given two lists of words:

- **Required words** — every word in this list must appear somewhere in your grid, readable in a straight line in any of the 8 compass directions (horizontal, vertical, or diagonal; forwards or backwards).
- **Forbidden words** — none of these words may appear anywhere in your grid in any direction.

The grid wraps around at the edges (**torus topology**): a word may begin near one edge and continue from the opposite edge, in any of the 8 directions (see below)

Your task is to produce the **smallest possible square grid** (side length N) that satisfies both constraints. All cells must be filled with uppercase letters A—Z.

The supplied diagram is visible in [image.png](image.png).

## Function signature

```python
def generate_grid(required_words: list[str], forbidden_words: list[str]) -> list[str]:
```

## Example

Required words:

```text
ACE
FIG
OAK
```

Forbidden words:

```text
-
```

Output (one valid 3×3 grid)

```text
ACE
FIG
OAK
```

## Scoring

Each test case is scored by how densely your grid packs the required letters:

```text
S       = total number of letters in the required words
density = S / N²
score   = 100 × min(1, density / 1.7)
```

where N is your submitted grid size. Full marks need every cell to be used by two required words on average, so words must share letters heavily.

Example: S = 500 letters in a 25×25 grid gives density 0.8 and score approximately 47.06; the same words in a 20×20 grid give density 1.25 and score 73.53.

An invalid grid (missing required word, forbidden word present, non-square, non-letter cells, or N smaller than the longest required word) scores 0.

## Information absent from the supplied screenshots

Word counts and length limits, input normalization rules, time/memory limits, the complete judge wrapper, and guarantees or output conventions for impossible/empty inputs are not shown.
