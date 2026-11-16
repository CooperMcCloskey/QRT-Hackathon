# Practice: square packing

This is our own reconstruction of the Imperial round's first problem, pieced
together from another team's code. The real statement, limits and scoring may
differ.

Place N axis-aligned squares on an integer grid so that no two overlap
(touching edges is fine). Minimise the area of the smallest axis-aligned
rectangle that contains them all.

**Input** (stdin): N on the first line, then N lines with one side length each.

**Output** (stdout): N lines `x y`, the integer corner with the smallest
coordinates of each square, in input order.

**Limits:** 1 ≤ N ≤ 128, 1 ≤ side ≤ 1000, time limit 8 seconds.

**Score:** bounding-rectangle area, lower is better.

Sample input:

```
3
2
1
1
```

Sample output (area 6):

```
0 0
2 0
2 1
```
