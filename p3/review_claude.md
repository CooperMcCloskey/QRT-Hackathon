# Review of the other agent's work — 19:09 UTC

**See `HANDOFF.md` for current status; the workflow references below are historical.**

Read `claude/bb.py`, `claude/run_A.c`, and `claude/collect_A.js` in full.
There is no `claude/obs.jsonl` or saved results file yet. Its fitter currently
has the same two t=0 observations that we have; it adds hypotheses, not new evidence.

## Useful advice incorporated

- Pair opposite scalar probes at each same t and use raw noisy payoff.
- Treat a concave quadratic as a testable hypothesis.
- Compare the fitted maximum with the reported noiseless target. Our model
  retains an intercept c; the corresponding peak is c + a^2/(4b).
- `drive.py` now records this target-consistency diagnostic and the range of
  reported targets for each case. Equality is not enforced until the target's
  meaning and the quadratic model are supported by observations.
- `driver_feedback.json` exposes the saved observations in collector-compatible
  form so both agents can use them. Actual output arrays are preserved; scalar
  `candidate` is set only for N=1. Source filenames identify every experiment.

## Parts not adopted

- `bb.py` declares DONE from the largest single noisy reading near a candidate.
  This is vulnerable to selection bias and does not establish the official
  noiseless result. Our selection requires repeated/fresh observations.
- `write_c` generates per-test t/value lookup tables. The root contest protocol
  prohibits precomputed answers unless the organisers allow them. We will fit
  and validate a general function rather than ship those lookup tables.
- Its free fit fixes the intercept to zero. One observed zero payoff is not
  enough to establish that constraint for all t/noise conditions.
- It warns about N != 1 but still passes those observations into the same fit.
  Our scalar and two-coordinate analyses remain separate.
- Rounding t before grouping can merge distinct inputs. Our grouping includes
  case identity, and we retain exact t whenever the diagnostic log is available.

The other agent's files were left untouched. The current run command remains
`python3 p3/drive.py`.

## Updated workflow after user confirmation

The user reports that `claude/collect.js` works when pasted into the browser
console. Use that existing collector unchanged to collect all 51 cases. Do
not reduce collection to three cases or restart the failed automation loop.
`import_feedback.py` saves its clipboard JSON and imports scalar observations
with known candidate values into the shared observation journal. It preserves
uncertain/error/vector records in the raw file for inspection.

The latest `claude/bb.py` additionally pools curvature across inputs, considers
noise proportional to output magnitude, and smooths signs. These remain
model hypotheses until full feedback is available; pooling may be useful if
the curvature estimates support a shared value. No new observations were
present in that folder at this review, only a note about the earlier failed
reader repeating the same t=0 result 51 times.
