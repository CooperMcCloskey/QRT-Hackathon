# P5 Document Lego
1. Input: N followed by N rectangular pieces, rows separated by |, spaces encoded as _, IDs 0..N-1; no rotations.
2. Real pages have independent widths 40..60 and heights 10..14; the sample is smaller. N and piece-size bounds are unspecified.
3. Output: piece page row col, four integers per line; any page labels and negative coordinates are allowed.
4. Unknown IDs or malformed lines invalidate the output. Duplicate pieces and overlaps cause later placements to be ignored.
5. Omitted pieces are treated as distractors. Each predicted page is matched to at most one real page, with a free whole-page translation.
6. Maximise correctly positioned non-space characters; score is their percentage. Official matching implementation is unavailable.
7. Time, memory, and source-size limits are missing; asked user. Handoff assumed 2 s and 50 KB without confirmation.
8. Current sample input exists but its reference placement/truth is absent, so its reconstruction score cannot yet be checked.
9. Existing generated tests include truth; their checker is a local scoring model, not the official judge.
