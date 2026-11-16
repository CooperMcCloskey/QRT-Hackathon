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
    if (checkedUpTo > trailv.size()) checkedUpTo = trailv.size();
    if (mark.empty()) mark.assign(N, 0);
    for (; checkedUpTo < trailv.size(); checkedUpTo++) {
        int l = trailv[checkedUpTo];
        if (l & 1) continue;
        int e = l >> 1; if (e >= E) continue;
        int start = eu[e], prevE = e, cur = ev[e], np = reqP[start], len = 1;
        bool closed = false;
        cyc.clear(); cyc.push_back(start);
        while (true) {
            if (cur == start) { closed = true; break; }
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

static vector<char> satSolve(double deadline, const vector<char>* hint = nullptr) {
    BR0 = 0; BR1 = 1 << 20; BC0 = 0; BC1 = 1 << 20;
    buildBase(E);
    if (hint) for (int e = 0; e < E; e++) cdcl::phase[e] = (*hint)[e];
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
        if (elapsed() > deadline) return false;
        fill(onLoop.begin(), onLoop.end(), 0);
        for (int e = 0; e < E; e++) if (cur[e]) { onLoop[eu[e]] = 1; onLoop[ev[e]] = 1; }
        vector<int> bad;
        for (int pi = 0; pi < nPearls; pi++) if (!pearlSat(cur, pi)) bad.push_back(pi);
        if (bad.empty()) { allPearlsReq(); return true; }
        int pi = bad[rndi(bad.size())];
        int p = pearls[pi], pr = p / C, pc = p % C;
        bool fixed = false;
        for (int rad = 3; !fixed; rad = rad * 3 / 2 + 1) {
            if (rad > 2 * max(R, C)) break;
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
