# Problem 3 handoff — 2026-11-16, 19:45 UTC

**Read this first. It supersedes older workflow instructions in this folder.**

## Update: four additional runs analysed, 20:30 UTC

- `feedback/collector_fcf0480865490964.json` contains all four LAST4 runs (51 cases each).
- Rebuilt model tag **CC44B484**. Held-out coefficient RMSE improved for COS1 (.1197→.1027), SIN1 (.1178→.1093), COS3 (.1633→.1346), SIN3 (.1796→.1269). These are prediction diagnostics, not judge-score claims.
- `final_candidate.c` exactly copies `sol_refined_CC44B484_v2.c` (scale 1.25, with target norm cap). It is compiled/output-checked. `last_candidate_console.js` provides **one validation run** of this candidate using the working panel.
- `final.c` still preserves measured best `sol_refined_E701CD73_v2.c` until that validation is available. No new probing method or basis is being started.
- Asked user for the actual remaining countdown because prior reviewer deadline (20:30) is now reached but user explicitly authorised more runs. Do not assume the contest is closed; obtain actual countdown if relevant.

## User-authorised additional refinement — 20:26 UTC

- User explicitly says there is time for **four more probing runs**. This extends work beyond the earlier stopping plan; keep refining the existing Fourier method.
- `last4_console.js`: exactly four N=4096 one-sided measurements at amplitude 1.0, ordered COS1, SIN1, COS3, SIN3. These are the existing components with relatively high uncertainty; no new basis or automation approach.
- Generator `make_last4.py`, metadata `last4_designs.json`, sources `probe_LAST4_*.c`. Same working manual-paste green panel and all-51 collector. Current best `final.c` stays preserved.
- `update_reconstruction.py` recognises LAST4 runs; run it once their feedback is imported. Use all prior measurements with empirical inverse-variance weights.

## Final selection — 20:26 UTC

- Latest export `feedback/collector_b3daab9c7ab6b12b.json` includes HYBRID1, 51 cases. Its mean clipped noisy feedback is **0.6029342992**, worse than refined v2 **0.6416463673**; its sum-raw/sum-target ratio is also lower (0.7635000927 versus 0.7763976800).
- Reject hybrid. **`final.c` exactly matches `sol_refined_E701CD73_v2.c`.** Preserve this selection.
- User must paste final.c back into Question 3 and use **Save & Proceed** to save the selected answer. Do not assume this has happened until confirmed.
- No further probing batches. Only finish saving the answer before the contest deadline.
- Official noiseless score is still unknown; >70% has not been confirmed. Never report 64.16% noisy feedback or the 77.64% raw aggregate ratio as the official score.

## Update: final selection ready to Submit, 20:20 UTC

- `feedback/collector_8cf763290d70d77a.json` contains both REBUILD_E701CD73 candidate runs, 51 cases each.
- Refined v1 mean clipped noisy feedback: 0.5441039821 (worse than previous). Refined v2: **0.6416463673**, versus previous best MODEL1 0.5776095588.
- `final.c` now exactly copies **`sol_refined_E701CD73_v2.c`**. Earlier final preserved as `final_before_refinement.c`. Selected source compiles locally and has recorded judge feedback for all 51 N=4096 cases.
- `final_selection.json` records metrics and the source. The 0.7763976800 sum-raw-noisy/sum-target ratio is a different noisy summary, NOT an official noiseless score or proof of >70 eligibility.
- **Have the user paste final.c and click Submit now, before 20:25.** Official score and submission confirmation remain unknown until the user reports them. Record that official result with `../tools/board.py judge` when supplied.
- No further broad probing batches. Preserve this candidate and secure the official submission.

## Update: refinements imported and rebuilt, 20:17 UTC

- `feedback/collector_389669aec64872f7.json` contains all six completed REFINE runs (51 cases each).
- Re-measured COS2/SIN2 remain noisy. Revised updater estimates per-run coefficient variance using held-out prediction error, then combines observations with inverse-variance weights. The earlier assumption that weight should always grow as h² was too optimistic for these observed runs.
- Current rebuilt tag **E701CD73**: `sol_refined_E701CD73_v1.c` (scale 1) and `_v2.c` (scale 1.25). They include spatial frequencies through 4 and shrink noisy components via smoothing selected by held-out error. Norm is capped using the observed target-energy constraint under the quadratic model.
- Both candidates compile, return 4096 finite bounded values on seven synthetic inputs, and are in `updated_models_console.js` for two runs with the working panel. One initial process startup exceeded a 2s local timeout; retries ran in 3–10ms and the complete checks passed with a 5s harness timeout. No judge runtime claim is made from that.
- Preserve `final.c` until this judge comparison is back. Ask user to run `updated_models_console.js` now, import the results, select best measured candidate, and submit by 20:25. Official >70 remains unconfirmed.

## Update: user requires official score above 70%, 20:13 UTC

- User explicitly says >70 is the absolute minimum. This has not yet been verified; do not confuse mean noisy Run Code feedback with the official noiseless result.
- Asked asynchronously whether a candidate has been Submitted and what official score appeared. Await actual evidence rather than assuming submission.
- The six existing REFINE runs remain the next data needed. At this update no `REFINE_*` results are saved locally.
- `update_reconstruction.py` now loads all saved scalar/wave/REFINE measurements, uses the supported known penalty 0.5 and exact discrete wave norms, and weights coefficient estimates by amplitude squared times norm squared. Smoothing remains selected by held-out error.
- It writes immutable `sol_refined_<tag>_v1.c` and `_v2.c` (scales 1 and 1.25), and `updated_models_console.js` for the working two-candidate panel. Tested once on the currently available old data; rebuilt candidates must await/refit the new REFINE data before being offered for final comparison. It preserves `final.c`.
- When REFINE data is imported: run `python3 update_reconstruction.py`, then give the user `pbcopy < .../updated_models_console.js`. Read the resulting judge feedback and select/Submit the final answer before 20:25.

## Update: model comparison saved, 20:09 UTC

- `feedback/collector_9d5d0260f6bd574d.json` contains the three candidate runs, 51 distinct t each.
- `candidate_comparison.json` and `.md` record the exact judge-feedback calculations. Mean clipped noisy feedback: MODEL1 0.5776095588, MODEL2 0.5156319503, MODEL3 0.5716131052. These are noisy feedback summaries, not official noiseless scores.
- `final.c` is an exact copy of `sol_wave_v1.c` (MODEL1). It has all 51 recorded judge cases and compiles locally. Recommend the user Submit this as the safety net; submission itself is not yet confirmed.
- MODEL1 versus MODEL3 is a near tie under noise. Preserve both versions for later comparison.
- `refine_console.js` contains the next six already-prepared Fourier refinements. Use the same working panel after the safety-net submission; do not Submit exploratory probe arrays.

## Update: root reviewer feedback read, approximately 20:08 UTC

- Read `/FEEDBACK.md` in full and re-read the current root `AGENTS.md`. **Lookup tables and feedback-learned values are explicitly allowed now.** Earlier statements here prohibiting them are obsolete.
- Root review says the contest ends at **20:30** and final submission should be in by **20:25**. The 20:10 approach freeze still applies.
- The page has active editor protection. All editor-writing and Run Code-clicking paths were removed from the current panel scripts and retired browser driver. The human performs those actions. Do not restore scripting of the protected editor.
- Current three candidate files are prepared; their judge feedback has not yet been saved at this update. Finish that comparison and get a scored answer submitted promptly.
- Prepared `refine_console.js`: six one-sided N=4096 probes, amplitude 1.5, ordered COS3, SIN3, SIN2 remeasure, COS2 remeasure, COS4, SIN4. Same manual panel workflow. Generator `make_refine_waves.py`; raw descriptions `refine_designs.json`; all six C files compiled and output-checked. Judge acceptance of amplitude 1.5 remains pending.
- For these runs use known-penalty estimate `<g,phi> = (y + .5*h*h*mean(phi^2))/h`, retaining exact discrete basis norm. Blend earlier measurements with variance-appropriate weighting. A larger amplitude improves precision only under the observed additive-noise hypothesis; reviewer recommends it from prior noise estimates.
- Current reconstruction approach already exists; these probes refine its coefficients. Do not spend remaining time developing another browser driver.

## Update: wave batch analysed, 20:05 UTC

- `feedback/collector_d708661bb64f8068.json` contains all original runs plus 10 completed wave runs, each with 51 distinct t and N=4096. High-N array output is judge-accepted.
- `wave_raw_analysis.json` records paired wave component/curvature estimates. Median quadratic penalty estimate is 0.5064. Large-array noise is still visible; smooth across t rather than fitting each noisy sample exactly.
- `fit_wave_model.py` fits each spatial coefficient as a cosine series in t plus a linear term, selecting ridge strength by leave-one-observation-out prediction error. The constant component also uses the earlier independent scalar probes.
- `wave_model.json` stores the fit and diagnostics. COS2 is strongly shrunk by cross-validation, while constant/COS1/SIN1/SIN2 carry more signal. These are model diagnostics, not official scores.
- Candidates: `sol_wave_v1.c` (model scale 1), `sol_wave_v2.c` (scale .8), `sol_wave_v3.c` (scale 1.2). Each returns 4096 samples of a general function in t and position u; no per-test answer table. All compiled and checked on seven synthetic inputs for length, finite values and bounds. No judge scores for these candidates yet.
- Next user action: paste `models_console.js` into the browser console. It uses the exact same working manual green-panel workflow for the three candidates, then export/import as before. Read their measured feedback and choose the best; do not assume the last candidate is best.
- No local objective checker is available because the judge's payoff is hidden. Do not report synthetic/local model predictions as judge scores or claim `run_tests.py` passed.

## Update: first full batch saved, approximately 20:00 UTC

- `feedback/collector_790001fcd57d9292.json` contains the complete first batch: 561 case records, 51 distinct t per round, 357 scalar records imported.
- Scalar paired estimates give quadratic penalty around 0.5 (median estimate 0.4976); see `batch1_analysis.json`. Scalar slopes are far too small to explain the noiseless targets. Do not equate the scalar optimum with the whole-question target.
- N=2 is accepted, target remains unchanged, and single-coordinate payoffs differ. Current hypothesis: the array samples an index-dependent decision function, payoff aggregates across entries. This needs direct high-N evidence.
- Next prepared experiment: `waves_console.js`, same now-working manual copy/run/collect panel, 10 runs at N=4096: ±0.5 constant, cos(2πu), sin(2πu), cos(4πu), sin(4πu), with u=i/4095. Each C source was compiled and exact output count/bounds checked. Sources `probe_WAVE_*.c`, descriptors `wave_designs.json`, generator `make_waves.py`.
- Wave batches save under the same browser storage key and append round IDs. Raw exports can therefore include the first batch again; deduplicate by round ID and case when analysing multiple exports.
- The importer now recovers `localStorage.qrt_bulk_results_v1` via the already functioning browser bridge when copying a terminal command overwrites clipboard JSON. User ran the importer successfully.
- No official positive final score is yet recorded. Use raw measured observations; model estimates are not judge scores.

## Current status and user preferences

- We own **Problem 3: Black box**. The user wants this problem maximised and is spending the remaining contest time on it.
- The user never writes or edits code. Supply complete files and exact copy commands. Keep explanations short; ask only for necessary runs/results.
- **The user just reported that the revised green-panel console batch appears to be working.** This confirms apparent workflow progress, not successful completion or a measured score.
- The user is currently running the batch. Let it finish; do not replace the editor, clipboard, console script, or generated probes during the batch.
- The authoritative current workflow is `bulk_console.js` with **manual C paste/run and automatic collection of all 51 cases**. Earlier automation attempts consumed too much time.
- Root AGENTS.md freezes new approaches at 20:10. At this update there are about 24 minutes before that freeze.

## Working user workflow

1. Terminal:

   ```bash
   pbcopy < /Users/coopermccloskey/Code/claude/QRT-Hackathon/p3/bulk_console.js
   ```

2. Paste into the **browser developer console**, where Claude's working collector was pasted. This is JavaScript; it must not go in the contest C editor.
3. The green panel supplies each experiment:
   - Click **1. COPY COMPLETE C CODE**.
   - Click the competition C editor, **Command+A → Command+V → Run Code**.
   - Wait for judge results, then click **2. RUN FINISHED — COLLECT ALL CASES** in the green panel.
   - The script reads all 51 cases and offers the next complete C program. Repeat.
4. After the batch, click **COPY ALL RESULTS**, then run:

   ```bash
   python3 /Users/coopermccloskey/Code/claude/QRT-Hackathon/p3/import_feedback.py
   ```

5. User says **feedback saved**. Read the resulting local files directly; do not ask the user to paste the full dataset into chat.

Browser recovery/export commands, only if needed:

```js
copy(JSON.stringify(window.qrtBulk))
// Persistent copy survives page refresh:
copy(localStorage.getItem('qrt_bulk_results_v1'))
```

The script saves after every completed run under `localStorage.qrt_bulk_results_v1`.
Repasting the script stops the old batch, replaces its panel and resumes saved
rounds. Use this only when recovery is needed. `DOWNLOAD RESULTS` exports JSON too.

## Current batch

Generated complete programs: `probe_bulk_<ID>.c`. The supplied C main and
OUTPUT_PATH input/output wrapper are preserved; `probe` returns the array below
and prints `QRT_BULK_<ID> t=... x=...` as diagnostic output.

| Order | ID | Returned array | Purpose |
|---|---|---|---|
| 1 | P05 | [0.5] | Positive scalar |
| 2 | M05 | [-0.5] | Matched negative scalar |
| 3 | P10 | [1.0] | Second amplitude |
| 4 | M10 | [-1.0] | Matched second amplitude |
| 5 | P05_R2 | [0.5] | Repeat for noise |
| 6 | M05_R2 | [-0.5] | Repeat for noise |
| 7 | ZERO | [0.0] | Intercept/reference |
| 8 | N2_ZERO | [0.0, 0.0] | Two-element reference |
| 9 | N2_LEFT | [0.5, 0.0] | First coordinate |
| 10 | N2_RIGHT | [0.0, 0.5] | Second coordinate |
| 11 | N2_BOTH | [0.5, 0.5] | Additivity/interaction |

This is up to 561 per-case observations, not 561 independent noise replicates.
Different t values can represent different objectives; compare matched inputs.
All 11 programs were locally compiled as C11 and checked for exact file output.
JavaScript syntax checks and a simulated-page batch test passed. Full live
collection/acceptance of these particular probes is pending the user's batch.

## Files and schemas

- `bulk_runner.js`: editable browser panel and sequential experiment logic. **Automatic editor access is currently disabled (`const editor = null`)** after it stalled on “Running P05”. Do not turn it on during the contest without evidence it works.
- `make_bulk.py`: builds `bulk_console.js` by embedding checked complete C programs plus `claude/collect.js`. Run only between user batches, if needed. It redirects the collector's completion callback and returns its promise; collection logic is Claude's script.
- `claude/collect.js`: user-reported working browser-console collector. Leave the other agent's source intact.
- `import_feedback.py`: saves the clipboard JSON to `feedback/collector_<hash>.json`, then appends known scalar observations to `observations.jsonl`.
- A bulk raw file contains `rounds[]`, each with `id`, `values`, `probe`, `observations[]`, `collector_diagnostics`.
- Each observation has `test_case`, displayed `t`, `N`, raw noisy payoff, noiseless target, optional `exact_t`, `candidate`, and `output` array.
- Raw vector observations stay in the feedback file; the journal imports **N=1 only**. Read raw rounds for N=2 analysis.
- Duplicate identical exports are ignored. Import the final batch once; different partial exports with overlapping rounds are not deduplicated per observation, so avoid treating repeated imports as independent measurements.
- `analysis.json`, `driver_feedback.json`, and `driver_state.json` belong to the **old failed drivers**. They are not the current batch output and may be empty or contain quarantined rows.

## Known judge observations

- Input is one float t; supplied C signature is `float* probe(float t, int* result_count)`.
- There is no in-process evaluator or feedback argument. Learning occurs across Run Code evaluations.
- Returns [0], [1], and [-1] passed all available cases in earlier runs.
- Judge feedback exposes `t`, `N`, `raw_noisy_score`, `raw_noiseless_target`, `normalized_noisy_score`.
- User supplied 51 case labels. Claude's `targets.json` records t=0,0.02,...,1 and a varying target per t. Its note says a zero run gave raw payoff zero across all 51 cases. These targets were provided by the other agent, rather than independently recollected by this agent.
- Verified individual t=0 readings: x=0 → raw 0; x=1 → raw -0.5035; x=-1 → raw -0.4290. Target 0.0148 for each. A separate visible snapshot of Claude's +1 run gives -0.5617 at t=0, evidence that identical outputs can yield different feedback.
- These are **judge-reported raw observations**, not locally computed or official final scores. No positive official final score is established in this session.
- Bonus eligibility depends on the official final **noiseless** score for the whole problem; noisy green passes/feedback do not establish eligibility.

## Mathematical work after data arrives

1. Validate collection: different case messages/t values, matching round output and N, exact t where available. Inspect errors and diagnostics before fitting.
2. At fixed t test `E[Y]=c(t)+a(t)x-b(t)x²`. With ±h and zero:
   `a=(y_plus-y_minus)/(2h)`, `b=(2*y_zero-y_plus-y_minus)/(2h²)`, stationary `x=a/(2b)` if b>0.
3. Use the second amplitude to test the model and repeats to measure noise. Three points fitting a quadratic proves nothing about model adequacy.
4. Pool curvature across t only if supported. Smooth/periodic regression may reduce noise; validate predictions against held-out runs. The old indexed Imperial write-up mentioned reconstructing a hidden curve wave by wave, but the linked source returned 404 and its actual method remains unverified.
5. Compare model peak `c+a²/(4b)` with the noiseless target. The target may not be the unconstrained scalar maximum; do not enforce equality without checking N semantics.
6. At N=2, contrast `f(.5,.5)-f(.5,0)-f(0,.5)+f(0,0)` tests interaction; hold N fixed in this calculation. Compare zero padding with N=1 to learn scaling/normalisation.
7. Choose candidates from repeated/fresh observations rather than the largest single noisy score. Preserve baseline storage and exact OUTPUT_PATH wrapper.

Current root protocol explicitly allows precomputed answer tables and values
learned from judge feedback. Table-based candidates from Claude are eligible
for comparison. No local checker can
calculate the hidden payoff from the supplied statement, so do not invent a
checker score or claim unperformed `run_tests.py` validation.

## Retired workflows / known failures

- Do not send the user back to `drive.py`, `arc_driver.py`, Apple Events, or terminal Enter loops.
- The original collector assigned the same visible t=0 message to all 51 cases; those are **not 51 independent observations**. Raw files are preserved and quarantined under `driver_state.json.unverified`.
- `claude/results_P_v1reader_note.json` explicitly notes this earlier failed collection. Do not use it as a dataset.
- Green test-case passes mean acceptance; raw/normalised payoff and the official noiseless result are separate.
- The user repeatedly requests all 51 cases per run and the fewest possible manual steps. Do not return to three-case screenshots.
