from pathlib import Path
s=Path('finSolve.cpp').read_text()
s=s.replace('// Masyu, shortest loop. DFS with propagation for a valid loop, then simulated\n// annealing on face flips (keeps a single simple loop, pearl rules penalised).', '// Masyu hybrid: SAT repair, annealing, fast valid flips and path shortcuts.\n// Standalone C++17. Original finSolve.cpp is preserved.')
s=s.replace('static vector<char> satSolve(double deadline, const vector<char>* hint = nullptr) {', 'static vector<char> satSolve(double deadline, const vector<char>* hint = nullptr) {\n    allPearlsReq(); // A timed-out regional repair must never relax a full solve.')
s=s.replace('if (elapsed() > deadline) return false;\n        fill(onLoop', 'if (elapsed() > deadline) { allPearlsReq(); return false; }\n        fill(onLoop')
s=s.replace('if (rad > 2 * max(R, C)) break;', 'if (rad > 2 * max(R, C) || elapsed() >= deadline) break;')
s=s.replace('long long conflStart = conflictsTotal;', 'long long conflStart = conflictsTotal;\n    unsigned budgetTicks = 0;\n    if (elapsed() >= deadline) return -1;')
s=s.replace('        while (true) {\n            int cr = propagate();', '        while (true) {\n            if (((++budgetTicks & 63) == 0 && elapsed() >= deadline) || conflictsTotal - conflStart > conflLimit) { cancelUntil(0); return -1; }\n            int cr = propagate();')
# Keep only the submission pipeline, removing unused development modes and DFS.
a=s.index('// ------------------------------------------------------------------ DFS solver')
b=s.index('// ------------------------------------------------------------------ CDCL SAT')
s=s[:a]+s[b:]
a=s.index('int main() {')
s=s[:a]+Path('dev/main_optimized.inc').read_text()
for name in ['shortcut','polish']:
    p=Path('dev/'+name+'_optimized.inc')
    if p.exists():
        a=s.index('int main() {')
        s=s[:a]+p.read_text()+'\n'+s[a:]
Path('finSolve_optimized.cpp').write_text(s)
