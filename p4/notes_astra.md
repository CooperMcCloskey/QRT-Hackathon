# Astra Masyu solver
1. Input: R and R equal-width rows, 6 <= R,C <= 50; at least two pearls.
2. Output: R rows over . - | L J F 7; coordinates are row/column internally.
3. Exactly one reciprocal degree-two cycle visits every pearl, no crossings.
4. White goes straight and at least one adjacent loop cell turns.
5. Black turns and both adjacent loop cells go straight.
6. Minimize length; score = 100 min(1,s Lref/L)^2 (0.5+0.5 exp(-max(0,T-1)/5)).
7. Ten seconds judge limit; Astra hard search deadline 7.3 seconds.
8. Partial wrapper: standalone stdin/stdout, honour OUTPUT_PATH when present.
9. Only files named with an _astra suffix are owned by this implementation.
