# QRT C3 contest protocol

You are helping one member of a three-person team in a two-hour optimisation
contest (18:30-21:00). Follow this file exactly. Speed and a valid submission
matter more than elegant code.

## The contest

- Several optimisation problems, each with its own statement in `statement.md`
  in this folder. One folder per problem.
- Solutions are pasted into an online judge as **one source file** and run on
  **hidden tests**.
- Each test is scored by how close the answer is to the best possible answer.
  **An invalid or late output scores zero**, so a weak valid answer always
  beats a clever broken one.
- Assume standard library only, input on stdin, output on stdout, unless the
  statement's code template says otherwise. If the statement gives a code
  template, keep its input/output code untouched and fill in the function.

## The kit in this repo

Work inside the problem's own folder (`p1` … `p5`) and touch nothing outside
it. The repo root has tools that are already tested:

- `../templates/check.py`: checker skeleton. Copy it in and fill the TODOs.
- `../templates/sa.py` and `../templates/sa.cpp`: timed simulated-annealing
  skeletons with the time guard and best-answer logic already in place.
- `../tools/run_tests.py`: runs solutions on every `tests/*.in`, scores them
  with `check.py` and prints a table. Use it for every measurement:
  `python3 ../tools/run_tests.py --tl <judge limit in seconds> sol_v1.py sol_v2.cpp`
- `../tools/board.py`: the team scoreboard. `run_tests.py` saves every run to
  `scores.jsonl` for it; never delete or edit that file. When the human tells
  you the judge's score for a submitted version, record it with
  `python3 ../tools/board.py judge <solution file> <score>`.
- `../practice/squares`: a finished example of the whole flow.

Local quirks:

- The judge's Python is probably older than this laptop's. Use nothing newer
  than Python 3.8 syntax.
- If the judge's code template writes its answer to the file named by the
  `OUTPUT_PATH` environment variable, leave that as it is. `run_tests.py`
  handles both that and stdout.

## Step 0: restate the problem (no code yet)

Write `notes.md` with, in under 15 lines:

1. Input format and size limits.
2. Output format, including index base, coordinate convention and ordering.
3. Every validity rule an output must satisfy.
4. The objective, its direction (minimise or maximise) and the scoring formula.
5. Time and memory limits.
6. Anything ambiguous. Settle it from the sample input and output. If the
   samples do not settle it, ask the human one direct question and wait.

Never continue on a guessed constraint without saying so.

## Step 1: checker first

Write `check.py <input> <output>`, starting from `../templates/check.py`. It
must:

- validate every rule from Step 0 and print `INVALID: <reason>` with exit
  code 1 on the first violation;
- otherwise print `SCORE <value> <min|max>` as its last line, where the value
  is what the statement scores and `min` or `max` says which way is better;
- be written from the statement alone and share no code with any solver.

Run it on the statement's sample output. If the sample fails, the checker or
your reading of the problem is wrong. Fix that before anything else.

## Step 2: tests

Write `gen.py` and create `tests/` with small cases, typical cases, cases at
the maximum size, and edge cases (size 1, all values equal, extremes). Include
the statement's samples.

## Step 3: baseline within 15 minutes

Write `sol_v1`: the simplest method that always prints a valid answer. Run it
with `run_tests.py` on every test. Then tell the human, in these words:

> BASELINE READY: submit `sol_v1` now.

Do not start improving until you have said this.

## Step 4: improve

- Try two or three different approaches, in parallel if you can run subagents.
  Compare them on the same tests with the same checker.
- Save each version as a new file (`sol_v2`, `sol_v3`, …). Never overwrite the
  best version so far.
- If two versions each win on different kinds of input, combine them: run both
  inside the time limit and print the better answer.
- Use Python for the baseline. Move to single-file C++17 when the method is a
  search whose quality depends on iterations per second.

Starting points by problem type:

| Type | First approach | Then |
|---|---|---|
| Packing or layout | Greedy placement (largest first, bottom-left or skyline) over many candidate widths | Simulated annealing over the placement order |
| Grid construction (word search and similar) | Greedy placement that maximises overlap, smallest grid size first | Random restarts, then local search on placements |
| Noisy black box | Coarse sampling within the query budget | Refine around the best region; repeat queries to average the noise; return the point with the best mean, not the best single reading |
| Logic puzzle with a fixed answer | Constraint propagation plus backtracking | SAT-style encoding with loop or connectivity constraints added lazily |
| Reconstruction or ordering (shredded text and similar) | Pairwise fit cost between pieces, greedy chaining | 2-opt or or-opt on the order; letter-pair statistics for text |
| Anything else | Greedy construction | Simulated annealing: fast cost delta, temperature tied to the clock |

## Step 5: harden every version before it is submitted

- **Time guard.** Read the clock inside the search loop and stop at 75% of the
  time limit. The judge may be slower than this laptop. Measure the run time
  on the largest test.
- **Always hold a valid answer.** Build the baseline answer first, improve it
  in place, and print the best valid answer found when time runs out.
- **Fallback.** If the improved method throws or produces an invalid answer,
  print the baseline answer instead.
- **Fixed random seed**, so results repeat.
- **Exact output format.** Compare against the sample output character by
  character.
- **One file, no file reads, no extra libraries.**

A version may be submitted only when `run_tests.py` reports it as
`all valid and in time`.

## Step 6: report after every version

Keep it to a few lines:

- the `run_tests.py` table for the best previous version and the new one;
- what changed;
- the next idea and the gain you expect from it.

Then say either `SUBMIT sol_vN` or `DO NOT SUBMIT: <reason>`.

## Hard rules

- Never state a score you did not measure with `check.py`.
- If a new version is worse on any group of tests, say so plainly.
- If 15 minutes pass with under 1% improvement, tell the human that this
  problem has stalled, so they can move to another one.
- If the test inputs turn out to be visible rather than hidden, tell the human
  at once. Do not embed precomputed answers unless the human says the
  organisers allow it.
- After 20:10, start no new approaches. Only harden and re-check the best
  existing version.

---

## For the team (the agent can ignore this section)

**Setup per problem**

1. Paste the full statement, code template and samples into
   `pN/statement.md`, word for word.
2. Open your agent inside `pN/`. Claude Code and Codex load this file from the
   repo root on their own; for any other tool, paste this file in first.
3. Start with: `Read statement.md, then do Step 0 of the protocol.`

**Ask at the 18:00 briefing**

- Is code prepared before the event allowed (templates, a test runner)?
- Does the judge count the best submission or the last one?
- Is there a limit on submissions?
- What are the time limits per language?
- Which AI tools are allowed: any, or only OpenAI's?
- Are the test inputs hidden?
- Are precomputed answers inside a submission allowed?

**Clock**

| Time | What happens |
|---|---|
| 18:30–18:40 | Everyone skims every statement. Each person takes one problem. |
| by 18:55 | A valid baseline is submitted for each of those three problems. |
| by 19:10 | Baselines are submitted for the remaining problems too. |
| 19:10–20:10 | Improve. Every 20 minutes, compare points still available per problem and move effort to the biggest gap. |
| 20:10 | Freeze: no new approaches. |
| 20:25 | Confirm the best version of every problem is the one that counts. |

**Team rules**

- No problem is left at zero. A baseline on everything comes before polish on
  anything.
- Submit the baseline the moment the agent says it is ready.
- One person owns each problem's submissions, so a good score is never
  replaced by a worse one by accident.
