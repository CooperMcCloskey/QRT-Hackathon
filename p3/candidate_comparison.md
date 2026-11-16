# Measured candidate comparison

Mean clipped noisy feedback, computed from all 51 saved judge messages. This is not the official noiseless result.

| Candidate | Cases | Mean clipped noisy feedback |
|---|---:|---:|
| MODEL1 | 51 | 0.5776 |
| MODEL2 | 51 | 0.5156 |
| MODEL3 | 51 | 0.5716 |

`final.c` is an exact copy of `sol_wave_v1.c`. MODEL1 and MODEL3 are close; one run does not establish a reliable noise-free ordering.

Next: submit final.c, then run the six already-prepared one-sided wave refinement probes in refine_console.js.
