# Step 0 — Black box (Problem 3)
Input: one float t on stdin, parsed by strtof; meaning and range unspecified.
Function: float* probe(float t, int* result_count); return static/dynamic array and set its length.
Output: template writes result_count floats in array order to OUTPUT_PATH, one %f per line, final newline.
Array length, element meanings/bounds, coordinate convention and index base: unspecified.
Known C contract: returned storage must remain alive and contain result_count readable floats.
Observed: [0], [1], [-1] pass available tests; 51 inputs t=0..1 step .02 reported by Claude.
Objective: maximise hidden payoff; payoff function and final aggregation are unspecified.
Noisy feedback: clamp(raw_noisy_score / raw_noiseless_target, 0, 1).
Bonus uses official final noiseless score for the whole question; noisy feedback is insufficient.
Time/memory limits, query/submission budget and feedback mechanism: unspecified.
No samples were supplied; user confirmed the screenshot is the entire statement.
User authorises empirical interface discovery: test explicit hypotheses using judge feedback.
Current workflow: bulk_console.js manual C paste/run, collect all 51; see HANDOFF.md first.
