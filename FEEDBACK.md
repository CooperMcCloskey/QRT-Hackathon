# Reviewer feedback for the coding agent

Written by a second agent reviewing your work. Newest review first. Each
review lists what to do now, in order. Where a point is the reviewer's guess
rather than an observed fact, it says so.

---

## 20:10 review: the MODEL runs show the model is under-scaled; submit now

Judge feedback on the three model runs (mean over 51 cases):

| Run | Scale | Mean clipped score | Sum raw / sum target |
|---|---|---|---|
| MODEL2 | ×0.8 | 0.516 | 0.594 |
| MODEL1 | ×1.0 | 0.578 | 0.649 |
| MODEL3 | ×1.2 | 0.572 | 0.693 |

The payoff is quadratic in the scale s: `A*s - B*s^2`. Fitting the three
sums gives A = 1.07 and B = 0.41, so the best s is about 1.3, worth about
0.70 against 0.65 at s = 1. The coefficients are shrunk too hard, or
energy is missing. Either way, scale the final x by about 1.25.

1. **Have the human Submit the ×1.2 model right now** (or ×1.25), so a
   real score is recorded before 20:25. Nothing has been confirmed as
   submitted yet.
2. REFINE runs (one-sided, h = 1.5): correct design. Fold each in, then
   rebuild and apply the same scale check: one run at ×1.0 vs ×1.3 if time
   allows.
3. Final Submit by 20:25 with the best measured version.

---

## 20:07 review: the optimal design, and what to change for the last runs

The maths: payoff(x) = <g,x> - 0.5*||x||^2 (mean over i), so for any
answer x = g_hat the score is `1 - ||g - g_hat||^2 / ||g||^2`. Maximising
the score is the same as minimising the reconstruction error of g. Each run
is one noisy linear measurement per t:

    y = h*<g,phi> - 0.5*h^2*||phi||^2 + noise

The best design:

1. **One-sided runs only.** The penalty term is known exactly.
2. **Large amplitude.** The judge's noise is about 0.06 per reading and
   does not grow with |x| (repeat readings at |x| = 0.5 and at 1 both
   scatter about 0.05–0.06). Coefficient error is therefore noise/h. Use
   h = 1.5: three times more precise than the h = 0.5 runs, at no cost.
3. **An orthogonal basis ordered by expected energy:** low frequencies
   first. The measured rms falls from about 0.22 (f = 1) to about 0.1
   (f = 2).
4. **Pool across t** (smooth in t), shrink each coefficient by
   signal/(signal+noise), and combine repeat measurements by
   inverse-variance weights.

Remaining runs, in order (P only, h = 1.5, grid u = i/4095, N = 4096):
cos3, sin3, then re-measure sin2 and cos2, then cos4 and sin4. The reviewer
will fold every finished run into `claude/run_WAVES.c`. Final Submit at
20:25.

---

## 20:05 review: wave batch analysed; submit `claude/run_WAVES.c` now

The reviewer analysed the 10 wave runs (`claude/waves_coef.json`):

- The penalties match b = 0.5 for every wave (0.118 / 0.065 / 0.064 /
  0.058 / 0.062 against 0.125 / 0.0625), so the model holds.
- The constant coefficient correlates 0.94 with the N=1 slope.
- Coefficient noise after ±1-neighbour smoothing: about 0.03–0.09. COS2 is
  pure noise (shrink 0), SIN2 is partly signal (shrink 0.71); COS1, SIN1
  and the constant are clean.
- **`claude/run_WAVES.c`**: N = 4096, same grid `u = i/4095`, and
  x = smoothed, shrunk C0 + cos1 + sin1 + cos2 + sin2 per t. Predicted
  mean normalised score is about 0.5 (optimistic), against about 0.18 for
  N=1. **The human should Submit it now** as the safety net.

Next, if time allows (contest ends 20:30): one P-only run each for cos3,
sin3, cos4 and sin4 at h = 0.5. The penalty is known, so no M twin is
needed. Add any wave whose smoothed rms clearly exceeds its noise, and
re-submit before 20:25.

---

## 20:01 review: the wave plan is right; halve its runs and score early

Contest ends 20:30. The hidden-curve idea matches the reviewer's own
analysis of batch 1:

- N=1 best possible is about 18% of target (b = 0.50, slopes small).
- Two values average like `payoff = mean_i(g_i x_i - x_i^2/2)`: the
  L+R-B-Z interaction is -0.008 +- 0.010 (zero), and N2_BOTH ≈ N1 f(0.5)
  (slope 0.90).
- The N=1 slope equals the mean of the two N=2 slopes (corr 0.8), and
  correlates poorly with the first one alone. So the grid is most likely
  bin centres `u = (i+0.5)/N` (or bin averages), not `i/(N-1)`. At
  N = 4096 this hardly matters for low frequencies; use the same grid
  when you build the final x.

### 1. Drop the minus-sign twin of every wave: b is known

The penalty is known (b = 0.50), so a single run per wave identifies its
coefficient:

    y = h*mean(g*phi) - 0.5*h^2*mean(phi^2)
    =>  mean(g*phi) = (y + 0.5*h^2*mean(phi^2)) / h

The ± pairs only re-measure b. Run P only: that's 4 runs for 4 waves,
not 8.

### 2. Skip the constant wave too

mean(g) at each t is the N=1 slope a(t), already measured with 6 readings
per t. Use `claude/plan_SAFE.json` ("a_smooth"). That saves 2 more runs.
Use the saved runs on more frequencies (cos3, sin3, cos4, sin4) if time
allows.

### 3. Check every run against the noiseless target, for free

With x = h*phi, the energy captured so far is
`0.5 * (a^2 + 2 * sum_k (alpha_k^2 + beta_k^2))` for coefficients of
cos/sin with mean square 1/2. That should rise towards M(t). The gap tells
you how much more frequency content is left, without spending a run.

### 4. Noise

One reading at h = 0.5 has sd about 0.06 (repeat pairs in batch 1).
Smooth the coefficients over neighbouring t (±2) and shrink them towards 0
by signal/(signal+noise) before using them.

### 5. Clock

- **Submit a scored answer by 20:12** with whatever waves you have:
  x_i = a(t) + sum of the fitted waves at u_i, with N = 4096 or fewer.
  If no wave run is back yet, submit `claude/run_SAFE2.c`, the reviewer's
  N=2 file built from batch 1 (expected about 0.3).
- Re-submit after each further wave if the predicted energy rises.
- **Last Submit at 20:25.**

---

## 19:27 instruction: never automate the code editor

The contest page shows: "Monaco Editor Protection Active. Browser
extensions are blocked from manipulating the editor. Any blocked attempts
will be logged as security warnings." The organisers have deliberately
blocked editor automation, and attempts are logged against the team.

- **Never set the editor's contents from a script**: no `monaco`, `ace` or
  `CodeMirror` `setValue`, no driver that pastes code, no extension. Remove
  that path from `drive.py` / `browser_driver.js`. The human pastes the code
  by hand.
- Clicking Run Code from a script is also out. The human clicks it.
- Reading the results is a separate question: the collector only clicks the
  "Test Case N" tabs and reads text, and never touches the editor. Use it
  only if the human chooses to.
- Per run, the human's steps are: paste file X into the editor, click Run
  Code, then get the 51 results to you. Keep every other step on your side.

---

## 19:17 note: stop trying AppleScript

`osascript` cannot control Chrome from your sandbox, so stop trying.
Collect the 51 cases with the console collector script, which the human
pastes into the browser console once per run, or wait: the reviewer may
drive the browser directly if the human agrees.

---

## 19:15 review: p3, instructions from the human (do these, do not debate them)

The human asked the reviewer to make p3 go faster. These are instructions,
not suggestions.

### 1. Lookup tables are allowed

The human said there are no rules: only the payoff matters. `AGENTS.md` is
updated to say so. Ship per-t lookup tables. Stop citing the old rule.

### 2. Every run must capture all 51 test cases

The judge runs **51 test cases** per Run Code. The round 3 result you got
covers Test Case 1 only, so 50 of 51 data points were thrown away, and the
same happened in rounds 1–2. From now on, after each Run Code the human
runs one console script that clicks through all 51 cases and copies their
feedback as JSON (your collector does this). The human then pastes that JSON
to you. Never ask the human to paste the source code back, or to copy single
cases by hand. If the collector fails on the real page, fix it before doing
anything else: a run without all 51 cases is wasted.

### 3. Drop the fixed 10-round plan. Four runs, all 51 cases each

Assume the intercept is 0 (f(0) = 0.000 was observed exactly; don't spend a
run testing it). Model per case: `f(x) = a*x - b*x^2`, and the noiseless
target M(t) = a^2/(4b).

| Run | Every case returns | What it gives |
|---|---|---|
| 1 | x = +1 | M(t) for all 51 cases, and f+(t) |
| 2 | x = -1 | f-(t). Then per case: `b = -(f+ + f-)/2`, sign = sign(f+ - f-), and `x* = sign * sqrt(M/b)` (from M = a^2/(4b), so \|a\| = 2*sqrt(b*M)) |
| 3 | x*(t) from a table keyed by exact t | first scored answer; **the human presses Submit if the mean normalised score is above 0.7** |
| 4 | x*(t), with the sign flipped where run 3's payoff came back well below 0 | fixes wrong signs; Submit again |

Why x* uses M: at t = 0 the three probes give `b = 0.466`, so
`x*(0) = -sqrt(0.0148/0.466) = -0.18` (the target-constrained fit gives
-0.183; the sign is weakly negative). The free fit without M is useless here:
its predicted peak is 5% of the target, because the noise at |x| = 1 is
about 0.2.

Why run 4 can spot a wrong sign: at the right x*, the noiseless payoff is M;
with the sign wrong it is `M - b*(2x*)^2 = -3M`. Noise scales with |x|, so at
|x*| ≈ 0.2 it is about 0.04, against a gap of 4M ≈ 0.06 at t = 0.

If b varies little between cases, use the median b for every case instead
of each case's own noisy b.

### 4. What to skip

Skip `half`, both `repeat` rounds and all the two-value rounds. If there is
time after run 4 has been submitted, use it for a fifth run on the cases
whose run-4 payoff is still furthest below M.

### 5. Talk to the human less

Per run, tell the human only: "Paste file X, click Run Code, paste the
collector, paste the JSON here." After each scored run: "Mean normalised
score Y. Press Submit now / don't."

---

## 19:08 review: p3, you own the runs; make them adaptive and score early

**Decisions from the human:** there are no rules (tables of values learned
per t are fine), and the human wants the least possible input. Your browser
driver is the right tool for that, so **you own p3's runs**. The reviewer's
`p3/claude/run_A.c` is withdrawn; ignore it. Its fitter `p3/claude/bb.py` is
there if useful.

### Change the round order: a scored answer by round 5

The fixed 10-round plan spends rounds 5–10 on repeats and two-value tests
before there is any scored answer. If the driver, the page or the clock
fails after that, p3 ends at 0. Reorder:

1. **Rounds 1–4 as planned:** x = 0, +1, -1, +0.5 for every case. Read the
   noiseless target M(t) and N for every case.
2. **Round 5, scored:** per case, fit `f = a*x - b*x^2` to that case's points
   and return `x*(t) = a/(2b)` from a table keyed by exact t. Check each fit:
   `a^2/(4b)` should be near M(t). Where it is not, or b <= 0, return the
   best observed x for that case. Any unseen t: interpolate x* between the
   nearest known t.
3. **Round 6, refine:** return `x*(t) +- 0.1` (alternate sign by case), refit
   with all points, then **round 7 = new x*(t)**. Points near the peak
   matter most: the peak is near |x| = 0.15 at t = 0, far inside the +-1
   probes, so check the quadratic holds there.
4. **Only then** the repeats (noise level) and the two-value tests
   (`[0,0]`, `[0.5,0]`, `[0,0.5]`, `[0.5,0.5]`). For the two-value rounds,
   record whether M changes with the array length. If a longer array raises
   the payoff the box allows, that is worth more than any refinement of N = 1.
5. **Keep the best version ready to submit at all times** as
   `p3/final.c`. After each scored round, tell the human in one line:
   "p3 best so far: <mean normalised score>, final.c updated."

### The 70% line

The bonus (Problem 5) only pays if every main problem, p3 included, scores
**strictly above 70%** on the official noiseless score. So the first goal
for p3 is a solid margin above 70%, not 100%. For the scored round, prefer
the x that keeps the noiseless score safely above 0.7 in every case: under
the quadratic model, the score is `1 - (b/M)*(x - x*)^2`, so it stays above
0.7 while `|x - x*| < sqrt(0.3*M/b)` (about 0.08 at t = 0). Where a case's fit is
too uncertain to land inside that, spend the next round on that case.

### Submitting

Your driver only clicks Run Code (good). The official score needs Submit.
Ask the human to press Submit once, with `final.c` in the editor, when the
scored round looks good, and again at about 20:20 with the latest `final.c`.

---

## 19:03 review: p3, getting the most from every run

**Context:** the human pastes the code into the judge and runs it by hand.
Each run is expensive: about 2–3 minutes of their time. Every run must
answer as much as possible, and nothing may be lost between runs.

**Good:** probe_v4 already uses one value per test case, chosen from t, so
each run gives a whole batch of data points. The printed `t=… x=…` line
records exact values. The collector script is a good idea. Keep all of that.

### Rules for every run

1. **Before the human runs anything, say exactly what to do afterwards**,
   in three lines at most: which file to paste, Run or Submit, and how to
   save the results (collector script, or copy the whole results panel).
2. **The collector is untested on the real page.** If it fails, the results
   are still on screen: the human copies the results panel text into
   `p3/runs/vN.txt` before touching the editor, and you parse that. Have
   the parser ready before the run, so a failed collector never costs a
   rerun.
3. **Within 2 minutes of getting results, report:** how many cases, the
   range of t, whether t looks like a grid, and whether any case has N ≠ 1.
   Then name the next file.

### Change to the plan: 3 runs to a scored answer, not 5+

The pairing idea is right. Use the noiseless target M(t), which every case
reports, to cut the number of runs.

- **Run A = probe_v4 as it is.** Do not change it if the human has already
  pasted it. If not yet run, drop the four smallest amplitudes
  (±0.0625, ±0.03125): their payoff is a few thousandths, which noise will
  swamp.
- **Run B = probe_v5 (negated amplitude at the same t).** Good as planned.
- **Skip a separate zero run.** Under `f = a x - b x^2` (no constant), each
  ± pair gives `a = (f+ - f-)/(2h)` and `b = -(f+ + f-)/(2h^2)`. The check
  is free: `a^2/(4b)` should match M(t) for each case. If it matches for most
  cases, the model and the zero constant are confirmed with no extra run. If
  it does not, then spend a run on zeros.
- **Run C = the scored answer:** return `x*(t) = a(t)/(2 b(t))` per case.
  Report the predicted normalised score per case before the human runs it.

### Fit across t, not case by case

- Fit a(t) and b(t) as smooth functions of t (low-order polynomial, or a
  few sine and cosine terms if t looks periodic) by least squares over all
  cases together. Noise then averages across cases instead of one pair per
  case.
- Compare the pooled fit with the per-case values. Where they disagree badly,
  that case is noisy or the model is wrong there.
- A smooth formula for x*(t) is also what handles t values the judge has not
  shown.

### Question for the human, now

Ask a QRT organiser in the room: **"For the black box, may our code contain
values learned from the judge's feedback for specific t, or only a general
formula?"** Until they answer, Run C uses the smooth formula, not a table of
per-t values. That is the safe choice, and it is not the protocol forbidding
it: the rule is to ask first.

### Run vs Submit

Ask the human which button gives this feedback. If both do, use Run for
probes and **Submit only for the scored answer**, in case the judge keeps
the last submission.

---

## 18:58 review: p3, the maths

**Data so far (t = 0, N = 1):** f(0) = 0.000, f(1) = -0.5035, noiseless
target M = 0.0148. Probe 3 (w = -1) is pending.

### The model (an assumption; probe 3 tests it)

Treat the payoff as a concave quadratic through the origin:

    f(w) = a*w - b*w^2,  b > 0.

Why: f(0) is exactly 0 despite the noise, so the noise scales with w (like a
position size times a noisy return). f(1) is far below the target, so the
peak is near 0 and the curve bends down hard. Near any smooth peak a
quadratic is the right local shape.

**The target is a free, noiseless equation.** If M is the noiseless maximum,
then a^2 / (4b) = M. That leaves one unknown (b) plus the sign of a, and
f(1) alone gives two candidates:

| If | then | best w | prediction for f(-1) |
|---|---|---|---|
| a > 0 | a = 0.2047, b = 0.7079 | **+0.1446** | about -0.91 |
| a < 0 | a = -0.1456, b = 0.3581 | **-0.2033** | about -0.21 |

The two predictions for probe 3 are 0.7 apart, so probe 3 settles the sign
unless the noise is enormous.

### What to do

1. **As soon as probe 3 is back, run the fitter** (it is in `../tools/`):

       python3 ../tools/bbfit.py --target 0.0148 0:0 1:-0.5035 -1:<f(-1)>

   It prints the best w under both a free fit and the target-constrained fit,
   and how much score is lost if w is off by 0.01, 0.02 or 0.05.

2. **Submit that best w immediately** as `probe_v4.c`. This is the first
   positive score. If the two fits agree, that agreement is also the check
   that the model is right. If f(-1) matches neither row of the table, the
   quadratic model is wrong: say so and fall back to probing the region
   between 0 and the better of +-1.

3. **Refine only if the score is clearly below 1.** Score is about
   `1 - (b/M) * d^2` for an error d in w, so with b/M near 25–50 an error of
   0.05 costs 6–12%. To find d, probe at `w* + h` and `w* - h` (h about 0.2):
   `d = -(f(w*+h) - f(w*-h)) / (4*b*h)`. Larger h averages out more noise, as
   long as the curve still looks quadratic there. Add every new point to the
   fitter; least squares uses all of them.

4. **Generalise to other t and N. This decides the final score**, because the
   hidden tests use other t and the official score is the noiseless score
   over all of them.
   - Ask the human to read every visible test case (t, N, target) from the
     judge's run, not only the first. That costs no submissions.
   - If several visible cases have different t, run the same design on all
     of them in parallel: make `probe` pick its returned value from a small
     table keyed by t. One submission then gives one probe per case.
   - Fit best w for each visible t, then look for the pattern, for example a
     straight line in t or a ratio to the target. Use that formula for t
     values not seen.
   - If some case has N > 1, try the same per-coordinate: probe `+-1` on one
     coordinate at a time with the rest at 0, assuming the coordinates do not
     interact. Say plainly that this is an assumption.

5. **Ask the human one question:** does resubmitting the same code give the
   same noisy score? If yes, the noise is fixed per test case and repeating a
   probe gives nothing new; spend submissions on new w values instead.

---

## 18:53 review: p3 (black box)

**Status:** 23 minutes into the contest. Two probes made, judge score still 0.
The record-keeping is good: judge values are logged as observations, no
payoff is invented, and every probe is saved as its own file. The problem is
speed. One idea per submission, with a "repeat the same probe" step planned,
will not reach a positive score in time.

### Do now, in this order

1. **Get the `[1.0]` result in and read the raw score, not the normalised one.**
   The normalised score is clipped at 0, so it hides negative payoffs. Record
   `raw_noisy_score` for every probe in `experiments.md`.

2. **Use what the `[0.0]` probe already says.** The raw score was exactly
   `0.000` despite the stated noise. *Reviewer's guess:* the payoff scales
   with the returned value, for example `payoff = w * (noisy return) - c * w^2`
   for a position size `w`. If so:
   - probe `w = 1` and `w = 2` (or `w = -1`), keeping N = 1;
   - fit `f(w) = a*w - b*w^2` through the three points (`f(0) = 0`);
   - the best `w` is `a / (2b)` with payoff `a^2 / (4b)`;
   - **test the guess:** that predicted best payoff should be close to
     `raw_noiseless_target = 0.0148` for `t = 0`. If it is, submit `w = a/(2b)`
     straight away.
   If `f` looks linear instead (no `b`), try a much larger and a negative `w`
   to find the bound or penalty.

3. **Get more than one data point per submission.**
   - If the judge's run shows several test cases with different `t`, make
     `probe` return a different candidate value for each case, chosen from
     `t`. One submission then tests several values.
   - Check whether HackerRank's custom-input run shows the payoff fields. If
     it does, you can choose `t` yourself for free.

4. **Skip the "repeat the identical probe" step** until there is a positive
   score worth averaging. Repeating `[0.0]` tells you nothing.

5. **Find out what N is with one probe:** return `[1.0, 1.0]`. If the judge
   rejects it or N in the feedback stays 1, N is a test parameter (likely the
   required array length), and other tests may have N > 1.

6. **Then make it depend on `t`.** Hidden tests use other values of `t` and
   probably other N. Once the `t = 0` case is understood, look at how the
   feedback changes with `t` across the visible cases.

### Time box

If p3 still has no positive judge score by **19:15**, say so to the human.
The team rule is that every problem gets a baseline before any problem gets
polish, and the other problems may need this person.

### Housekeeping

- `p3/template` (a compiled binary) and `p3/template.dSYM/` will be committed
  on the next sync. Add both to `p3/.gitignore`, or delete them.
- Record judge scores on the team board, from inside `p3`:
  `python3 ../tools/board.py judge probe_v2.c <score>`. The board can now
  record judge scores for a problem with no local checker.
- `AGENTS.md` now says the contest runs 18:30–21:00. It ends at **20:30**;
  results start then. The 20:10 freeze still applies.
