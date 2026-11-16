# Black box experiments

**Current status (19:45 UTC): read `HANDOFF.md` first. The user reports the
revised green-panel batch is working. Older driver instructions below are
historical. Use manual C paste/run and the panel's COLLECT ALL CASES button.**

The user confirmed that the short statement is complete and authorised
learning the interface through test runs. Unknown constraints are hypotheses,
not established validity rules. Local compilation cannot measure hidden payoff.

## Probe 1: constant zero

- Source: `probe_v1.c`, derived from the supplied C template by filling only `probe`.
- Hypothesis: the judge accepts an array containing one float.
- Return `[0.0]` for every input `t`.
- Purpose: obtain the first validation message or payoff and inspect the feedback format.
- Local checks: C11 compilation succeeded; seven synthetic inputs produced exactly `0.000000\n` in OUTPUT_PATH with no stdout. Compiler warnings came from the unchanged template's null-pointer notation.
- Judge result: user screenshot says all available test cases passed. Displayed case: `t=0.00, N=1, raw_noisy_score=0.000, raw_noiseless_target=0.0148, normalized_noisy_score=0.000`. These are judge-reported observations, not locally computed scores.
- The feedback reveals t despite the hidden-test label; no precomputed answers are embedded.
- Use the platform's Run Code/test action first and retain all feedback.

## Next experiment depends on the response

- If rejected for shape or bounds, correct only the requirement revealed by the message.
- If accepted with numeric feedback, repeat the identical probe to check noise.
- Once the interface is established, vary one value at a time and compare repeated observations.
- Record raw feedback when available as well as normalized/clipped feedback.
- Treat the final official noiseless score as distinct from individual noisy observations.

Do not create a synthetic local payoff and report it as the judge score.

## Probe 2: constant one

- Source: `probe_v2.c`; `template.c` is the current paste-ready copy.
- Return `[1.0]` for every t; keep N=1 to isolate the effect of the value.
- Hypothesis: 1.0 is accepted and its feedback helps distinguish dependence on the returned value from dependence on array length.
- Judge result: pending. Obtain the same t=0 case for comparison.
- The zero probe remains saved unchanged as `probe_v1.c`.

Probe 2 result: all available tests passed; t=0.00, N=1, raw_noisy_score=-0.5035, raw_noiseless_target=0.0148, normalized_noisy_score=0.000 (user screenshot).

## Probe 3: constant minus one

Return [-1.0] with N=1 unchanged. Judge validity and payoff pending. Compare with +1 and 0; obtain messages from the first three cases to learn how t varies.

## Probe 4: batch across input contexts

- User explicitly requested many cases per run and minimal manual work.
- Source: `probe_v4.c`; current paste-ready copy: `template.c`.
- Return one of 16 signed scalar candidates, selected deterministically by a hash of input t.
- At t=0 the chosen value is -1, preserving the planned comparison with the recorded +1 observation.
- Stdout prints a diagnostic t/value mapping; the unchanged main writes the answer to OUTPUT_PATH.
- Different t values may have different objectives. Do not rank candidates across contexts as if t were fixed.
- A follow-up sign-reversed schedule can produce paired observations at each same t.
- Do not put all candidates in one returned array: whether N represents jointly scored coordinates or separate trials is unknown.
- `collect_results.js` is an optional browser-console helper to read visible per-case feedback into one JSON clipboard payload. Its UI selectors need verification on the actual contest page.
- Judge acceptance and payoff are pending; this is an exploratory run, not a final solution.

## Probe 5: matched sign reversal (prepared, not yet run)

- Source: `probe_v5.c`; negates probe_v4's scalar at every input t.
- Collect and save all probe_v4 feedback before running this file.
- `collect_results_v5.js` is the collector with the matching diagnostic marker.
- Combined with a zero reference at the same t, the pair can estimate linear and quadratic coefficients under a quadratic-payoff hypothesis. Independent amplitudes and repetitions are needed to test that hypothesis.
- Current `template.c` remains probe_v4 while its results are pending.

## Automatic diagnostic batch

- Run `python3 p3/drive.py` from the repo root, or use its absolute path.
- Ten full C programs: 0, +1, -1, +0.5, repeat +1, repeat -1, [0,0], [0.5,0], [0,0.5], [0.5,0.5].
- The six scalar runs identify/test a quadratic model and provide repeated observations; the four two-coordinate runs test array interaction.
- All generated programs are compiled and checked for exact output format before being offered to the user.
- The browser helper attempts editor control and clicks Run Code only. It observes the judge responses already requested by the page and falls back to collecting visible case messages.
- If editor automation is unavailable, the terminal supplies complete source files and the feedback collector through the clipboard.
- Feedback is saved under `feedback/`, progress in `driver_state.json`, and initial model estimates in `analysis.json`. No local official-score checker is claimed.
- After the user reports `batch done`, inspect those files and choose the next experiment or model implementation. No final submission is made by the driver.

## Probe 6: direct small negative scalar

Automation has not supplied reliable multi-case observations. Return [-0.05] in one complete C file and obtain manual compiler messages for cases 1, 26 and 51. This is a new controlled scalar experiment, not a claimed optimum or final submission. The value is near the unconstrained quadratic stationary point suggested by the available t=0 observations, but the quadratic/noise model and its behaviour at other t remain unverified. Original supplied input/output code is preserved.

## Console batch using the working reader

`bulk_console.js` contains 11 compiled/output-checked complete C probes and
the current `claude/collect.js` collector. Only its completion callback is
redirected so the batch can retain each result. The page panel supplies complete
C source for manual paste/run, then the user clicks RUN FINISHED — COLLECT ALL
CASES. Automatic editor control is disabled. It saves each completed round in
`localStorage.qrt_bulk_results_v1`. The user can copy/export the combined JSON
and save it with `import_feedback.py`. No Apple Events or terminal Enter loop.
The importer preserves vector observations in raw feedback and imports known
scalar outputs into observations.jsonl. Final noiseless scoring still requires
the official judge. Existing targets from `claude/targets.json` are observations
for model analysis; the new probes use constant values without per-test tables.
