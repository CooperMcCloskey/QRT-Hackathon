# QRT C3 hackathon kit

Competition: 18:30-21:00. One folder per problem, one person per problem.

## Live competition

The competition has started. All five questions are **Approximate Solution**
problems.

| Question | Problem | Folder | Action |
|---|---|---|---|
| 1 | Square packing++ | `p1/` | Claim an owner and get a valid baseline submitted |
| 2 | Word Search Generation | `p2/` | Claim an owner and get a valid baseline submitted |
| 3 | Black box | `p3/` | Our next problem in this session; full statement needed |
| 4 | Masyu | `p4/` | Claim an owner and get a valid baseline submitted |
| 5 | Bonus: Document Lego | `p5/` | Locked until the bonus requirement is met |

**Bonus requirement: the judge score must be strictly greater than 70% on
each of Problems 1, 2, 3 and 4 before we can do Problem 5.** A score of exactly
70% does not qualify; an average above 70% does not qualify.

- Get valid submissions on all four main problems before polishing one.
- Then prioritise whichever main problems still need to exceed 70%; use
  recorded judge scores to track eligibility for the bonus.
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
