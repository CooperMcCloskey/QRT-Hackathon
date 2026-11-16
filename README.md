# QRT C3 hackathon kit

Competition: 18:30–20:30. One folder per problem, one person per problem.

## Live competition

The competition has started. All five questions are **Approximate Solution**
problems.

| Question | Problem | Folder | Who |
|---|---|---|---|
| 1 | Square packing++ | `p1/` | Teammate |
| 2 | Word Search Generation | `p2/` | Teammate |
| 3 | Black box | `p3/` | This laptop (only this problem) |
| 4 | Masyu | `p4/` | Teammate |
| 5 | Bonus: Document Lego | `p5/` | Anyone with spare time |

**Bonus rule: Problem 5 can be worked on and submitted at any time, but its
points only count if the final score is strictly greater than 70% on each of
Problems 1, 2, 3 and 4.** Exactly 70% does not qualify, and an average above
70% does not qualify. For Problem 3 this is the official noiseless score, not
the noisy feedback.

- Work on Problem 5 can start early, but it only pays off if all four main
  problems end above 70%. So a main problem stuck at or below 70% comes first.
- Use recorded judge scores to track which main problems are still at or
  below 70%, and help whichever one is furthest behind.
- Keep one submission owner per problem and share ownership with the team.
- At 20:10, stop starting new approaches and harden/re-check existing solutions.
- By 20:25, confirm the best eligible version of every attempted problem is
  the one that counts.

## Start a problem

1. Paste the full statement, code template and samples into `pN/statement.md`.
2. Open your coding agent **inside `pN/`** and say:
   `Read statement.md, then do Step 0 of the protocol.`
   The protocol (`AGENTS.md` at the repo root) loads automatically in Claude
   Code and Codex. For any other tool, paste `AGENTS.md` in first.
3. Submit the baseline as soon as the agent says `BASELINE READY`.

## Score solutions locally

From inside the problem folder:

```bash
python3 ../tools/run_tests.py --tl 2 sol_v1.py sol_v2.cpp
```

It runs every `tests/*.in` through each solution and `check.py`, then prints a
score table. `--tl` is the judge's time limit in seconds.

## Live scoreboard

Every `run_tests.py` run is saved to `scores.jsonl` in the problem folder.

```bash
python3 tools/board.py --watch 10
```

After you submit a version, record what the judge gave it (from inside the
problem folder):

```bash
python3 ../tools/board.py judge sol_v2.py 87.5
```

Share scores and code between laptops:

```bash
python3 tools/board.py sync
```

## What is here

| Path | Purpose |
|---|---|
| `AGENTS.md` | The protocol the agent follows, plus the team clock and briefing questions |
| `tools/run_tests.py` | Runs and scores solutions on all local tests |
| `tools/board.py` | Scoreboard across all problems, judge scores, and sync |
| `templates/` | Checker skeleton and timed simulated-annealing skeletons (Python, C++) |
| `p1` … `p5` | One folder per contest problem |
| `practice/squares` | A worked example: square packing, start to scored answer |

## Sharing work tonight

- Only touch your own problem folder, so nobody overwrites anyone else.
- The competition is live: commit and sync your own problem's work so the
  team can see progress. Never delete or edit `scores.jsonl`.
