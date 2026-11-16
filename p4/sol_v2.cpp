// Masyu, shortest loop. DFS with propagation for a valid loop, then simulated
// annealing on face flips (keeps a single simple loop, pearl rules penalised).
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

// ------------------------------------------------------------------ DFS solver
namespace dfs {
static vector<int> st, don, dunk, endc, plen, ppr;
static int totalOn, closedFlag;
static vector<pair<int*, int>> trail;
static vector<int> qc, qp;            // cell queue, pearl queue
static vector<char> inqc, inqp;
static long long nodes;
static double deadline;
static bool timeout;

static inline void setv(int& x, int v) { trail.push_back({&x, x}); x = v; }
static inline void pushCell(int c) { if (!inqc[c]) { inqc[c] = 1; qc.push_back(c); } }
static inline void pushPearl(int p) { if (!inqp[p]) { inqp[p] = 1; qp.push_back(p); } }

static bool assign(int e, int val) {
    if (st[e] != -1) return st[e] == val;
    int u = eu[e], v = ev[e];
    if (val == 1) {
        if (don[u] >= 2 || don[v] >= 2) return false;
        int a = don[u] == 0 ? u : endc[u], b = don[v] == 0 ? v : endc[v];
        int la = don[u] == 0 ? 0 : plen[u], lb = don[v] == 0 ? 0 : plen[v];
        int pa = don[u] == 0 ? isPearl[u] : ppr[u], pb = don[v] == 0 ? isPearl[v] : ppr[v];
        if (a == v) {  // closing a loop
            if (la != totalOn || pa != nPearls) return false;
            setv(closedFlag, 1);
        } else {
            setv(endc[a], b); setv(endc[b], a);
            setv(plen[a], la + lb + 1); setv(plen[b], la + lb + 1);
            setv(ppr[a], pa + pb); setv(ppr[b], pa + pb);
        }
        setv(st[e], 1);
        setv(don[u], don[u] + 1); setv(don[v], don[v] + 1);
        setv(dunk[u], dunk[u] - 1); setv(dunk[v], dunk[v] - 1);
        setv(totalOn, totalOn + 1);
        if (a != v) {
            // edge between the two new ends: may only close if everything is in
            int dr = a / C - b / C, dc = a % C - b % C;
            if (abs(dr) + abs(dc) == 1) {
                int ee = -1;
                for (int d = 0; d < 4; d++) if (nbr[a * 4 + d] == b) ee = cedge[a * 4 + d];
                if (ee >= 0 && st[ee] == -1) {
                    if (!(la + lb + 1 == totalOn && pa + pb == nPearls)) {
                        if (!assign(ee, 0)) return false;
                    }
                }
            }
        }
    } else {
        setv(st[e], 0);
        setv(dunk[u], dunk[u] - 1); setv(dunk[v], dunk[v] - 1);
    }
    pushCell(u); pushCell(v);
    for (int pi : edgePearls[e]) pushPearl(pi);
    return true;
}

static bool checkCell(int x) {
    int on = don[x], unk = dunk[x];
    if (unk == 0) {
        if (on == 1) return false;
        if (on == 0 && isPearl[x]) return false;
        return true;
    }
    if (on == 2 || (on == 0 && !isPearl[x] && unk == 1) || closedFlag) {
        for (int d = 0; d < 4; d++) { int e = cedge[x * 4 + d]; if (e >= 0 && st[e] == -1 && !assign(e, 0)) return false; }
        return true;
    }
    if (on == 1 && unk == 1) {
        for (int d = 0; d < 4; d++) { int e = cedge[x * 4 + d]; if (e >= 0 && st[e] == -1) return assign(e, 1); }
    }
    if (on == 0 && isPearl[x]) {
        if (unk < 2) return false;
        if (unk == 2) for (int d = 0; d < 4; d++) { int e = cedge[x * 4 + d]; if (e >= 0 && st[e] == -1 && !assign(e, 1)) return false; }
    }
    return true;
}

static bool checkPearl(int pi) {
    auto& cf = pat[pi].cfg;
    int ncomp = 0;
    static int comp[8];
    for (int k = 0; k < (int)cf.size(); k++) {
        bool ok = true;
        for (auto& l : cf[k]) { int s = st[l.e]; if (s != -1 && s != l.v) { ok = false; break; } }
        if (ok) comp[ncomp++] = k;
    }
    if (ncomp == 0) return false;
    // literals common to all compatible configs
    for (auto& l : cf[comp[0]]) {
        if (st[l.e] != -1) continue;
        bool all = true;
        for (int j = 1; j < ncomp && all; j++) {
            bool found = false;
            for (auto& m : cf[comp[j]]) if (m.e == l.e) { found = (m.v == l.v); break; }
            if (!found) all = false;
        }
        if (all && !assign(l.e, l.v)) return false;
    }
    return true;
}

static bool propagate() {
    while (!qc.empty() || !qp.empty()) {
        if (!qp.empty()) {
            int p = qp.back(); qp.pop_back(); inqp[p] = 0;
            if (!checkPearl(p)) return false;
        } else {
            int c = qc.back(); qc.pop_back(); inqc[c] = 0;
            if (!checkCell(c)) return false;
        }
    }
    return true;
}
static void clearQ() {
    for (int c : qc) inqc[c] = 0;
    for (int p : qp) inqp[p] = 0;
    qc.clear(); qp.clear();
}
static void undo(size_t mark) {
    while (trail.size() > mark) { *trail.back().first = trail.back().second; trail.pop_back(); }
}

static vector<int> sol;
static int closeAll() {  // when closed, set all unknown off
    for (int e = 0; e < E; e++) if (st[e] == -1 && !assign(e, 0)) return 0;
    return propagate();
}

static vector<int> bfsq, seen;
static int seenStamp = 0;
static bool connected() {
    if (seen.empty()) { seen.assign(N, 0); bfsq.resize(N); }
    int s = -1;
    for (int pi = 0; pi < nPearls; pi++) { s = pearls[pi]; break; }
    if (s < 0) return true;
    ++seenStamp;
    int h = 0, t = 0; bfsq[t++] = s; seen[s] = seenStamp;
    int gotP = 0, gotOn = 0;
    while (h < t) {
        int x = bfsq[h++];
        gotP += isPearl[x]; gotOn += don[x];
        for (int d = 0; d < 4; d++) {
            int e = cedge[x * 4 + d];
            if (e < 0 || st[e] == 0) continue;
            int y = nbr[x * 4 + d];
            if (seen[y] != seenStamp) { seen[y] = seenStamp; bfsq[t++] = y; }
        }
    }
    return gotP == nPearls && gotOn == 2 * totalOn;
}

static bool tryLit(int e, int v) {
    size_t m = trail.size();
    bool ok = assign(e, v) && propagate() && connected();
    clearQ(); undo(m);
    return ok;
}
static long long probeBudget;
// failed-literal probing to fixpoint; returns false on contradiction
static bool probe() {
    bool changed = true;
    while (changed) {
        changed = false;
        for (int e = 0; e < E; e++) {
            if (st[e] != -1) continue;
            if (closedFlag) return true;
            // only probe edges touching something relevant
            int u = eu[e], v = ev[e];
            if (!(don[u] || don[v] || isPearl[u] || isPearl[v] || dunk[u] <= 2 || dunk[v] <= 2)) continue;
            if (--probeBudget < 0) return true;
            if (!tryLit(e, 1)) {
                if (!(assign(e, 0) && propagate() && connected())) return false;
                changed = true;
            } else if (!tryLit(e, 0)) {
                if (!(assign(e, 1) && propagate() && connected())) return false;
                changed = true;
            }
        }
    }
    return true;
}

static long long nodeLimit;
static bool useProbe = true;
static bool search(int depth) {
    if (timeout) return false;
    ++nodes;
    if ((nodes & 63) == 0 && elapsed() > deadline) { timeout = true; return false; }
    if (nodes > nodeLimit) return false;
    if (closedFlag) {
        size_t m = trail.size();
        if (closeAll()) { sol = st; return true; }
        clearQ(); undo(m);
        return false;
    }
    if (useProbe) {
        probeBudget = 4LL * E;
        if (!probe()) { clearQ(); return false; }
        if (closedFlag) {
            size_t m = trail.size();
            if (closeAll()) { sol = st; return true; }
            clearQ(); undo(m);
            return false;
        }
    }
    // choose branch: path end with fewest unknown edges
    int best = -1, bu = 99, ties = 0;
    for (int x = 0; x < N; x++) if (don[x] == 1) {
        int u = dunk[x];
        if (u < bu) { bu = u; best = x; ties = 1; }
        else if (u == bu && rndi(++ties) == 0) best = x;
    }
    if (best < 0) {
        for (int pi = 0; pi < nPearls; pi++) if (don[pearls[pi]] == 0) { best = pearls[pi]; break; }
    }
    if (best < 0) return false;
    int cand[4], nc = 0;
    for (int d = 0; d < 4; d++) { int e = cedge[best * 4 + d]; if (e >= 0 && st[e] == -1) cand[nc++] = e; }
    if (nc == 0) return false;
    int e = cand[rndi(nc)];
    int first = 1;
    for (int t = 0; t < 2; t++) {
        int val = t == 0 ? first : 1 - first;
        size_t m = trail.size();
        if (assign(e, val) && propagate() && connected()) {
            if (search(depth + 1)) return true;
        }
        clearQ(); undo(m);
        if (timeout || nodes > nodeLimit) return false;
    }
    return false;
}

static bool init() {
    st.assign(E, -1); don.assign(N, 0); dunk.assign(N, 0);
    endc.assign(N, 0); plen.assign(N, 0); ppr.assign(N, 0);
    for (int x = 0; x < N; x++) { endc[x] = x; for (int d = 0; d < 4; d++) if (cedge[x * 4 + d] >= 0) dunk[x]++; }
    totalOn = 0; closedFlag = 0;
    inqc.assign(N, 0); inqp.assign(nPearls, 0);
    trail.clear(); trail.reserve(1 << 16);
    for (int x = 0; x < N; x++) pushCell(x);
    for (int p = 0; p < nPearls; p++) pushPearl(p);
    return propagate();
}

// returns edge state vector or empty
static vector<int> solve(double dl) {
    deadline = dl; timeout = false; nodes = 0;
    if (!init()) return {};
    size_t base = trail.size();
    double lim = 200;
    while (!timeout) {
        sol.clear();
        nodes = 0; nodeLimit = (long long)lim;
        if (search(0)) return sol;
        undo(base); clearQ();
        lim *= 1.5;
    }
    return {};
}
}  // namespace dfs

#include "sat_part.h"
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

int main() {
    ios::sync_with_stdio(false);
    cin >> R;
    G.resize(R);
    for (int i = 0; i < R; i++) cin >> G[i];
    C = G[0].size();
    build();
    sa::prep();

    vector<char> best;
    int bestL = 1 << 30;

    const char* md = getenv("MODE");
    string mode = md ? md : "";
    double W = getenv("SAW") ? atof(getenv("SAW")) : 20.0;
    double TS = getenv("SATS") ? atof(getenv("SATS")) : 3.0;
    double TE = getenv("SATE") ? atof(getenv("SATE")) : 0.3;
    if (getenv("NOPROBE")) dfs::useProbe = false;
    vector<char> start(E, 0);
    for (int c = 0; c + 1 < C; c++) { start[cedge[c * 4 + 1]] = 1; start[cedge[((R - 1) * C + c) * 4 + 1]] = 1; }
    for (int r = 0; r + 1 < R; r++) { start[cedge[(r * C) * 4 + 2]] = 1; start[cedge[(r * C + C - 1) * 4 + 2]] = 1; }
    if (mode == "dfs") {
        vector<int> s = dfs::solve(8.5);
        if (!s.empty()) { best.assign(E, 0); for (int e = 0; e < E; e++) best[e] = s[e] == 1; bestL = 0; for (int e = 0; e < E; e++) bestL += best[e]; }
        cerr << "nodes=" << dfs::nodes << " ";
    } else if (mode == "sat") {
        vector<char> r = satSolve(8.5);
        if (!r.empty()) { best = r; bestL = 0; for (int e = 0; e < E; e++) bestL += best[e]; }
        cerr << "confl=" << cdcl::conflictsTotal << " iters=" << satIters << " hook=" << cdcl::hookConfl << " ";
    } else if (mode == "combo") {
        vector<char> r = satSolve(9.0);
        double tsat = elapsed();
        if (!r.empty()) {
            sa::setState(r); best = r; bestL = sa::L;
            sa::run(best, bestL, max(0.9, tsat + 0.3), W, TS, TE);
        } else {
            sa::setState(start);
            sa::run(best, bestL, max(0.9, tsat + 0.3), W, TS, TE);
        }
        cerr << "tsat=" << tsat << " ";
    } else if (mode == "full" || mode == "") {
        int wmin = getenv("WMIN") ? atoi(getenv("WMIN")) : 3, wmax = getenv("WMAX") ? atoi(getenv("WMAX")) : 8;
        long long cl = getenv("CL") ? atoll(getenv("CL")) : 500;
        double tend = getenv("TEND") ? atof(getenv("TEND")) : 0.9;
        double tsa = getenv("TSA") ? atof(getenv("TSA")) : 0.3;
        vector<char> r = satSolve(9.0);
        double tsat = elapsed();
        if (!r.empty()) { best = r; bestL = 0; for (int e = 0; e < E; e++) bestL += best[e]; }
        int satL = bestL;
        // SA from the border rectangle
        sa::setState(start);
        sa::run(best, bestL, max(tsa, elapsed() + 0.05), W, TS, TE);
        int saL = bestL;
        if (!best.empty()) {
            vector<char> cur = best;
            int fails = 0;
            while (elapsed() < tend) {
                int e0;
                do { e0 = rndi(E); } while (!cur[e0]);
                int x = eu[e0];
                int grow = min(fails / 30, 6);
                int h = wmin + grow + rndi(wmax - wmin + 1), w = wmin + grow + rndi(wmax - wmin + 1);
                int r0 = max(0, x / C - rndi(h)), c0 = max(0, x % C - rndi(w));
                int r1 = min(R - 1, r0 + h - 1), c1 = min(C - 1, c0 + w - 1);
                if (lnsStep(cur, r0, r1, c0, c1, cl, tend)) fails = 0; else fails++;
            }
            best = cur; bestL = 0; for (int e = 0; e < E; e++) bestL += best[e];
        }
        cerr << "satL=" << satL << " saL=" << saL << " tsat=" << tsat << " tried=" << lnsTried << " impr=" << lnsImproved << " unsat=" << lnsUnsat << " ";
    } else if (mode == "lns") {
        vector<char> r = satSolve(9.0);
        double tsat = elapsed();
        int wmin = getenv("WMIN") ? atoi(getenv("WMIN")) : 3, wmax = getenv("WMAX") ? atoi(getenv("WMAX")) : 8;
        long long cl = getenv("CL") ? atoll(getenv("CL")) : 500;
        double tend = getenv("TEND") ? atof(getenv("TEND")) : 0.9;
        if (!r.empty()) {
            vector<char> cur = r;
            while (elapsed() < tend) {
                // random loop cell
                vector<int> cells;
                int e0;
                do { e0 = rndi(E); } while (!cur[e0]);
                int x = eu[e0];
                int h = wmin + rndi(wmax - wmin + 1), w = wmin + rndi(wmax - wmin + 1);
                int r0 = max(0, x / C - rndi(h)), c0 = max(0, x % C - rndi(w));
                int r1 = min(R - 1, r0 + h - 1), c1 = min(C - 1, c0 + w - 1);
                lnsStep(cur, r0, r1, c0, c1, cl, tend);
            }
            best = cur; bestL = 0; for (int e = 0; e < E; e++) bestL += best[e];
        }
        cerr << "tsat=" << tsat << " tried=" << lnsTried << " impr=" << lnsImproved << " unsat=" << lnsUnsat << " ";
    } else if (mode == "sa") {
        sa::setState(start);
        sa::run(best, bestL, 0.85, W, TS, TE);
        cerr << "viol=" << sa::nViol << " L=" << sa::L << " ";
    } else {
    // 1) SA from the border rectangle (good on sparse puzzles)
    sa::setState(start);
    sa::run(best, bestL, 0.6, W, TS, TE);
    // 2) no valid loop: DFS (slow path), then polish with SA
    if (best.empty()) {
        vector<int> s = dfs::solve(8.0);
        if (!s.empty()) {
            vector<char> b2(E, 0); for (int e = 0; e < E; e++) b2[e] = s[e] == 1;
            sa::setState(b2);
            best = b2; bestL = sa::L;
            sa::run(best, bestL, elapsed() + 0.3, W, 0.5, 0.1);
        }
    }
    }
    if (best.empty()) best = start;
    vector<string> out = render(best);
    const char* op = getenv("OUTPUT_PATH");
    FILE* fo = op ? fopen(op, "w") : stdout;
    if (!fo) fo = stdout;
    for (auto& row : out) fprintf(fo, "%s\n", row.c_str());
    fclose(fo);
    cerr << "bestL=" << bestL << " t=" << elapsed() << "\n";
    return 0;
}
