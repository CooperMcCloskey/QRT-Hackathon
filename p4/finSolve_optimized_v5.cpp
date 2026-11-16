// Masyu hybrid: SAT repair, annealing, fast valid flips and path shortcuts.
// Standalone C++17. Original finSolve.cpp is preserved.
#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>
#include <vector>
using namespace std;

static const chrono::steady_clock::time_point T0 = chrono::steady_clock::now();
static double elapsed() { return chrono::duration<double>(chrono::steady_clock::now() - T0).count(); }
static uint64_t rs = 88172645463325252ULL;
static inline uint64_t rnd() { rs ^= rs << 7; rs ^= rs >> 9; return rs; }
static inline int rndi(int n) { return (int)(rnd() % (uint64_t)n); }
static inline double rnd01() { return (double)(rnd() >> 11) * (1.0 / 9007199254740992.0); }

static int R, C, N, E, HN;
static vector<string> G;
static vector<int> eu, ev;            // edge endpoints
static vector<int> nbr;               // nbr[c*4+d]
static vector<int> cedge;             // cedge[c*4+d]
static const int DR[4] = {-1, 0, 1, 0}, DC[4] = {0, 1, 0, -1};  // N E S W
static vector<char> isPearl;
static int nPearls;
static vector<int> pearls;

struct Lit { int e; int v; };
struct Pattern { vector<vector<Lit>> cfg; };
static vector<Pattern> pat;           // per pearl index
static vector<vector<int>> edgePearls;

static int stepE(int c, int d) { return c < 0 ? -1 : cedge[c * 4 + d]; }
static int stepC(int c, int d) { return c < 0 ? -1 : nbr[c * 4 + d]; }

static void build() {
    N = R * C;
    HN = R * (C - 1);
    E = HN + (R - 1) * C;
    eu.assign(E, 0); ev.assign(E, 0);
    for (int r = 0; r < R; r++) for (int c = 0; c + 1 < C; c++) { int e = r * (C - 1) + c; eu[e] = r * C + c; ev[e] = r * C + c + 1; }
    for (int r = 0; r + 1 < R; r++) for (int c = 0; c < C; c++) { int e = HN + r * C + c; eu[e] = r * C + c; ev[e] = (r + 1) * C + c; }
    nbr.assign(N * 4, -1); cedge.assign(N * 4, -1);
    for (int r = 0; r < R; r++) for (int c = 0; c < C; c++) {
        int x = r * C + c;
        for (int d = 0; d < 4; d++) {
            int nr = r + DR[d], nc = c + DC[d];
            if (nr < 0 || nr >= R || nc < 0 || nc >= C) continue;
            nbr[x * 4 + d] = nr * C + nc;
            int e;
            if (d == 0) e = HN + nr * C + nc;
            else if (d == 2) e = HN + r * C + c;
            else if (d == 1) e = r * (C - 1) + c;
            else e = r * (C - 1) + nc;
            cedge[x * 4 + d] = e;
        }
    }
    isPearl.assign(N, 0);
    for (int r = 0; r < R; r++) for (int c = 0; c < C; c++) if (G[r][c] != '.') { isPearl[r * C + c] = 1; pearls.push_back(r * C + c); }
    nPearls = pearls.size();
    pat.resize(nPearls);
    edgePearls.assign(E, {});
    for (int pi = 0; pi < nPearls; pi++) {
        int p = pearls[pi];
        char t = G[p / C][p % C];
        auto& cf = pat[pi].cfg;
        auto addOn = [&](vector<Lit>& L, int e) { if (e < 0) return false; L.push_back({e, 1}); return true; };
        auto addOff = [&](vector<Lit>& L, int e) { if (e >= 0) L.push_back({e, 0}); };
        if (t == 'B') {
            for (int dx : {1, 3}) for (int dy : {0, 2}) {
                vector<Lit> L; bool ok = true;
                int px = stepC(p, dx), py = stepC(p, dy);
                ok &= addOn(L, stepE(p, dx)); ok &= addOn(L, stepE(px, dx));
                ok &= addOn(L, stepE(p, dy)); ok &= addOn(L, stepE(py, dy));
                if (!ok) continue;
                addOff(L, stepE(p, dx ^ 2)); addOff(L, stepE(p, dy ^ 2));
                addOff(L, stepE(px, dy)); addOff(L, stepE(px, dy ^ 2));
                addOff(L, stepE(py, dx)); addOff(L, stepE(py, dx ^ 2));
                cf.push_back(L);
            }
        } else {
            for (int a : {0, 1}) {  // a=0 vertical (N,S), a=1 horizontal (E,W)
                int d1 = a, d2 = a + 2, q1 = (a + 1) % 4, q2 = (a + 3) % 4;
                vector<Lit> base; bool ok = true;
                ok &= addOn(base, stepE(p, d1)); ok &= addOn(base, stepE(p, d2));
                if (!ok) continue;
                addOff(base, stepE(p, q1)); addOff(base, stepE(p, q2));
                int n1 = stepC(p, d1), n2 = stepC(p, d2);
                int b1 = stepE(n1, d1), b2 = stepE(n2, d2);
                // (a) beyond-1 off
                { vector<Lit> L = base; addOff(L, b1); cf.push_back(L); }
                // (b) beyond-1 on, beyond-2 off
                if (b1 >= 0) { vector<Lit> L = base; L.push_back({b1, 1}); addOff(L, b2); cf.push_back(L); }
            }
        }
        for (auto& L : cf) for (auto& l : L) {
            auto& v = edgePearls[l.e];
            if (v.empty() || v.back() != pi) v.push_back(pi);
        }
    }
}

// ------------------------------------------------------------------ CDCL SAT
static vector<char> reqP, bldP;   // pearls the loop must contain / pearls whose rules are encoded
static int nReq = 0;
static void allPearlsReq() { reqP = isPearl; bldP = isPearl; nReq = nPearls; }
namespace cdcl {
static int nv;
static vector<int> arena;                 // clause literals
struct Cl { int start, size; bool learnt; float act; };
static vector<Cl> cls;
static vector<vector<int>> watches;       // by literal
static vector<signed char> asg;           // per var: -1 undef, 0 false, 1 true
static vector<int> lvl, reason, trailv, trailLim;
static vector<signed char> phase;
static vector<double> act;
static double varInc = 1, clInc = 1;
static int qhead = 0;
static bool unsatFlag = false;
static vector<char> seenv;
// heap over vars by activity
static vector<int> heap, hpos;
static inline bool hlt(int a, int b) { return act[a] > act[b]; }
static void hup(int i) {
    int v = heap[i];
    while (i > 0) { int p = (i - 1) >> 1; if (!hlt(v, heap[p])) break; heap[i] = heap[p]; hpos[heap[i]] = i; i = p; }
    heap[i] = v; hpos[v] = i;
}
static void hdown(int i) {
    int v = heap[i], n = heap.size();
    while (true) {
        int c = 2 * i + 1; if (c >= n) break;
        if (c + 1 < n && hlt(heap[c + 1], heap[c])) c++;
        if (!hlt(heap[c], v)) break;
        heap[i] = heap[c]; hpos[heap[i]] = i; i = c;
    }
    heap[i] = v; hpos[v] = i;
}
static void hinsert(int v) { if (hpos[v] >= 0) return; heap.push_back(v); hpos[v] = heap.size() - 1; hup(heap.size() - 1); }
static int hpop() { int v = heap[0]; int last = heap.back(); heap.pop_back(); hpos[v] = -1; if (!heap.empty()) { heap[0] = last; hpos[last] = 0; hdown(0); } return v; }

static inline int litVal(int l) { int a = asg[l >> 1]; return a < 0 ? -1 : (a ^ (l & 1)); }
static inline int level() { return trailLim.size(); }
// native at-most-K over a var set (lazy reasons, reason id -2)
static bool cardOn = false;
static int cardK = 0, cardCnt = 0;
static vector<int> cardVars, tpos;
static vector<char> cardIn, counted;
static vector<int> cardBuf;
static inline void enqueue(int l, int r) {
    int v = l >> 1; asg[v] = !(l & 1); lvl[v] = level(); reason[v] = r; tpos[v] = trailv.size(); trailv.push_back(l);
}
static void buildCardReason(int p) {
    cardBuf.clear(); cardBuf.push_back(p);
    int tp = tpos[p >> 1];
    for (int t : cardVars) if (counted[t] && tpos[t] < tp) cardBuf.push_back(2 * t + 1);
}

static void init(int n) {
    nv = n; arena.clear(); cls.clear();
    if ((int)watches.size() > 2 * n) watches.resize(2 * n);
    for (auto& w : watches) w.clear();
    watches.resize(2 * n);
    asg.assign(n, -1); lvl.assign(n, 0); reason.assign(n, -1); trailv.clear(); trailLim.clear();
    phase.assign(n, 0); act.assign(n, 0); seenv.assign(n, 0);
    tpos.assign(n, 0); cardIn.assign(n, 0); counted.assign(n, 0); cardVars.clear(); cardOn = false; cardCnt = 0; cardK = 1 << 30;
    heap.clear(); hpos.assign(n, -1);
    for (int v = 0; v < n; v++) hinsert(v);
    qhead = 0; unsatFlag = false; varInc = 1; clInc = 1;
}

static void cancelUntil(int lv) {
    if (level() <= lv) return;
    for (int i = (int)trailv.size() - 1; i >= trailLim[lv]; i--) {
        int v = trailv[i] >> 1; phase[v] = asg[v]; asg[v] = -1; reason[v] = -1; hinsert(v);
        if (counted[v]) { counted[v] = 0; cardCnt--; }
    }
    trailv.resize(trailLim[lv]); trailLim.resize(lv); qhead = trailv.size();
}

static int addClauseRaw(const vector<int>& lits, bool learnt) {
    int cr = cls.size();
    cls.push_back({(int)arena.size(), (int)lits.size(), learnt, 0});
    for (int l : lits) arena.push_back(l);
    watches[lits[0]].push_back(cr); watches[lits[1]].push_back(cr);
    return cr;
}

// add a problem clause at level 0
static void addClause(vector<int> lits) {
    if (unsatFlag) return;
    cancelUntil(0);
    sort(lits.begin(), lits.end()); lits.erase(unique(lits.begin(), lits.end()), lits.end());
    vector<int> k;
    for (size_t i = 0; i < lits.size(); i++) {
        if (i + 1 < lits.size() && (lits[i] ^ 1) == lits[i + 1]) return;  // tautology
        int v = litVal(lits[i]);
        if (v == 1) return;
        if (v == -1) k.push_back(lits[i]);
    }
    if (k.empty()) { unsatFlag = true; return; }
    if (k.size() == 1) { enqueue(k[0], -1); return; }
    addClauseRaw(k, false);
}

static int propagate() {
    while (qhead < (int)trailv.size()) {
        int p = trailv[qhead++];
        if (cardOn && !(p & 1) && cardIn[p >> 1]) {
            counted[p >> 1] = 1; cardCnt++;
            if (cardCnt > cardK) {
                int cr = cls.size();
                cls.push_back({(int)arena.size(), 0, false, 0});
                for (int t : cardVars) if (counted[t]) arena.push_back(2 * t + 1);
                cls[cr].size = arena.size() - cls[cr].start;
                return cr;
            }
            if (cardCnt == cardK) for (int u : cardVars) if (asg[u] < 0) enqueue(2 * u + 1, -2);
        }
        int fl = p ^ 1;
        vector<int>& ws = watches[fl];
        size_t i = 0, j = 0, n = ws.size();
        while (i < n) {
            int cr = ws[i++];
            int* c = &arena[cls[cr].start];
            if (c[0] == fl) { c[0] = c[1]; c[1] = fl; }
            if (litVal(c[0]) == 1) { ws[j++] = cr; continue; }
            int sz = cls[cr].size; bool found = false;
            for (int k = 2; k < sz; k++) if (litVal(c[k]) != 0) {
                c[1] = c[k]; c[k] = fl; watches[c[1]].push_back(cr); found = true; break;
            }
            if (found) continue;
            ws[j++] = cr;
            if (litVal(c[0]) == 0) {
                while (i < n) ws[j++] = ws[i++];
                ws.resize(j);
                return cr;
            }
            enqueue(c[0], cr);
        }
        ws.resize(j);
    }
    return -1;
}

static void bumpVar(int v) {
    act[v] += varInc;
    if (act[v] > 1e100) { for (auto& a : act) a *= 1e-100; varInc *= 1e-100; }
    if (hpos[v] >= 0) hup(hpos[v]);
}

static void analyze(int confl, vector<int>& out, int& btlev) {
    out.clear(); out.push_back(-1);
    int pathC = 0, p = -1, idx = trailv.size() - 1;
    do {
        const int* L; int csz;
        if (confl == -2) { buildCardReason(p); L = cardBuf.data(); csz = cardBuf.size(); }
        else { Cl& c = cls[confl]; if (c.learnt) { c.act += clInc; } L = &arena[c.start]; csz = c.size; }
        for (int k = (p == -1 ? 0 : 1); k < csz; k++) {
            int q = L[k], v = q >> 1;
            if (!seenv[v] && lvl[v] > 0) {
                seenv[v] = 1; bumpVar(v);
                if (lvl[v] >= level()) pathC++; else out.push_back(q);
            }
        }
        while (!seenv[trailv[idx] >> 1]) idx--;
        p = trailv[idx]; idx--;
        confl = reason[p >> 1]; seenv[p >> 1] = 0; pathC--;
        // reason clause has p as c[0]
    } while (pathC > 0);
    out[0] = p ^ 1;
    static vector<int> toClear; toClear.assign(out.begin() + 1, out.end());
    // simple minimisation: drop literals whose reason is subsumed
    size_t j = 1;
    for (size_t i = 1; i < out.size(); i++) {
        int v = out[i] >> 1, r = reason[v];
        if (r < 0) { out[j++] = out[i]; continue; }
        int* L = &arena[cls[r].start]; bool keep = false;
        for (int k = 1; k < cls[r].size; k++) { int u = L[k] >> 1; if (!seenv[u] && lvl[u] > 0) { keep = true; break; } }
        if (keep) out[j++] = out[i];
    }
    out.resize(j);
    for (int q : toClear) seenv[q >> 1] = 0;
    btlev = 0;
    if (out.size() > 1) {
        int mi = 1;
        for (size_t i = 2; i < out.size(); i++) if (lvl[out[i] >> 1] > lvl[out[mi] >> 1]) mi = i;
        swap(out[1], out[mi]); btlev = lvl[out[1] >> 1];
    }
}

static double luby(double y, int x) {
    int size, seq;
    for (size = 1, seq = 0; size < x + 1; seq++, size = 2 * size + 1);
    while (size - 1 != x) { size = (size - 1) >> 1; seq--; x = x % size; }
    return pow(y, seq);
}

static void reduceDB() {
    vector<int> lr;
    for (int i = 0; i < (int)cls.size(); i++) if (cls[i].learnt && cls[i].size > 2) lr.push_back(i);
    sort(lr.begin(), lr.end(), [](int a, int b) { return cls[a].act < cls[b].act; });
    vector<char> del(cls.size(), 0);
    for (size_t k = 0; k < lr.size() / 2; k++) {
        int cr = lr[k];
        int l0 = arena[cls[cr].start];
        int v = l0 >> 1;
        if (asg[v] >= 0 && reason[v] == cr) continue;  // locked
        del[cr] = 1;
    }
    for (auto& ws : watches) {
        size_t j = 0;
        for (size_t i = 0; i < ws.size(); i++) if (!del[ws[i]]) ws[j++] = ws[i];
        ws.resize(j);
    }
    for (size_t i = 0; i < cls.size(); i++) if (del[i]) cls[i].size = 0;  // tombstone (never watched)
}


// in-search subtour detection: a closed cycle that holds some but not all pearls
static bool hookOn = false;
static size_t checkedUpTo = 0;
static vector<int> hookClause, cyc, mark;
static int markStamp = 0;
static bool cycleCheck() {
    static vector<unsigned> visited; static unsigned visitStamp=0;
    if (visited.empty()) visited.assign(N,0);
    if (++visitStamp==0) { fill(visited.begin(),visited.end(),0); ++visitStamp; }
    if (checkedUpTo > trailv.size()) checkedUpTo = trailv.size();
    if (mark.empty()) mark.assign(N, 0);
    for (; checkedUpTo < trailv.size(); checkedUpTo++) {
        int l = trailv[checkedUpTo];
        if (l & 1) continue;
        int e = l >> 1; if (e >= E) continue;
        if (visited[eu[e]]==visitStamp || visited[ev[e]]==visitStamp) continue;
        visited[eu[e]]=visitStamp;
        int start = eu[e], prevE = e, cur = ev[e], np = reqP[start], len = 1;
        bool closed = false;
        cyc.clear(); cyc.push_back(start);
        while (true) {
            if (cur == start) { closed = true; break; }
            visited[cur]=visitStamp;
            np += reqP[cur]; cyc.push_back(cur);
            int nx = -1;
            for (int d = 0; d < 4; d++) { int f = cedge[cur * 4 + d]; if (f >= 0 && f != prevE && asg[f] == 1) { nx = f; break; } }
            if (nx < 0) break;
            cur = (eu[nx] == cur) ? ev[nx] : eu[nx]; prevE = nx;
            if (++len > N) break;
        }
        if (!closed || np == nReq || np == 0) continue;
        ++markStamp;
        for (int y : cyc) mark[y] = markStamp;
        hookClause.clear();
        bool allFalse = true;
        for (int y : cyc) for (int d = 0; d < 4; d++) {
            int f = cedge[y * 4 + d]; if (f < 0) continue;
            if (mark[nbr[y * 4 + d]] == markStamp) continue;
            hookClause.push_back(2 * f);
            if (asg[f] != 0) allFalse = false;
        }
        if (!allFalse || hookClause.empty()) continue;
        checkedUpTo++;
        return true;
    }
    return false;
}
static long long hookConfl = 0;
// returns 1 sat, 0 unsat, -1 timeout
static long long conflictsTotal = 0;
static long long conflLimit = (long long)4e18;
static int solve(double deadline) {
    long long conflStart = conflictsTotal;
    unsigned budgetTicks = 0;
    if (elapsed() >= deadline) return -1;
    if (unsatFlag) return 0;
    cancelUntil(0);
    if (propagate() >= 0) { unsatFlag = true; return 0; }
    vector<int> learnt;
    int curr = 0;
    long long nLearnts = 0, maxLearnts = 4000;
    while (true) {
        double budget = luby(2, curr++) * 100;
        long long confl = 0;
        while (true) {
            if (((++budgetTicks & 63) == 0 && elapsed() >= deadline) || conflictsTotal - conflStart > conflLimit) { cancelUntil(0); return -1; }
            int cr = propagate();
            if (cr >= 0) {
                conflictsTotal++; confl++;
                if (level() == 0) { unsatFlag = true; return 0; }
                int bt;
                analyze(cr, learnt, bt);
                cancelUntil(bt);
                if (learnt.size() == 1) enqueue(learnt[0], -1);
                else { int ncr = addClauseRaw(learnt, true); cls[ncr].act = clInc; enqueue(learnt[0], ncr); nLearnts++; }
                varInc *= 1 / 0.95; clInc *= 1 / 0.999;
                if (conflictsTotal - conflStart > conflLimit || ((conflictsTotal & 255) == 0 && elapsed() > deadline)) { cancelUntil(0); return -1; }
            } else {
                if (hookOn && cycleCheck()) {
                    hookConfl++; conflictsTotal++; confl++;
                    vector<int>& c = hookClause;
                    sort(c.begin(), c.end(), [](int a, int b) { return lvl[a >> 1] > lvl[b >> 1]; });
                    if (lvl[c[0] >> 1] == 0) { unsatFlag = true; return 0; }
                    if (c.size() == 1) { cancelUntil(0); enqueue(c[0], -1); continue; }
                    int l0 = lvl[c[0] >> 1], l1 = lvl[c[1] >> 1];
                    if (l0 > l1) { cancelUntil(l1); int cr2 = addClauseRaw(c, false); enqueue(c[0], cr2); continue; }
                    cancelUntil(l0);
                    int cr2 = addClauseRaw(c, false);
                    int bt;
                    analyze(cr2, learnt, bt);
                    cancelUntil(bt);
                    if (learnt.size() == 1) enqueue(learnt[0], -1);
                    else { int ncr = addClauseRaw(learnt, true); cls[ncr].act = clInc; enqueue(learnt[0], ncr); nLearnts++; }
                    varInc *= 1 / 0.95; clInc *= 1 / 0.999;
                    continue;
                }
                if (confl >= budget) { cancelUntil(0); break; }
                if (nLearnts > maxLearnts) { reduceDB(); nLearnts /= 2; maxLearnts = maxLearnts * 11 / 10; }
                int next = -1;
                while (!heap.empty()) { int v = hpop(); if (asg[v] < 0) { next = v; break; } }
                if (next < 0) return 1;
                trailLim.push_back(trailv.size());
                enqueue(2 * next + (phase[next] ? 0 : 1), -1);
            }
        }
    }
}
static inline int modelVal(int v) { return asg[v] == 1; }
}  // namespace cdcl

// Build the Masyu CNF over edge vars and solve with lazy subtour cuts.
static long long satIters = 0;
static int BR0 = 0, BR1 = 1 << 20, BC0 = 0, BC1 = 1 << 20;  // build box (cells)
static inline bool inBox(int x, int m) { int r = x / C, c = x % C; return r >= BR0 - m && r <= BR1 + m && c >= BC0 - m && c <= BC1 + m; }
static void buildBase(int nvars) {
    using namespace cdcl;
    init(nvars);
    hookOn = true; checkedUpTo = 0;
    auto P = [](int e) { return 2 * e; };
    auto Nn = [](int e) { return 2 * e + 1; };
    for (int x = 0; x < N; x++) {
        if (!inBox(x, 1)) continue;
        vector<int> S;
        for (int d = 0; d < 4; d++) if (cedge[x * 4 + d] >= 0) S.push_back(cedge[x * 4 + d]);
        int k = S.size();
        for (int i = 0; i < k; i++) {  // not exactly one
            vector<int> c = {Nn(S[i])};
            for (int j = 0; j < k; j++) if (j != i) c.push_back(P(S[j]));
            addClause(c);
        }
        for (int a = 0; a < k; a++) for (int b = a + 1; b < k; b++) for (int c = b + 1; c < k; c++)
            addClause({Nn(S[a]), Nn(S[b]), Nn(S[c])});
        if (bldP[x]) { vector<int> c; for (int e : S) c.push_back(P(e)); addClause(c); }
    }
    // pearl configs: at least one config holds (Tseitin-free via config literal OR):
    // encode with auxiliary? Use direct clauses instead.
    for (int pi = 0; pi < nPearls; pi++) {
        int p = pearls[pi];
        if (!inBox(p, 3) || !bldP[p]) continue;
        char t = G[p / C][p % C];
        if (t == 'B') {
            for (int ax = 0; ax < 2; ax++) {
                int d1 = ax, d2 = ax + 2;  // ax0: N/S, ax1: E/W
                int e1 = stepE(p, d1), e2 = stepE(p, d2);
                vector<int> c;
                if (e1 >= 0) c.push_back(P(e1));
                if (e2 >= 0) c.push_back(P(e2));
                addClause(c);
                if (e1 >= 0 && e2 >= 0) addClause({Nn(e1), Nn(e2)});
                for (int d : {d1, d2}) {
                    int e = stepE(p, d); if (e < 0) continue;
                    int q = stepC(p, d), b = stepE(q, d);
                    if (b < 0) { addClause({Nn(e)}); continue; }
                    addClause({Nn(e), P(b)});
                    int pa = stepE(q, (d + 1) % 4), pb = stepE(q, (d + 3) % 4);
                    if (pa >= 0) addClause({Nn(e), Nn(pa)});
                    if (pb >= 0) addClause({Nn(e), Nn(pb)});
                }
            }
        } else {
            int eN = stepE(p, 0), eE = stepE(p, 1), eS = stepE(p, 2), eW = stepE(p, 3);
            // straight: pairs equal; no mixing
            auto eqv = [&](int a, int b) {
                if (a < 0 && b < 0) return;
                if (a < 0) { addClause({Nn(b)}); return; }
                if (b < 0) { addClause({Nn(a)}); return; }
                addClause({Nn(a), P(b)}); addClause({Nn(b), P(a)});
            };
            eqv(eN, eS); eqv(eE, eW);
            for (int a : {eN, eS}) for (int b : {eE, eW}) if (a >= 0 && b >= 0) addClause({Nn(a), Nn(b)});
            for (int ax = 0; ax < 2; ax++) {
                int d1 = ax, d2 = ax + 2;
                int e1 = stepE(p, d1); if (e1 < 0) continue;
                int b1 = stepE(stepC(p, d1), d1), b2 = stepE(stepC(p, d2), d2);
                if (b1 < 0 || b2 < 0) continue;
                addClause({Nn(e1), Nn(b1), Nn(b2)});
            }
        }
    }
}
static vector<char> solveLoop(double deadline) {
    using namespace cdcl;
    auto P = [](int e) { return 2 * e; };
    vector<int> comp(N, -1);
    vector<char> on(E, 0);
    while (true) {
        int r = solve(deadline);
        if (r != 1) return {};
        for (int e = 0; e < E; e++) on[e] = modelVal(e);
        // find cycles
        fill(comp.begin(), comp.end(), -1);
        vector<vector<int>> cyc;  // cells per cycle
        for (int x = 0; x < N; x++) {
            if (comp[x] >= 0) continue;
            int dg = 0; for (int d = 0; d < 4; d++) { int e = cedge[x * 4 + d]; if (e >= 0 && on[e]) dg++; }
            if (!dg) continue;
            int id = cyc.size(); cyc.push_back({});
            vector<int> stck = {x}; comp[x] = id;
            while (!stck.empty()) {
                int y = stck.back(); stck.pop_back(); cyc[id].push_back(y);
                for (int d = 0; d < 4; d++) { int e = cedge[y * 4 + d]; if (e >= 0 && on[e]) { int z = nbr[y * 4 + d]; if (comp[z] < 0) { comp[z] = id; stck.push_back(z); } } }
            }
        }
        int full = -1;
        bool anyCut = false;
        for (int id = 0; id < (int)cyc.size(); id++) {
            int np = 0; for (int y : cyc[id]) np += reqP[y];
            if (np == nReq) { full = id; break; }
            if (np == 0) continue;
            // cut on cycle's cell set
            vector<int> c;
            for (int y : cyc[id]) for (int d = 0; d < 4; d++) {
                int e = cedge[y * 4 + d]; if (e < 0) continue;
                int z = nbr[y * 4 + d]; if (comp[z] != id) c.push_back(P(e));
            }
            addClause(c); anyCut = true;
            // cut on enclosed region (cells inside or on the cycle)
            {
                // ray casting by rows: inside if odd number of vertical cycle edges crossing to the left
                vector<char> ins(N, 0);
                for (int rr = 0; rr < R; rr++) {
                    int par = 0;
                    for (int cc = 0; cc < C; cc++) {
                        int y = rr * C + cc;
                        if (comp[y] == id) ins[y] = 1;
                        else if (par) ins[y] = 1;
                        // vertical edge going north from y in this cycle toggles parity
                        int e = cedge[y * 4 + 0];
                        if (comp[y] == id && e >= 0 && on[e] && comp[nbr[y * 4 + 0]] == id) par ^= 1;
                    }
                }
                int inP = 0; for (int y = 0; y < N; y++) if (ins[y] && reqP[y]) inP++;
                if (inP > 0 && inP < nReq) {
                    vector<int> c2;
                    for (int y = 0; y < N; y++) if (ins[y]) for (int d = 0; d < 4; d++) {
                        int e = cedge[y * 4 + d]; if (e < 0) continue;
                        if (!ins[nbr[y * 4 + d]]) c2.push_back(P(e));
                    }
                    if (!c2.empty()) addClause(c2);
                }
            }
        }
        if (full >= 0) {
            vector<char> res(E, 0);
            for (int e = 0; e < E; e++) if (on[e] && comp[eu[e]] == full) res[e] = 1;
            return res;
        }
        satIters++;
        if (!anyCut) return {};  // should not happen
        if (elapsed() > deadline) return {};
    }
}

static int satRandAct = 0;
static vector<char> satSolve(double deadline, const vector<char>* hint = nullptr) {
    allPearlsReq(); // A timed-out regional repair must never relax a full solve.
    BR0 = 0; BR1 = 1 << 20; BC0 = 0; BC1 = 1 << 20;
    buildBase(E);
    if (hint) for (int e = 0; e < E; e++) cdcl::phase[e] = (*hint)[e];
    if (satRandAct) {  // randomised variable order for hedged restarts
        for (int v = 0; v < E; v++) cdcl::act[v] = rnd01();
        cdcl::heap.clear(); fill(cdcl::hpos.begin(), cdcl::hpos.end(), -1);
        for (int v = 0; v < E; v++) cdcl::hinsert(v);
    }
    cdcl::conflLimit = (long long)4e18;
    return solveLoop(deadline);
}

// Sinz sequential counter: sum(xs) <= K, aux vars from 'base'
static void atMostK(const vector<int>& xs, int K, int base) {
    using namespace cdcl;
    int n = xs.size();
    if (K >= n) return;
    if (K == 0) { for (int x : xs) addClause({2 * x + 1}); return; }
    auto S = [&](int i, int j) { return base + i * K + j; };  // i in [0,n-1), j in [0,K)
    auto Pl = [](int v) { return 2 * v; };
    auto Nl = [](int v) { return 2 * v + 1; };
    addClause({Nl(xs[0]), Pl(S(0, 0))});
    for (int j = 1; j < K; j++) addClause({Nl(S(0, j))});
    for (int i = 1; i < n - 1; i++) {
        addClause({Nl(xs[i]), Pl(S(i, 0))});
        addClause({Nl(S(i - 1, 0)), Pl(S(i, 0))});
        for (int j = 1; j < K; j++) {
            addClause({Nl(xs[i]), Nl(S(i - 1, j - 1)), Pl(S(i, j))});
            addClause({Nl(S(i - 1, j)), Pl(S(i, j))});
        }
        addClause({Nl(xs[i]), Nl(S(i - 1, K - 1))});
    }
    addClause({Nl(xs[n - 1]), Nl(S(n - 2, K - 1))});
}

// one LNS step on an arbitrary set of free cells (edges touching a free cell are free)
static long long lnsTried = 0, lnsImproved = 0, lnsUnsat = 0;
static bool lnsCells(vector<char>& cur, const vector<char>& freeCell, int r0, int r1, int c0, int c1, long long climit, double deadline, bool useCard = true, bool plateau = false) {
    static vector<int> fr;
    static vector<char> isFree;
    fr.clear(); isFree.assign(E, 0);
    int curFree = 0;
    for (int e = 0; e < E; e++) {
        if (freeCell[eu[e]] || freeCell[ev[e]]) { isFree[e] = 1; fr.push_back(e); curFree += cur[e]; }
    }
    if (useCard && curFree < 2) return false;
    int K = plateau ? curFree : curFree - 2;
    int n = fr.size();
    BR0 = r0; BR1 = r1; BC0 = c0; BC1 = c1;
    buildBase(E);
    for (int e = 0; e < E; e++) if (!isFree[e]) cdcl::addClause({2 * e + (cur[e] ? 0 : 1)});
    cdcl::checkedUpTo = cdcl::trailv.size();
    (void)n;
    cdcl::cardOn = useCard; cdcl::cardK = K; cdcl::cardVars = fr;
    for (int e : fr) cdcl::cardIn[e] = 1;
    if (plateau) { vector<int> d; for (int e : fr) d.push_back(2 * e + (cur[e] ? 1 : 0)); cdcl::addClause(d); }
    for (int e = 0; e < E; e++) cdcl::phase[e] = cur[e];
    cdcl::conflLimit = climit;
    lnsTried++;
    vector<char> r = solveLoop(deadline);
    if (r.empty()) { if (cdcl::unsatFlag) lnsUnsat++; return false; }
    int Lr = 0, Lc = 0;
    for (int e = 0; e < E; e++) { Lr += r[e]; Lc += cur[e]; }
    if (!useCard || Lr < Lc) { cur = r; lnsImproved++; return true; }
    if (plateau && Lr == Lc) { cur = r; return false; }
    return false;
}
static bool lnsStep(vector<char>& cur, int r0, int r1, int c0, int c1, long long climit, double deadline, bool plateau = false) {
    static vector<char> fc;
    fc.assign(N, 0);
    for (int r = r0; r <= r1; r++) for (int c = c0; c <= c1; c++) fc[r * C + c] = 1;
    return lnsCells(cur, fc, r0, r1, c0, c1, climit, deadline, true, plateau);
}
// loop order of cells
static vector<int> loopOrder(const vector<char>& cur) {
    vector<int> ord;
    int s = -1;
    for (int e = 0; e < E; e++) if (cur[e]) { s = eu[e]; break; }
    if (s < 0) return ord;
    int prev = -1, x = s;
    do {
        ord.push_back(x);
        int nx = -1;
        for (int d = 0; d < 4; d++) { int f = cedge[x * 4 + d]; if (f >= 0 && cur[f] && nbr[x * 4 + d] != prev) { nx = nbr[x * 4 + d]; break; } }
        if (nx < 0) break;
        prev = x; x = nx;
        if ((int)ord.size() > N) break;
    } while (x != s);
    return ord;
}
static bool lnsStretch(vector<char>& cur, int len, int margin, long long climit, double deadline) {
    vector<int> ord = loopOrder(cur);
    int L = ord.size();
    if (L < 4) return false;
    len = min(len, L);
    int st = rndi(L);
    static vector<char> fc;
    fc.assign(N, 0);
    int r0 = R, r1 = -1, c0 = C, c1 = -1;
    for (int i = 0; i < len; i++) {
        int x = ord[(st + i) % L], xr = x / C, xc = x % C;
        for (int dr = -margin; dr <= margin; dr++) for (int dc = -margin; dc <= margin; dc++) {
            if (abs(dr) + abs(dc) > margin) continue;
            int rr = xr + dr, cc = xc + dc;
            if (rr < 0 || rr >= R || cc < 0 || cc >= C) continue;
            fc[rr * C + cc] = 1;
            r0 = min(r0, rr); r1 = max(r1, rr); c0 = min(c0, cc); c1 = max(c1, cc);
        }
    }
    return lnsCells(cur, fc, r0, r1, c0, c1, climit, deadline);
}

// Repair a single-loop state with pearl violations into a valid loop, window by window.
static bool pearlSat(const vector<char>& on, int pi) {
    for (auto& L : pat[pi].cfg) {
        bool ok = true;
        for (auto& l : L) if (on[l.e] != l.v) { ok = false; break; }
        if (ok) return true;
    }
    return false;
}
static long long repairCalls = 0;
static bool repairLoop(vector<char>& cur, double deadline) {
    vector<char> onLoop(N, 0);
    for (int iter = 0; iter < 4 * nPearls + 50; iter++) {
        if (elapsed() > deadline) { allPearlsReq(); return false; }
        fill(onLoop.begin(), onLoop.end(), 0);
        for (int e = 0; e < E; e++) if (cur[e]) { onLoop[eu[e]] = 1; onLoop[ev[e]] = 1; }
        vector<int> bad;
        for (int pi = 0; pi < nPearls; pi++) if (!pearlSat(cur, pi)) bad.push_back(pi);
        if (bad.empty()) { allPearlsReq(); return true; }
        int pi = bad[rndi(bad.size())];
        int p = pearls[pi], pr = p / C, pc = p % C;
        bool fixed = false;
        for (int rad = 3; !fixed; rad = rad * 3 / 2 + 1) {
            if (rad > 2 * max(R, C) || elapsed() >= deadline) break;
            int r0 = max(0, pr - rad), r1 = min(R - 1, pr + rad), c0 = max(0, pc - rad), c1 = min(C - 1, pc + rad);
            vector<char> fc(N, 0);
            for (int r = r0; r <= r1; r++) for (int c = c0; c <= c1; c++) fc[r * C + c] = 1;
            // required: satisfied on-loop pearls, plus every pearl in the window (+2 margin)
            reqP.assign(N, 0); bldP.assign(N, 0); nReq = 0;
            for (int pj = 0; pj < nPearls; pj++) {
                int q = pearls[pj], qr = q / C, qc = q % C;
                bool near = qr >= r0 - 2 && qr <= r1 + 2 && qc >= c0 - 2 && qc <= c1 + 2;
                bool inWin = qr >= r0 && qr <= r1 && qc >= c0 && qc <= c1;
                bool sat = pearlSat(cur, pj);
                if (inWin || (onLoop[q] && sat)) { bldP[q] = 1; }
                else if (near && !inWin) { bldP[q] = 0; }
                if (inWin || onLoop[q]) { reqP[q] = 1; nReq++; }
            }
            repairCalls++;
            if (lnsCells(cur, fc, r0, r1, c0, c1, 2000 + 20LL * rad * rad, deadline, false)) fixed = true;
            else if (getenv("DBG")) cerr << "repair fail pearl " << pr << "," << pc << " rad " << rad << " unsat=" << cdcl::unsatFlag << " nReq=" << nReq << "\n";
        }
        if (!fixed) { allPearlsReq(); return false; }
    }
    allPearlsReq();
    return false;
}

// ------------------------------------------------------------------ SA on face flips
namespace sa {
static int F, FC;  // faces (R-1)*(C-1)
static vector<char> on;
static vector<int> deg;
static vector<int> onList, onPos;
static vector<char> viol;
static int nViol, L;
static vector<vector<int>> facePearls;
static vector<int> fe;  // fe[f*4+k]: top,right,bottom,left
static vector<int> fc;  // corners: TL,TR,BR,BL
static vector<int> edgeFace;  // edgeFace[e*2+s]

static bool pearlOk(int pi) {
    for (auto& L : pat[pi].cfg) {
        bool ok = true;
        for (auto& l : L) if (on[l.e] != l.v) { ok = false; break; }
        if (ok) return true;
    }
    return false;
}

static void prep() {
    FC = C - 1; F = (R - 1) * (C - 1);
    fe.assign(F * 4, 0); fc.assign(F * 4, 0); edgeFace.assign(E * 2, -1);
    facePearls.assign(F, {});
    for (int i = 0; i + 1 < R; i++) for (int j = 0; j + 1 < C; j++) {
        int f = i * FC + j;
        int tl = i * C + j, tr = tl + 1, bl = tl + C, br = bl + 1;
        fc[f * 4 + 0] = tl; fc[f * 4 + 1] = tr; fc[f * 4 + 2] = br; fc[f * 4 + 3] = bl;
        fe[f * 4 + 0] = cedge[tl * 4 + 1];
        fe[f * 4 + 1] = cedge[tr * 4 + 2];
        fe[f * 4 + 2] = cedge[bl * 4 + 1];
        fe[f * 4 + 3] = cedge[tl * 4 + 2];
        for (int k = 0; k < 4; k++) {
            int e = fe[f * 4 + k];
            if (edgeFace[e * 2] < 0) edgeFace[e * 2] = f; else edgeFace[e * 2 + 1] = f;
            for (int pi : edgePearls[e]) {
                auto& v = facePearls[f];
                if (find(v.begin(), v.end(), pi) == v.end()) v.push_back(pi);
            }
        }
    }
}

static void setState(const vector<char>& s) {
    on = s; deg.assign(N, 0); onList.clear(); onPos.assign(E, -1); L = 0;
    for (int e = 0; e < E; e++) if (on[e]) { deg[eu[e]]++; deg[ev[e]]++; onPos[e] = onList.size(); onList.push_back(e); L++; }
    viol.assign(nPearls, 0); nViol = 0;
    for (int pi = 0; pi < nPearls; pi++) { viol[pi] = !pearlOk(pi); nViol += viol[pi]; }
}

static inline void toggle(int e) {
    if (on[e]) {
        on[e] = 0; deg[eu[e]]--; deg[ev[e]]--;
        int p = onPos[e], last = onList.back(); onList[p] = last; onPos[last] = p; onList.pop_back(); onPos[e] = -1; L--;
    } else {
        on[e] = 1; deg[eu[e]]++; deg[ev[e]]++;
        onPos[e] = onList.size(); onList.push_back(e); L++;
    }
}

// cells count = edges count for a cycle
static void run(vector<char>& best, int& bestL, double tEnd, double W, double Ts, double Te) {
    double tStart = elapsed();
    double temp = Ts;
    long long it = 0;
    while (true) {
        if ((it & 1023) == 0) {
            double now = elapsed();
            if (now >= tEnd) break;
            double fr = (now - tStart) / (tEnd - tStart);
            temp = Ts * pow(Te / Ts, fr);
        }
        it++;
        int e = onList[rndi(onList.size())];
        int f = edgeFace[e * 2 + (rnd() & 1)];
        if (f < 0) { f = edgeFace[e * 2]; }
        int m = 0;
        for (int k = 0; k < 4; k++) m |= on[fe[f * 4 + k]] << k;
        if (m == 15 || m == 5 || m == 10) continue;
        int k = __builtin_popcount(m);
        // corner q between edge q-1 and q (corner0 TL: left(3), top(0))
        bool ok = true;
        int pearlDrop = 0;
        for (int q = 0; q < 4; q++) {
            int a = (m >> ((q + 3) & 3)) & 1, b = (m >> q) & 1;
            int cell = fc[f * 4 + q];
            if (!a && !b && deg[cell] != 0) { ok = false; break; }
            if (a && b && isPearl[cell]) pearlDrop++;
        }
        if (!ok) continue;
        int dL = 4 - 2 * k;
        auto& fp = facePearls[f];
        int oldV = 0;
        for (int pi : fp) oldV += viol[pi];
        for (int q = 0; q < 4; q++) toggle(fe[f * 4 + q]);
        int newV = 0;
        static int tmp[64];
        int nfp = fp.size();
        for (int i = 0; i < nfp; i++) { tmp[i] = !pearlOk(fp[i]); newV += tmp[i]; }
        double delta = dL + W * (newV - oldV);
        if (delta <= 0 || rnd01() < exp(-delta / temp)) {
            for (int i = 0; i < nfp; i++) viol[fp[i]] = tmp[i];
            nViol += newV - oldV;
            if (nViol == 0 && L < bestL) { bestL = L; best = on; }
        } else {
            for (int q = 0; q < 4; q++) toggle(fe[f * 4 + q]);
        }
    }
}
}  // namespace sa

static vector<string> render(const vector<char>& on) {
    vector<string> out(R, string(C, '.'));
    for (int x = 0; x < N; x++) {
        int m = 0;
        for (int d = 0; d < 4; d++) { int e = cedge[x * 4 + d]; if (e >= 0 && on[e]) m |= 1 << d; }
        char ch = '.';
        switch (m) {
            case 1 | 4: ch = '|'; break;
            case 2 | 8: ch = '-'; break;
            case 1 | 2: ch = 'L'; break;
            case 1 | 8: ch = 'J'; break;
            case 4 | 2: ch = 'F'; break;
            case 4 | 8: ch = '7'; break;
        }
        out[x / C][x % C] = ch;
    }
    return out;
}

// Replace long pearl-free arcs by shortest paths through unused cells.
// Keep two edges at each pearl: its shape and both adjacent shapes stay fixed.
static void shortcutImprove(vector<char>& cur, double end) {
    if ((int)cur.size() != E || elapsed() >= end) return;
    vector<int> blocked(N), pred(N), dist(N), queue(N);
    while (elapsed() < end) {
        vector<int> path = loopOrder(cur);
        int length = (int)path.size();
        if (length < 4) return;
        vector<int> positions;
        for (int i = 0; i < length; ++i)
            if (isPearl[path[i]]) positions.push_back(i);
        if (positions.size() < 2) return;
        vector<pair<int, int>> gaps;
        for (int i = 0; i < (int)positions.size(); ++i) {
            int j = (i + 1) % positions.size();
            int gap = (positions[j] - positions[i] + length) % length;
            if (gap > 6) gaps.push_back({-gap, i});
        }
        sort(gaps.begin(), gaps.end());
        bool improved = false;
        for (auto entry : gaps) {
            if (elapsed() >= end) return;
            int begin = positions[entry.second] + 2;
            int finish = positions[entry.second] - entry.first - 2;
            int a = path[begin % length], b = path[finish % length];
            int old = finish - begin;
            if (abs(a / C - b / C) + abs(a % C - b % C) >= old) continue;
            fill(blocked.begin(), blocked.end(), 0);
            for (int x : path) blocked[x] = 1;
            for (int k = begin; k <= finish; ++k) blocked[path[k % length]] = 0;
            fill(dist.begin(), dist.end(), -1);
            int head = 0, tail = 0;
            queue[tail++] = a; dist[a] = 0; pred[a] = -1;
            while (head < tail && dist[b] < 0) {
                if ((head & 255) == 0 && elapsed() >= end) return;
                int x = queue[head++];
                if (dist[x] + 1 >= old) continue;
                for (int d = 0; d < 4; ++d) {
                    int y = nbr[x * 4 + d];
                    if (y < 0 || blocked[y] || dist[y] >= 0) continue;
                    // A shortest improving path must still reach b before old.
                    int dy = dist[x] + 1;
                    if (dy + abs(y / C - b / C) + abs(y % C - b % C) >= old) continue;
                    pred[y] = x; dist[y] = dy; queue[tail++] = y;
                }
            }
            if (dist[b] < 0 || dist[b] >= old) continue;
            vector<char> candidate = cur;
            auto toggle = [&](int x, int y) {
                for (int d = 0; d < 4; ++d) if (nbr[x * 4 + d] == y) {
                    candidate[cedge[x * 4 + d]] ^= 1;
                    return;
                }
            };
            for (int k = begin; k < finish; ++k)
                toggle(path[k % length], path[(k + 1) % length]);
            for (int x = b; x != a; x = pred[x]) toggle(x, pred[x]);
            // The replacement is a simple BFS path disjoint from the retained
            // arc, so it preserves the single loop. Check all local rules too.
            bool okay = true;
            for (int pi = 0; pi < nPearls && okay; ++pi) okay = pearlSat(candidate, pi);
            if (!okay) continue;
            vector<int> degree(N, 0);
            int count = 0;
            for (int e = 0; e < E; ++e) if (candidate[e]) {
                ++degree[eu[e]]; ++degree[ev[e]]; ++count;
            }
            for (int x = 0; x < N && okay; ++x)
                if ((degree[x] != 0 && degree[x] != 2) || (isPearl[x] && degree[x] != 2)) okay = false;
            if (okay && count < length && (int)loopOrder(candidate).size() == count) {
                cur.swap(candidate); improved = true; break;
            }
        }
        if (!improved) return;
    }
}

// Strict-valid version of face annealing. Include after namespace sa: it shares
// immutable face indexing, but keeps its own state. No pearl violations are
// accepted, and nonbranching face replacements preserve a single simple loop.
static void hardPolish(vector<char>& cur, double end, double temperature, bool walk) {
    if ((int)cur.size() != E || elapsed() >= end) return;
    vector<unsigned char> masks(N, 0);
    vector<char> on = cur, saved = cur;
    vector<int> active, position(E, -1);
    active.reserve(E);
    for (int e = 0; e < E; ++e) if (on[e]) {
        position[e] = active.size(); active.push_back(e);
        int d = e < HN ? 1 : 2;
        masks[eu[e]] |= 1 << d;
        masks[ev[e]] |= 1 << (d ^ 2);
    }
    if (active.size() < 4) return;
    int length = active.size(), bestLength = length;
    static const unsigned char legal[16] = {1,0,0,1,0,1,1,0,0,1,1,0,1,0,0,0};
    static const int toggles[4] = {6,12,9,3};
    const double begin = elapsed();
    double uphill = temperature > 0 ? exp(-2.0 / temperature) : 0;
    for (unsigned long long iter = 0;; ++iter) {
        if ((iter & 1023) == 0) {
            double now = elapsed();
            if (now >= end) break;
            double fraction = (now - begin) / max(1e-9, end - begin);
            double t = temperature > 0 ? temperature * pow(0.15 / temperature, fraction) : 0;
            uphill = t > 0 ? exp(-2.0 / t) : 0;
        }
        int edge = active[rndi(active.size())];
        int f = sa::edgeFace[2 * edge + (rnd() & 1)];
        if (f < 0) f = sa::edgeFace[2 * edge];
        int m = 0;
        for (int k = 0; k < 4; ++k) m |= on[sa::fe[4 * f + k]] << k;
        // Opposite edges could split the loop; empty/full faces create/delete it.
        if (m == 0 || m == 15 || m == 5 || m == 10) continue;
        int delta = 4 - 2 * __builtin_popcount((unsigned)m);
        if (delta > 0 && (length + delta > bestLength + 6 || rnd01() >= uphill)) continue;
        bool okay = true;
        for (int k = 0; k < 4; ++k)
            if (!legal[masks[sa::fc[4 * f + k]] ^ toggles[k]]) { okay = false; break; }
        if (!okay) continue;
        for (int k = 0; k < 4; ++k) masks[sa::fc[4 * f + k]] ^= toggles[k];
        for (int pi : sa::facePearls[f]) {
            int x = pearls[pi], mask = masks[x];
            bool straight = mask == 5 || mask == 10;
            if (!mask || straight != (G[x / C][x % C] == 'W')) { okay = false; break; }
            int turns = 0;
            for (int d = 0; d < 4; ++d) if (mask & (1 << d)) {
                int y = nbr[4 * x + d];
                if (y < 0) { okay = false; break; }
                int other = masks[y];
                turns += other != 5 && other != 10;
            }
            if (!okay || (straight ? turns == 0 : turns != 0)) { okay = false; break; }
        }
        if (!okay) {
            for (int k = 0; k < 4; ++k) masks[sa::fc[4 * f + k]] ^= toggles[k];
            continue;
        }
        for (int k = 0; k < 4; ++k) {
            int e = sa::fe[4 * f + k];
            if (on[e]) {
                int p = position[e], last = active.back();
                active[p] = last; position[last] = p;
                active.pop_back(); position[e] = -1; on[e] = 0;
            } else {
                on[e] = 1; position[e] = active.size(); active.push_back(e);
            }
        }
        length += delta;
        if (length < bestLength) { bestLength = length; saved = on; }
    }
    cur = walk && length == bestLength ? on : saved;
}

// Embedded Astra implementation; standard headers must remain outside namespace.
#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <numeric>
#include <queue>
#include <functional>
#include <string>
#include <utility>
#include <vector>
namespace astra_portfolio {
using namespace std;
// Two-watched-literal CDCL engine; all learned clauses remain local to this solve.
class CDCL {
    struct Clause { vector<int> l; };
    function<vector<int>()> theory; int ticks=0;
    int n; vector<Clause> cs; vector<vector<int>> watch;
    vector<int> val,lev,reason,trail,limits; vector<unsigned char> phase,seen;
    vector<double> act; priority_queue<pair<double,int>> heap;
    int head=0; double inc=1; bool okay=true,ready=false;
    chrono::steady_clock::time_point origin;
    double deadline;
    int value(int p)const{return val[p>>1]<0?-1:(val[p>>1]^(p&1));}
    void put(int p,int why){int x=p>>1;val[x]=1-(p&1);phase[x]=val[x];lev[x]=limits.size();reason[x]=why;trail.push_back(p);}
    void bump(int x){act[x]+=inc;heap.emplace(act[x],x);}
    int store(vector<int> a){int id=cs.size();cs.push_back({move(a)});if(cs[id].l.size()>1){watch[cs[id].l[0]].push_back(id);watch[cs[id].l[1]].push_back(id);}return id;}
    int propagate(){
        while(head<(int)trail.size()){
            int bad=trail[head++]^1;auto& list=watch[bad];size_t out=0;
            for(size_t i=0;i<list.size();i++){
                int id=list[i];auto& a=cs[id].l;
                if(a[0]==bad)swap(a[0],a[1]);
                if(value(a[0])==1){list[out++]=id;continue;}
                size_t j=2;while(j<a.size()&&value(a[j])==0)++j;
                if(j<a.size()){swap(a[1],a[j]);watch[a[1]].push_back(id);continue;}
                list[out++]=id;
                if(value(a[0])==0){while(++i<list.size())list[out++]=list[i];list.resize(out);return id;}
                put(a[0],id);
            }
            list.resize(out);
        }
        return -1;
    }
    void cancel(int level){
        if((int)limits.size()<=level)return;
        int end=limits[level];for(int i=trail.size()-1;i>=end;i--){int x=trail[i]>>1;val[x]=-1;heap.emplace(act[x],x);}
        trail.resize(end);head=min(head,end);limits.resize(level);
    }
    pair<vector<int>,int> analyze(int conflict){
        vector<int> result(1,-1),touched;int paths=0,p=-1,index=trail.size()-1;
        do {
            for(int q:cs[conflict].l){int x=q>>1;if(x==(p>>1)||seen[x]||lev[x]==0)continue;
                seen[x]=1;touched.push_back(x);bump(x);
                if(lev[x]==(int)limits.size())++paths;else result.push_back(q);
            }
            while(!seen[trail[index]>>1])--index;
            p=trail[index--];seen[p>>1]=0;--paths;conflict=reason[p>>1];
        }while(paths>0);
        result[0]=p^1;int back=0,pos=1;
        for(int i=1;i<(int)result.size();i++)if(lev[result[i]>>1]>back){back=lev[result[i]>>1];pos=i;}
        if(result.size()>1)swap(result[1],result[pos]);
        for(int x:touched)seen[x]=0;
        return {move(result),back};
    }
public:
    CDCL(int count,chrono::steady_clock::time_point t,double dl):n(count),watch(2*n),val(n,-1),lev(n),reason(n,-1),phase(n,0),seen(n),act(n,0),origin(t),deadline(dl){}
    void add(vector<int> a){
        if(!okay)return;cancel(0);sort(a.begin(),a.end());vector<int>b;
        for(int p:a){if(value(p)==1)return;if(value(p)==0)continue;if(!b.empty()&&b.back()==p)continue;if(!b.empty()&&b.back()==(p^1))return;b.push_back(p);}
        if(b.empty()){okay=false;return;}
        for(int p:b){act[p>>1]+=0.001;if(ready)heap.emplace(act[p>>1],p>>1);}
        int id=store(move(b));if(cs[id].l.size()==1)put(cs[id].l[0],id);
    }
    void initialize(){ready=true;for(int x=0;x<n;x++)heap.emplace(act[x],x);}
    int solve(){
        if(!okay)return 0;
        int conflicts=0,nextRestart=128; unsigned guard=0;
        for(;;){
            if((++guard&127)==0&&chrono::duration<double>(chrono::steady_clock::now()-origin).count()>=deadline)return -1;
            if(heap.size()>size_t(32*n)){
                priority_queue<pair<double,int>> fresh;
                for(int x=0;x<n;x++)if(val[x]<0)fresh.emplace(act[x],x);
                heap.swap(fresh);
            }
            int c=propagate();
            if(c<0&&theory&&((++ticks&7)==0)){
                auto cut=theory();if(!cut.empty())c=store(move(cut));
            }
            if(c>=0){
                int top=0;for(int p:cs[c].l)top=max(top,lev[p>>1]);cancel(top);
                if(limits.empty()){okay=false;return 0;}
                ++conflicts;auto learned=analyze(c);cancel(learned.second);
                int id=store(move(learned.first));put(cs[id].l[0],id);inc*=1.0526315789473684;
                if(inc>1e90){inc*=1e-90;for(double& a:act)a*=1e-90;heap={};for(int x=0;x<n;x++)if(val[x]<0)heap.emplace(act[x],x);}
                if(conflicts>=nextRestart){cancel(0);nextRestart+=128+(nextRestart/3);}
            }else{
                int x=-1;
                while(!heap.empty()){auto [a,v]=heap.top();heap.pop();if(val[v]<0&&a==act[v]){x=v;break;}}
                if(x<0)return 1;
                limits.push_back(trail.size());put(2*x+(phase[x]?0:1),-1);
            }
        }
    }
    void setTheory(function<vector<int>()> f){theory=move(f);}
    bool selected(int x)const{return val[x]==1;}
    void rootLevel(){cancel(0);}
};

class Solver {
public:
    using Clock=chrono::steady_clock;
    Clock::time_point start=Clock::now();
    static constexpr int shape[7]={0,10,5,3,9,6,12}; // N,E,S,W
    static constexpr char glyph[8]=".-|LJF7";
    int R,C,N,P=0,bestLen=100000,lower=4;
    vector<string> grid;
    vector<array<int,4>> nb;
    vector<int> pearl,dom,initial,best,deg,mask,parent,sz,req,seen,activity;
    vector<int> queue;
    vector<unsigned char> queued;
    vector<pair<int,int>> trail;
    int edgeSupport[4][128]{},on[4]{},off[4]{},forced[128]{},possible[128]{};
    int forcedCount=0,colourCount[2]{0,0};
    int serial=0,mode=0,limit=100000,nodeLimit=1000000;
    long long nodes=0,totalNodes=0;
    double deadline=0.85;
    bool stopped=false,oneOnly=true;
    unsigned long long rng=0x91a0b16c53ULL;
    unsigned random(){rng^=rng<<7;rng^=rng>>9;return unsigned(rng);}
    double elapsed()const{return chrono::duration<double>(Clock::now()-start).count();}
    bool expired(){if(elapsed()>=deadline){stopped=true;return true;}return false;}
    int colour(int x)const{return ((x/C)+(x%C))&1;}
    void push(int x){if(x>=0&&!queued[x]){queued[x]=1;queue.push_back(x);}}
    bool narrow(int x,int value){
        value &= dom[x];
        if(value==dom[x])return true;
        if(!value){activity[x]+=8;return false;}
        trail.emplace_back(x,dom[x]);
        if((dom[x]&1)&&!(value&1)){++forcedCount;++colourCount[colour(x)];}
        dom[x]=value;push(x);for(int y:nb[x])push(y);
        return true;
    }
    void undo(size_t mark){
        for(int x:queue)queued[x]=0;queue.clear();
        while(trail.size()>mark){auto [x,d]=trail.back();trail.pop_back();
            if((d&1)&&!(dom[x]&1)){--forcedCount;--colourCount[colour(x)];}
            dom[x]=d;
        }
    }
    bool propagate(){
        int ticks=0;
        while(!queue.empty()){
            if((++ticks&1023)==0&&expired())return false;
            int x=queue.back();queue.pop_back();queued[x]=0;
            for(int d=0;d<4;d++){
                int y=nb[x][d];if(y<0)continue;
                int allowed=edgeSupport[d][dom[y]];
                // Black-neighbour rules are binary shape constraints.
                if(pearl[x]==2){
                    int straight=(d&1)?2:4;
                    if(!(dom[y]&straight))allowed&=off[d];
                }
                if(pearl[y]==2){
                    int f=forced[dom[y]];
                    if(f&(1<<(d^2)))allowed&=(d&1)?2:4;
                    // If x cannot be straight along this axis, black y cannot connect.
                    int straight=(d&1)?2:4;
                    if(!(dom[x]&straight)&&!narrow(y,off[d^2]))return false;
                }
                if(!narrow(x,allowed))return false;
            }
            if(pearl[x]==1){
                int viable=0;
                for(int s=1;s<=2;s++)if(dom[x]&(1<<s)){
                    int d=s==1?1:0,a=nb[x][d],b=nb[x][d^2];
                    if(a>=0&&b>=0&&((dom[a]&120)||(dom[b]&120)))viable|=1<<s;
                }
                if(!narrow(x,viable))return false;
                if((dom[x]&(dom[x]-1))==0){
                    int d=dom[x]==2?1:0,a=nb[x][d],b=nb[x][d^2];
                    if(!(dom[a]&120)&&!narrow(b,120))return false;
                    if(!(dom[b]&120)&&!narrow(a,120))return false;
                }
            }
        }
        return true;
    }
    int root(int x){while(parent[x]!=x){parent[x]=parent[parent[x]];x=parent[x];}return x;}
    bool validate(const vector<int>& answer)const{
        int first=-1,count=0;
        for(int x=0;x<N;x++){
            int s=answer[x];if(s<0||s>6)return false;
            if(!s){if(pearl[x])return false;continue;}
            if(first<0)first=x;++count;
            int m=shape[s];
            if(pearl[x]==1&&s>2)return false;
            if(pearl[x]==2&&s<=2)return false;
            int turns=0;
            for(int d=0;d<4;d++)if(m&(1<<d)){
                int y=nb[x][d];if(y<0||!(shape[answer[y]]&(1<<(d^2))))return false;
                turns+=answer[y]>2;
            }
            if(pearl[x]==1&&!turns)return false;
            if(pearl[x]==2&&turns)return false;
        }
        if(first<0)return false;
        int prev=-1,cur=first,walk=0;
        do{int next=-1;for(int d=0;d<4;d++)if(shape[answer[cur]]&(1<<d)){
                int y=nb[cur][d];if(y!=prev){next=y;break;}}
            prev=cur;cur=next;if(cur<0||++walk>count)return false;
        }while(cur!=first);
        return walk==count;
    }
    // Returns -1 contradiction, 0 unfinished, 1 complete valid cycle.
    int connectivity(bool reach){
        iota(parent.begin(),parent.end(),0);fill(sz.begin(),sz.end(),1);
        fill(deg.begin(),deg.end(),0);fill(req.begin(),req.end(),0);
        int cycle=-1;
        for(int x=0;x<N;x++){
            mask[x]=forced[dom[x]];
            for(int d=1;d<=2;d++)if(mask[x]&(1<<d)){
                int y=nb[x][d];if(y<0)return -1;
                ++deg[x];++deg[y];int a=root(x),b=root(y);
                if(a==b){if(cycle>=0)return -1;cycle=x;}
                else {if(sz[a]<sz[b])swap(a,b);parent[b]=a;sz[a]+=sz[b];}
            }
        }
        for(int x=0;x<N;x++)if(!(dom[x]&1))++req[root(x)];
        if(cycle>=0){
            int r=root(cycle);if(req[r]!=forcedCount)return -1;
            vector<int> a(N,0);int len=0;
            for(int x=0;x<N;x++)if(root(x)==r){
                if(__builtin_popcount((unsigned)dom[x])!=1)return -1;
                a[x]=__builtin_ctz((unsigned)dom[x]);++len;
            }
            if(len>=limit||!validate(a))return -1;
            if(len<bestLen){bestLen=len;best=move(a);}
            return 1;
        }
        // A premature closure edge is forbidden even before choosing its shape.
        for(int x=0;x<N;x++)if(deg[x]==1){
            int r=root(x);
            if(req[r]==forcedCount)continue;
            for(int d=0;d<4;d++){
                int y=nb[x][d];
                if(y>x&&deg[y]==1&&root(y)==r&&!(mask[x]&(1<<d))){
                    if(!narrow(x,off[d])||!narrow(y,off[d^2]))return -1;
                }
            }
        }
        if(reach){
            ++serial;vector<int> q;int s=-1;
            for(int x=0;x<N;x++)if(pearl[x]){s=x;break;}
            q.push_back(s);seen[s]=serial;int count=0;
            for(size_t i=0;i<q.size();i++){
                int x=q[i];count+=!(dom[x]&1);
                for(int d=0;d<4;d++)if(possible[dom[x]]&(1<<d)){
                    int y=nb[x][d];if(y>=0&&seen[y]!=serial&&(possible[dom[y]]&(1<<(d^2)))){
                        seen[y]=serial;q.push_back(y);
                    }
                }
            }
            if(count!=forcedCount)return -1;
        }
        return 0;
    }
    bool search(int depth=0){
        if(stopped)return false;
        ++nodes;++totalNodes;
        if(nodes>nodeLimit){stopped=true;return false;}
        if((nodes&63)==0&&expired())return false;
        if(max(forcedCount,2*max(colourCount[0],colourCount[1]))>=limit)return false;
        if(!propagate())return false;
        if(max(forcedCount,2*max(colourCount[0],colourCount[1]))>=limit)return false;
        int status=connectivity(depth<4||(nodes&15)==0);
        if(status<0)return false;
        if(status>0)return true;
        if(!queue.empty()){
            if(!propagate())return false;
            status=connectivity(false);if(status<0)return false;if(status>0)return true;
        }
        int selected=-1;long long high=-(1LL<<60);
        for(int x=0;x<N;x++){
            int k=__builtin_popcount((unsigned)dom[x]);if(k<=1)continue;
            if((dom[x]&1)&&!pearl[x])continue;
            long long score=10000/k+30*min(activity[x],100)+ (pearl[x]?700:0)+(deg[x]==1?1200:0);
            if(mode)score+=random()%600;
            if(score>high){high=score;selected=x;}
        }
        if(selected<0){
            for(int x=0;x<N;x++)if(dom[x]&(dom[x]-1)){selected=x;break;}
        }
        if(selected<0)return false;
        int x=selected;
        vector<pair<int,int>> options;
        for(int s=0;s<7;s++)if(dom[x]&(1<<s)){
            int cost=0;
            for(int d=0;d<4;d++)if(shape[s]&(1<<d)){
                int y=nb[x][d];cost+=(dom[y]&1)?15:0;
                cost-=deg[y]==1?8:0;
                if(best.size()==(size_t)N&&best[x]==s)cost-=5;
            }
            if(mode)cost+=int(random()%20);
            options.emplace_back(cost,s);
        }
        sort(options.begin(),options.end());
        bool found=false;
        for(auto [score,s]:options){
            size_t m=trail.size();
            if(narrow(x,1<<s)&&search(depth+1)){found=true;if(oneOnly){undo(m);return true;}limit=bestLen;}
            undo(m);if(stopped)break;
        }
        if(!found)++activity[x];
        return found;
    }
    void reset(const vector<int>& base){
        dom=base;trail.clear();queue.clear();fill(queued.begin(),queued.end(),0);
        forcedCount=0;colourCount[0]=colourCount[1]=0;
        for(int x=0;x<N;x++){if(!(dom[x]&1)){++forcedCount;++colourCount[colour(x)];}push(x);}
        stopped=false;nodes=0;
    }
    bool satSearch(double end,int margin,bool useTheory,int variant=0,bool boundaryCuts=false){
        vector<array<int,4>> edge(N);for(auto& a:edge)a.fill(-1);
        vector<pair<int,int>> ends;
        for(int x=0;x<N;x++)for(int d=1;d<=2;d++)if(nb[x][d]>=0){int y=nb[x][d],e=ends.size();ends.emplace_back(x,y);edge[x][d]=edge[y][d^2]=e;}
        int E=ends.size();
        if(variant){
            for(int i=E-1;i>0;i--)swap(ends[i],ends[random()%(i+1)]);
            for(int e=0;e<E;e++){auto [x,y]=ends[e];int d=y==x+1?1:2;edge[x][d]=edge[y][d^2]=e;}
        }
        CDCL sat(E+1,start,end);
        sat.add({2*E+1});
        auto lit=[&](int x,int d){return 2*((x>=0&&edge[x][d]>=0)?edge[x][d]:E);};
        for(int x=0;x<N;x++){
            int a[4];for(int d=0;d<4;d++)a[d]=lit(x,d);
            for(int d=0;d<4;d++){vector<int> c={a[d]^1};for(int k=0;k<4;k++)if(k!=d)c.push_back(a[k]);sat.add(c);}
            for(int skip=0;skip<4;skip++){vector<int> c;for(int d=0;d<4;d++)if(d!=skip)c.push_back(a[d]^1);sat.add(c);}
            if(pearl[x]==1){
                sat.add({a[0]^1,a[2]});sat.add({a[2]^1,a[0]});
                sat.add({a[1]^1,a[3]});sat.add({a[3]^1,a[1]});sat.add({a[0],a[1]});
                for(int d=0;d<2;d++)sat.add({a[d]^1,lit(nb[x][d],d)^1,lit(nb[x][d^2],d^2)^1});
            }else if(pearl[x]==2){
                sat.add({a[0],a[2]});sat.add({a[0]^1,a[2]^1});sat.add({a[1],a[3]});sat.add({a[1]^1,a[3]^1});
                for(int d=0;d<4;d++)sat.add({a[d]^1,lit(nb[x][d],d)});
            }
        }
        if(margin>=0){
            int r0=R,c0=C,r1=0,c1=0;for(int x=0;x<N;x++)if(pearl[x]){r0=min(r0,x/C);r1=max(r1,x/C);c0=min(c0,x%C);c1=max(c1,x%C);}
            r0-=margin;r1+=margin;c0-=margin;c1+=margin;
            for(int e=0;e<E;e++){auto [x,y]=ends[e];if(x/C<r0||x/C>r1||x%C<c0||x%C>c1||y/C<r0||y/C>r1||y%C<c0||y%C>c1)sat.add({2*e+1});}
        }
        vector<int> uf(N),usize(N),pcnt(N);
        if(useTheory)sat.setTheory([&](){
            iota(uf.begin(),uf.end(),0);fill(usize.begin(),usize.end(),1);fill(pcnt.begin(),pcnt.end(),0);
            auto find=[&](int x){while(uf[x]!=x){uf[x]=uf[uf[x]];x=uf[x];}return x;};
            vector<int> cycles;
            for(int e=0;e<E;e++)if(sat.selected(e)){
                auto [x,y]=ends[e];int a=find(x),b=find(y);
                if(a==b)cycles.push_back(a);
                else {if(usize[a]<usize[b])swap(a,b);uf[b]=a;usize[a]+=usize[b];}
            }
            if(cycles.empty())return vector<int>{};
            for(int x=0;x<N;x++)if(pearl[x])++pcnt[find(x)];
            for(int v:cycles){int root=find(v);if(pcnt[root]==P)continue;
                vector<int> cut;
                if(boundaryCuts&&pcnt[root]>0){
                    for(int e=0;e<E;e++)if((find(ends[e].first)==root)!=(find(ends[e].second)==root))cut.push_back(2*e);
                }else{
                    for(int e=0;e<E;e++)if(sat.selected(e)&&find(ends[e].first)==root)cut.push_back(2*e+1);
                }
                return cut;
            }
            return vector<int>{};
        });
        sat.initialize();
        while(elapsed()<end){
            int status=sat.solve();if(status!=1)return false;
            vector<int> m(N),component(N,-1);vector<vector<int>> groups;vector<int> pc;
            for(int e=0;e<E;e++)if(sat.selected(e)){
                auto [x,y]=ends[e];int d=y==x+1?1:2;m[x]|=1<<d;m[y]|=1<<(d^2);
            }
            for(int x=0;x<N;x++)if(m[x]&&component[x]<0){
                int id=groups.size(),npearl=0;vector<int> q={x};component[x]=id;
                for(size_t k=0;k<q.size();k++){
                    int v=q[k];npearl+=pearl[v]!=0;
                    for(int d=0;d<4;d++)if(m[v]&(1<<d)){int y=nb[v][d];if(component[y]<0){component[y]=id;q.push_back(y);}}
                }
                groups.push_back(move(q));pc.push_back(npearl);
            }
            for(int id=0;id<(int)groups.size();id++)if(pc[id]==P){
                vector<int> candidate(N,0);for(int x:groups[id])for(int s=1;s<7;s++)if(shape[s]==m[x])candidate[x]=s;
                if(validate(candidate)&&int(groups[id].size())<bestLen){bestLen=groups[id].size();best=move(candidate);return true;}
            }
            sat.rootLevel();
            for(int id=0;id<(int)groups.size();id++){
                vector<int> cut;
                for(int x:groups[id])for(int d=1;d<=2;d++)if(m[x]&(1<<d))cut.push_back(2*edge[x][d]+1);
                sat.add(move(cut));
            }
        }
        return false;
    }
    void shortcutImprove(double end){
        int index[16];fill(index,index+16,-1);for(int s=0;s<7;s++)index[shape[s]]=s;
        while(elapsed()<end){
            vector<int> path;int first=-1;for(int x=0;x<N;x++)if(best[x]){first=x;break;}
            int prev=-1,cur=first;
            do{path.push_back(cur);int next=-1;for(int d=0;d<4;d++)if(shape[best[cur]]&(1<<d)){
                    int y=nb[cur][d];if(y!=prev){next=y;break;}}
                prev=cur;cur=next;
            }while(cur!=first);
            int L=path.size();vector<int> positions;for(int i=0;i<L;i++)if(pearl[path[i]])positions.push_back(i);
            vector<pair<int,int>> gaps;for(int i=0;i<(int)positions.size();i++){
                int j=(i+1)%positions.size(),distance=(positions[j]-positions[i]+L)%L;
                if(distance>6)gaps.emplace_back(-distance,i);
            }
            sort(gaps.begin(),gaps.end());bool improved=false;
            for(auto entry:gaps){
                if(elapsed()>=end)break;
                int i=entry.second,begin=positions[i]+2,finish=positions[i]-entry.first-2;
                int a=path[begin%L],b=path[finish%L],old=finish-begin;
                if(abs(a/C-b/C)+abs(a%C-b%C)>=old)continue;
                vector<unsigned char> blocked(N);for(int x=0;x<N;x++)blocked[x]=best[x]!=0;
                for(int k=begin;k<=finish;k++)blocked[path[k%L]]=0;
                vector<int> pred(N,-1),distance(N,-1),q={a};distance[a]=0;
                for(size_t h=0;h<q.size()&&distance[b]<0;h++){
                    int x=q[h];if(distance[x]+1>=old)continue;
                    for(int y:nb[x])if(y>=0&&!blocked[y]&&distance[y]<0){distance[y]=distance[x]+1;pred[y]=x;q.push_back(y);}
                }
                if(distance[b]<0||distance[b]>=old)continue;
                vector<int> m(N);for(int x=0;x<N;x++)m[x]=shape[best[x]];
                auto toggle=[&](int x,int y){int d=y==x-C?0:y==x+1?1:y==x+C?2:3;m[x]^=1<<d;m[y]^=1<<(d^2);};
                for(int k=begin;k<finish;k++)toggle(path[k%L],path[(k+1)%L]);
                for(int x=b;x!=a;x=pred[x])toggle(x,pred[x]);
                vector<int> candidate(N);bool okay=true;for(int x=0;x<N;x++){candidate[x]=index[m[x]];if(candidate[x]<0)okay=false;}
                if(okay&&validate(candidate)){
                    bestLen+=distance[b]-old;best=move(candidate);improved=true;break;
                }
            }
            if(!improved)break;
        }
    }

    void faceImprove(double end){
        vector<int> current(N);for(int x=0;x<N;x++)current[x]=shape[best[x]];
        int index[16];fill(index,index+16,-1);for(int s=0;s<7;s++)index[shape[s]]=s;
        int length=bestLen,stale=0;
        auto legalPearl=[&](int x){
            if(!pearl[x])return true;int m=current[x];if(!m)return false;
            bool straight=m==5||m==10;if((pearl[x]==1)!=straight)return false;
            int turns=0;for(int d=0;d<4;d++)if(m&(1<<d)){
                int y=nb[x][d];if(y<0)return false;int n=current[y];turns+=n!=5&&n!=10;
            }
            return pearl[x]==1?turns>0:turns==0;
        };
        for(int iter=0;bestLen>lower;iter++){
            if((iter&1023)==0&&(elapsed()>=end||stale>20000+20*N))break;
            ++stale;
            int r=random()%(R-1),c=random()%(C-1),a=r*C+c;
            int cells[4]={a,a+1,a+C+1,a+C};
            int m=0;m|=(current[a]&2)?1:0;m|=(current[a+1]&4)?2:0;
            m|=(current[a+C]&2)?4:0;m|=(current[a]&4)?8:0;
            if(!m||m==15||m==5||m==10)continue;
            int count=__builtin_popcount((unsigned)m),delta=4-2*count;
            if(delta>0&&(random()%1000)>=2)continue;
            int toggles[4]={6,12,9,3},old[4];bool okay=true;
            for(int k=0;k<4;k++){
                old[k]=current[cells[k]];
                int next=old[k]^toggles[k];if(index[next]<0){okay=false;break;}
            }
            if(!okay)continue;
            for(int k=0;k<4;k++)current[cells[k]]^=toggles[k];
            for(int x:cells){
                if(!legalPearl(x))okay=false;
                for(int y:nb[x])if(y>=0&&!legalPearl(y))okay=false;
            }
            if(!okay){for(int k=0;k<4;k++)current[cells[k]]=old[k];continue;}
            length+=delta;
            if(length<bestLen){
                vector<int> candidate(N);for(int x=0;x<N;x++)candidate[x]=index[current[x]];
                if(validate(candidate)){bestLen=length;best=move(candidate);stale=0;}
            }
            if(length>bestLen+6){for(int x=0;x<N;x++)current[x]=shape[best[x]];length=bestLen;}
        }
    }

    void localImprove(double end){
        deadline=end;oneOnly=false;mode=0;nodeLimit=1200;
        int failures=0;
        while(elapsed()<end&&bestLen>lower&&failures<250){
            vector<int> base(N);for(int x=0;x<N;x++)base[x]=1<<best[x];
            int centre=random()%N;for(int tries=0;tries<30&&!best[centre];tries++)centre=random()%N;
            int height=4+random()%9,width=4+random()%9;
            int r0=max(0,centre/C-height/2),r1=min(R,r0+height);
            int c0=max(0,centre%C-width/2),c1=min(C,c0+width);
            for(int r=r0;r<r1;r++)for(int c=c0;c<c1;c++)base[r*C+c]=initial[r*C+c];
            int old=bestLen;reset(base);limit=bestLen;search();
            if(bestLen<old)failures=0;else ++failures;
        }
    }

public:
    Solver(vector<string> g):R(g.size()),C(g[0].size()),N(R*C),grid(move(g)){
        nb.resize(N);pearl.resize(N);dom.resize(N);initial.resize(N);deg.resize(N);mask.resize(N);
        parent.resize(N);sz.resize(N);req.resize(N);seen.resize(N);activity.resize(N);queued.resize(N);
        for(int d=0;d<4;d++)for(int s=0;s<7;s++)((shape[s]&(1<<d))?on[d]:off[d])|=1<<s;
        for(int m=1;m<128;m++){
            forced[m]=15;for(int s=0;s<7;s++)if(m&(1<<s)){forced[m]&=shape[s];possible[m]|=shape[s];}
            for(int d=0;d<4;d++){
                if(m&on[d^2])edgeSupport[d][m]|=on[d];
                if(m&off[d^2])edgeSupport[d][m]|=off[d];
            }
        }
        int r0=R,r1=0,c0=C,c1=0;
        for(int r=0;r<R;r++)for(int c=0;c<C;c++){
            int x=r*C+c;nb[x]={r?x-C:-1,c+1<C?x+1:-1,r+1<R?x+C:-1,c?x-1:-1};
            pearl[x]=grid[r][c]=='W'?1:grid[r][c]=='B'?2:0;
            initial[x]=pearl[x]==1?6:pearl[x]==2?120:127;
            for(int d=0;d<4;d++)if(nb[x][d]<0)initial[x]&=off[d];
            if(pearl[x]){++P;r0=min(r0,r);r1=max(r1,r);c0=min(c0,c);c1=max(c1,c);}
        }
        lower=max(4,2*(r1-r0+c1-c0));
    }
    vector<string> solve(){
        satSearch(N>500?1.10:0.30,2,N>500);
        if(best.empty())satSearch(min(7.15,elapsed()+1.6),2,true,0,true);
        if(best.empty())satSearch(min(7.15,elapsed()+1.0),2,false);
        int attempt=0;
        while(best.empty()&&elapsed()<7.1){
            satSearch(min(7.15,elapsed()+0.6+0.2*attempt),attempt%3==2?-1:2,true,1,attempt%2==0);
            ++attempt;
        }
        if(!best.empty()&&bestLen>lower){
            if(N<=100){reset(initial);limit=bestLen;oneOnly=false;mode=0;nodeLimit=200000;deadline=0.40;search();}
            double end=min(7.3,max(0.85,elapsed()+0.12));
            faceImprove(min(end,elapsed()+0.12));
            localImprove(max(elapsed(),end-0.08));
            faceImprove(end);
            if(elapsed()<0.88)shortcutImprove(0.90);
        }
        vector<string> out(R,string(C,'.'));
        if(!best.empty())for(int x=0;x<N;x++)out[x/C][x%C]=glyph[best[x]];
        cerr<<"astra length="<<(best.empty()?0:bestLen)<<" nodes="<<totalNodes<<" time="<<elapsed()<<"\n";
        return out;
    }
};
constexpr int Solver::shape[7];
constexpr char Solver::glyph[8];

} // namespace astra_portfolio

// Alternate, independently implemented SAT feasibility portfolio.
static vector<char> astraCandidate(double globalEnd) {
    if (::elapsed() >= globalEnd) return {};
    astra_portfolio::Solver solver(G);
    solver.start = ::T0;
    solver.satSearch(globalEnd, 2, ::N > 500);
    if (solver.best.empty()) return {};
    solver.faceImprove(min(globalEnd, ::elapsed() + 0.03));
    if (!solver.validate(solver.best)) return {};
    vector<char> result(::E, 0);
    for (int e = 0; e < ::E; ++e) {
        int d = e < ::HN ? 1 : 2;
        result[e] = (astra_portfolio::Solver::shape[solver.best[::eu[e]]] & (1 << d)) != 0;
    }
    return result;
}

int main() {
    ios::sync_with_stdio(false); cin.tie(nullptr);
    if (!(cin >> R) || R < 1) return 0;
    G.resize(R); for (auto& row : G) cin >> row;
    C = G[0].size(); build(); sa::prep(); allPearlsReq();
    const double hardEnd = 7.45;
    double tend = 0.92;
    if (getenv("SEED")) rs ^= (uint64_t)atoll(getenv("SEED")) * 0x9E3779B97F4A7C15ULL;
    auto lenOf = [&](const vector<char>& v) { int l=0; for(char x:v) l+=x; return l; };
    auto validLoop = [&](const vector<char>& v) {
        if ((int)v.size()!=E) return false;
        vector<int> dg(N); int tot=0;
        for(int e=0;e<E;e++) if(v[e]) { dg[eu[e]]++; dg[ev[e]]++; tot++; }
        if(tot<4) return false;
        for(int x=0;x<N;x++) if((dg[x]!=0 && dg[x]!=2)||(isPearl[x] && dg[x]!=2)) return false;
        for(int pi=0;pi<nPearls;pi++) if(!pearlSat(v,pi)) return false;
        return (int)loopOrder(v).size()==tot;
    };
    auto polish = [&](vector<char>& cur,double dur,bool walk) {
        hardPolish(cur,min(hardEnd,elapsed()+dur),0.5,walk);
    };
    vector<char> start(E,0), best;
    for(int c=0;c+1<C;c++) { start[cedge[c*4+1]]=1; start[cedge[((R-1)*C+c)*4+1]]=1; }
    for(int r=0;r+1<R;r++) { start[cedge[(r*C)*4+2]]=1; start[cedge[(r*C+C-1)*4+2]]=1; }
    int bestL=1<<30;
    vector<char> cand;
    if (nPearls * 10 >= N) {
        auto direct=astraCandidate(0.18);
        if(!direct.empty() && validLoop(direct)) { polish(direct,0.03,false); cand=direct; }
    }
    if (cand.empty()) {
    vector<char> b; int bl=1<<30;
    sa::setState(start);
    sa::run(b,bl,0.25,20.0,3.0,0.3);
    vector<char> hint=sa::on;
    if(!b.empty() && validLoop(b)) cand=b;
    else {
        vector<char> rp=hint;
        if(repairLoop(rp,2.0) && validLoop(rp)) { polish(rp,0.03,false); cand=rp; }
        if(cand.empty() || (N<=1300 && elapsed()<0.45)) {
            vector<char> r;
            if(!cand.empty()) r=satSolve(elapsed()+0.2);
            else {
                r=astraCandidate(min(hardEnd-0.25,elapsed()+0.6));
                double bud=0.35;
                for(int k=0;r.empty() && elapsed()<hardEnd-0.30;k++) {
                    satRandAct=k>=2;
                    r=satSolve(min(hardEnd-0.25,elapsed()+bud),k%2==0?&hint:nullptr);
                    if(k%2==1) bud*=1.8;
                }
                satRandAct=0;
            }
            if(!r.empty() && validLoop(r)) {
                polish(r,0.05,false);
                if(cand.empty() || lenOf(r)<lenOf(cand)) cand=r;
            }
        }
    }
    }
    if(cand.empty() && elapsed()<hardEnd-0.05) {
        satRandAct=1;
        auto r=satSolve(hardEnd-0.02);
        if(!r.empty() && validLoop(r)) cand=r;
    }
    if(!cand.empty()) {
        best=cand; bestL=lenOf(best);
        vector<char> cur=cand;
        tend=min(hardEnd,max(tend,elapsed()+0.25));
        shortcutImprove(cur,min(tend,elapsed()+0.005));
        int fails=0,steps=0;
        while(elapsed()<tend) {
            if(++steps%40==0 && elapsed()+0.012<tend) {
                polish(cur,0.009,true);
                shortcutImprove(cur,min(tend,elapsed()+0.003));
                continue;
            }
            int e0; do { e0=rndi(E); } while(!cur[e0]);
            int x=eu[e0],grow=min(fails/30,6);
            int h=3+grow+rndi(6),w=3+grow+rndi(6);
            int r0=max(0,x/C-rndi(h)),c0=max(0,x%C-rndi(w));
            int r1=min(R-1,r0+h-1),c1=min(C-1,c0+w-1);
            bool ok;
            if(E<600 && rndi(10)==0) ok=lnsStep(cur,0,R-1,0,C-1,2000,tend);
            else ok=lnsStep(cur,r0,r1,c0,c1,500,tend);
            if(ok) fails=0; else fails++;
        }
        if(validLoop(cur) && lenOf(cur)<=bestL) { best=cur; bestL=lenOf(cur); }
    }
    // No bounded solver can promise feasibility on every hidden instance.
    // Retain the original starting loop only when independently valid.
    if(best.empty() && validLoop(start)) { best=start; bestL=lenOf(best); }
    if(best.empty()) best.assign(E,0);
    auto out=render(best);
    const char* path=getenv("OUTPUT_PATH"); FILE* file=path?fopen(path,"w"):stdout;
    if(!file) file=stdout;
    for(auto& row:out) fprintf(file,"%s\n",row.c_str());
    if(file!=stdout) fclose(file);
    cerr<<"bestL="<<bestL<<" t="<<elapsed()<<"\n";
    return 0;
}
