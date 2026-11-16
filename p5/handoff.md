# P5 Document Lego: handoff (written 20:08, contest ends 21:00)

## State in one line
Valid C++ submission exists, but it only scores about 10-11% on my local tests. No approach I tried rebuilds whole pages.

## What to submit
- `sol_v1.cpp` (46 KB): already announced as baseline. Local mean 10.6% over 7 tests.
- `sol_v2.cpp` (47 KB): same method, word-frequency-aware scoring. Local mean 11.0% (better on 4 tests, worse on t5). Not a real improvement; submit either.
- `sol_v1_bigdict.cpp` (113 KB): 25k-word list instead of 6k. Only if HackerRank accepts that size. Scores about the same locally.
- All run to a 1.6 s internal cut-off (`TLIM` in the source). Output is always valid.

## Unverified assumptions
- Time limit 2 s (not in the statement). If lower, reduce `TLIM`.
- Source limit 50 KB (my memory of HackerRank, not confirmed).
- Judge text is plain English prose. My tests use Wikipedia text (wikitext-2), which is harder vocabulary.
- Generator, inferred from the statement's example: greedy word wrap; a word that doesn't fit is hyphenated if at least 2 letters fit on each side, so a hyphen always sits in the last column and every other row has at most 3 trailing spaces; pieces are 3-7 wide and 1-3 tall; all-space pieces are dropped (so a row ending in 2+ spaces marks the page's right edge).
- Local scorer `check.py` compares characters after the best shift per page pair. The judge may score by piece instead.

## Why it is stuck (the main finding)
- Evidence only exists between horizontally adjacent pieces (and across a line wrap). Vertical neighbours share no text.
- Each junction row gives about 3 nats when it cuts a word and about 0.5 at a word boundary. Picking the right piece among ~400 needs about 6-7 nats.
- So a wrong piece chosen as "best of 400" scores as well as the true one. Measured: true right-neighbour is top-ranked for only 55 of 320 true adjacencies (t1).
- Only 3-row junctions are reliable alone. Everything else needs joint search over many pieces, or a stronger language model (word pairs / frequencies), which doesn't fit in 50 KB the way I built it.

## What was tried (all in this folder)
| File | Method | Result |
|---|---|---|
| `core_v1.cpp` | Row-major beam search (first free cell is always a piece's top-left corner), char trigram | ~5%, garbage: row 0 has no vertical check |
| `core_v3.cpp` | Same beam + embedded word list (word / prefix / suffix / infix checks), mutual-best pair links | ~4%; links are 96% pure but only ~10 per test |
| `core_v6.cpp` | Beam with leftmost-corner order inside a band of rows, so tall pieces are checked on all rows at once | ~4%; even from the true top-left piece with beam 300 it goes wrong in row 0 |
| `core_v5.cpp` | Merge fragments by posterior confidence (score minus log-sum of rivals), thresholds relaxed over passes | ~11%, best so far, this is `sol_v1/v2` |
| `core_v7.cpp` | v5 plus reverse (mutual) confidence check | 10-18% on 3 tests, no clear gain, slower |

## Ideas not yet tried, most promising first
1. Beam over a 2-3 row band with a much wider beam and scoring only tall pieces first, filling 1-row pieces into exact-width gaps afterwards.
2. Stronger evidence: per-word frequencies (not 4 buckets), common word pairs ("of the"), and words repeated within the input itself.
3. Right-edge pieces are identifiable (2+ trailing spaces, or letter + hyphen at row end). Use them to fix W and to chain rows via hyphenated words.
4. If the source limit is really larger than 50 KB, embed a char 5-gram model.

## How to rebuild and test
- Text corpus (not in the repo, re-download): wikitext-2 train/valid from `raw.githubusercontent.com/pytorch/examples/main/word_language_model/data/wikitext-2/`, `first20hours/google-10000-english`, `hermitdave/FrequencyWords` en_50k. Scripts expect them in `/home/claude/corpus/` and themselves in `/home/claude/p5/`; edit the paths.
- `python3 gen.py <seed> <pages> tests/tN` writes `tN.in` and `tN.truth`.
- `python3 prep3.py 6000 wl6k.inc` builds the compressed word list.
- `calib.cpp` (compile with `-DWLINC='"wl6k.inc"'`) writes the evidence tables `calib_lite.inc`.
- `python3 build.py core_v5.cpp sol.cpp wl6k.inc calib_lite.inc` makes the single-file submission.
- `./rt.sh <binary>` runs all `tests/t*.in` through `check.py`. Compile with `-DLOCAL` for stderr diagnostics and env overrides (`TS`, `C0`, `TLIM`).
- `fragcheck.py` / `regdbg.py` / `show.py` inspect an output against the truth.
- `tools/run_tests.py` was not used and `scores.jsonl` was not written for p5.
