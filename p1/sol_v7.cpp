// Square packing++ : lowest-gap best-fit skyline decoder + simulated annealing
// over the priority order, for a set of fixed boxes (W x H), shrinking H.
// Always holds a valid answer (row -> shelf -> greedy skyline -> SA).
#include <algorithm>
#include <chrono>
#include <climits>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>
#include <vector>
using namespace std;

typedef long long ll;

static double TIME_LIMIT = 1.5;       // ASSUMED judge limit (HackerRank C++ default is 2 s)
static double TIME_BUDGET = 0.75 * 1.5;
static const ll INF = (ll)4e18;

static const chrono::steady_clock::time_point T0 = chrono::steady_clock::now();
static double elapsed() {
    return chrono::duration<double>(chrono::steady_clock::now() - T0).count();
}

static uint64_t rng_state = 88172645463325252ULL;
static inline uint64_t rnd() {
    rng_state ^= rng_state << 7;
    rng_state ^= rng_state >> 9;
    return rng_state;
}
static inline int rnd_int(int k) { return (int)(rnd() % (uint64_t)k); }
static inline double rnd01() { return (double)(rnd() >> 11) * (1.0 / 9007199254740992.0); }

static int n;
static vector<int> a;          // sides
static ll S = 0;               // sum of squared sides
static int m = 0;              // largest side
static int R = 0;              // number of distinct sizes
static vector<int> rk, rankSize, cnt0, cntw;
static vector<ll> segX, segW, segH;
static vector<int> segP, segN, nxt, prv;
static vector<ll> curX, curY;  // scratch positions written by decode
static vector<int> seqAt;      // item placed at each placement step
static vector<ll> limAt;       // gap limit at each placement step
static int nPlaced = 0;
static bool aborted = false;
static bool exactFit = false;
static int stepMask = 255;     // how often decode reads the clock

// Decode a priority order into a packing inside a strip of width W with
// height cap Hcap (INF = no cap). Repeatedly takes the lowest skyline gap and
// puts there the first square of the order that fits; if none fits the gap is
// raised to its lower neighbour. Returns the area of squares left unplaced.
static ll decode(const int* perm, ll W, ll Hcap, ll& Hmax) {
    int ns = 1, head = 0;
    segX[0] = 0; segW[0] = W; segH[0] = 0; segP[0] = -1; segN[0] = -1;
    for (int i = 0; i <= n; i++) { nxt[i] = i + 1; prv[i + 1] = i; }
    memcpy(cntw.data(), cnt0.data(), sizeof(int) * R);
    int minr = 0;
    ll minSize = rankSize[0];
    int rem = n;
    ll left_area = S;
    Hmax = 0;
    int steps = 0;
    nPlaced = 0;
    while (rem > 0) {
        if ((++steps & stepMask) == 0 && elapsed() > TIME_LIMIT * 0.8) { aborted = true; return -1; }
        int best = head;
        for (int s = segN[head]; s != -1; s = segN[s]) if (segH[s] < segH[best]) best = s;
        ll h = segH[best], g = segW[best];
        ll lim = min(g, Hcap - h);
        int pl = segP[best], pr = segN[best];
        ll hl = pl == -1 ? Hcap : segH[pl];
        ll hr = pr == -1 ? Hcap : segH[pr];
        if (lim >= minSize) {
            int p = nxt[0];
            if (exactFit && g <= Hcap - h) {
                int q = (int)(lower_bound(rankSize.begin(), rankSize.end(), (int)g) - rankSize.begin());
                if (q < R && rankSize[q] == g && cntw[q] > 0) {
                    while (a[perm[p - 1]] != g) p = nxt[p];
                } else {
                    while (a[perm[p - 1]] > lim) p = nxt[p];
                }
            } else
            while (a[perm[p - 1]] > lim) p = nxt[p];
            int item = perm[p - 1];
            ll s = a[item];
            nxt[prv[p]] = nxt[p]; prv[nxt[p]] = prv[p];
            seqAt[n - rem] = item; limAt[n - rem] = lim;
            rem--;
            left_area -= s * s;
            if (--cntw[rk[item]] == 0 && rem > 0) {
                while (cntw[minr] == 0) minr++;
                minSize = rankSize[minr];
            }
            bool left = (hl >= hr);
            ll nh = h + s;
            if (nh > Hmax) Hmax = nh;
            curX[item] = h;
            if (s == g) {
                curY[item] = segX[best];
                segH[best] = nh;
                if (pl != -1 && segH[pl] == nh) {
                    segW[pl] += segW[best];
                    segN[pl] = pr; if (pr != -1) segP[pr] = pl;
                    best = pl;
                }
                pr = segN[best];
                if (pr != -1 && segH[pr] == nh) {
                    segW[best] += segW[pr];
                    int q = segN[pr];
                    segN[best] = q; if (q != -1) segP[q] = best;
                }
            } else if (left) {
                curY[item] = segX[best];
                segX[best] += s; segW[best] -= s;
                if (pl != -1 && segH[pl] == nh) {
                    segW[pl] += s;
                } else {
                    int q = ns++;
                    segX[q] = segX[best] - s; segW[q] = s; segH[q] = nh;
                    segP[q] = pl; segN[q] = best; segP[best] = q;
                    if (pl != -1) segN[pl] = q; else head = q;
                }
            } else {
                curY[item] = segX[best] + g - s;
                segW[best] -= s;
                if (pr != -1 && segH[pr] == nh) {
                    segX[pr] -= s; segW[pr] += s;
                } else {
                    int q = ns++;
                    segX[q] = segX[best] + g - s; segW[q] = s; segH[q] = nh;
                    segP[q] = best; segN[q] = pr; segN[best] = q;
                    if (pr != -1) segP[pr] = q;
                }
            }
        } else {
            if (h >= Hcap) break;
            ll t = min(hl, hr);
            if (t >= INF / 2) break;   // cannot happen when W >= largest side
            segH[best] = t;
            if (pl != -1 && segH[pl] == t) {
                segW[pl] += segW[best];
                segN[pl] = pr; if (pr != -1) segP[pr] = pl;
                best = pl;
            }
            pr = segN[best];
            if (pr != -1 && segH[pr] == t) {
                segW[best] += segW[pr];
                int q = segN[pr];
                segN[best] = q; if (q != -1) segP[q] = best;
            }
        }
    }
    nPlaced = n - rem;
    return left_area;
}

// ---- rollout beam search over the placement sequence ----
// A state is an order whose first k placements are fixed; its cost is the
// result of decoding the whole order (the rest acts as the rollout policy).
struct BState {
    vector<int> perm;   // normalised: placement sequence, then unplaced items
    vector<ll> lim;     // gap limit seen at each placement step
    int placed;
    ll cost;
    uint64_t hash;
};
static vector<int> stampRank, stampItem;
static int stampCur = 0;

static void normalise(const int* perm, BState& st, ll cost) {
    st.perm.resize(n);
    st.lim.assign(limAt.begin(), limAt.begin() + nPlaced);
    st.placed = nPlaced;
    st.cost = cost;
    ++stampCur;
    uint64_t hsh = 1469598103934665603ULL;
    for (int t = 0; t < nPlaced; t++) {
        st.perm[t] = seqAt[t];
        stampItem[seqAt[t]] = stampCur;
        hsh = (hsh ^ (uint64_t)a[seqAt[t]]) * 1099511628211ULL;
    }
    int w = nPlaced;
    for (int t = 0; t < n; t++) if (stampItem[perm[t]] != stampCur) st.perm[w++] = perm[t];
    st.hash = hsh;
}

// Improves `perm` for box (W, Ht). Returns the best cost found (0 = fits).
static ll beam_run(ll W, ll Ht, vector<int>& perm, int B, int C, double t_end) {
    ll hm;
    ll c = decode(perm.data(), W, Ht, hm);
    if (aborted) return -1;
    vector<BState> beam(1), next;
    normalise(perm.data(), beam[0], c);
    ll bestCost = c;
    vector<int> bestPerm = beam[0].perm;
    vector<int> child(n), js;
    for (int k = 0; k < n && bestCost > 0; k++) {
        next.clear();
        bool live = false;
        for (size_t bi = 0; bi < beam.size(); bi++) {
            const BState& st = beam[bi];
            next.push_back(st);
            if (k >= st.placed) continue;
            live = true;
            ll lim = st.lim[k];
            int curSize = a[st.perm[k]];
            ++stampCur;
            js.clear();
            for (int j = k + 1; j < n; j++) {
                int it = st.perm[j];
                if (a[it] <= lim && a[it] != curSize && stampRank[rk[it]] != stampCur) {
                    stampRank[rk[it]] = stampCur;
                    js.push_back(j);
                }
            }
            if ((int)js.size() > C) {
                sort(js.begin(), js.end(), [&](int p, int q) { return a[st.perm[p]] > a[st.perm[q]]; });
                int keep = C - C / 4;
                for (int t = keep; t < C; t++) {
                    int u = t + rnd_int((int)js.size() - t);
                    swap(js[t], js[u]);
                }
                js.resize(C);
            }
            for (size_t ji = 0; ji < js.size(); ji++) {
                int j = js[ji];
                memcpy(child.data(), st.perm.data(), sizeof(int) * n);
                int v = child[j];
                for (int t = j; t > k; t--) child[t] = child[t - 1];
                child[k] = v;
                ll cc = decode(child.data(), W, Ht, hm);
                if (aborted) { perm = bestPerm; return bestCost; }
                next.push_back(BState());
                normalise(child.data(), next.back(), cc);
                if (cc < bestCost) {
                    bestCost = cc; bestPerm = next.back().perm;
                    if (cc == 0) { perm = bestPerm; return 0; }
                }
            }
            if (elapsed() > t_end) { perm = bestPerm; return bestCost; }
        }
        if (!live) break;
        // keep the B best distinct states
        vector<int> idx(next.size());
        for (size_t t = 0; t < idx.size(); t++) idx[t] = (int)t;
        stable_sort(idx.begin(), idx.end(), [&](int p, int q) {
            if (next[p].cost != next[q].cost) return next[p].cost < next[q].cost;
            return next[p].hash < next[q].hash;
        });
        vector<BState> nb;
        for (size_t t = 0; t < idx.size() && (int)nb.size() < B; t++) {
            if (t > 0 && next[idx[t]].cost == next[idx[t - 1]].cost && next[idx[t]].hash == next[idx[t - 1]].hash) continue;
            nb.push_back(BState());
            nb.back().perm.swap(next[idx[t]].perm);
            nb.back().lim.swap(next[idx[t]].lim);
            nb.back().placed = next[idx[t]].placed;
            nb.back().cost = next[idx[t]].cost;
            nb.back().hash = next[idx[t]].hash;
        }
        beam.swap(nb);
    }
    perm = bestPerm;
    return bestCost;
}

static vector<ll> bestX, bestY;
static ll bestArea;

static ll bbox_area(const vector<ll>& X, const vector<ll>& Y) {
    ll x0 = INF, y0 = INF, x1 = -INF, y1 = -INF;
    for (int i = 0; i < n; i++) {
        x0 = min(x0, X[i]); y0 = min(y0, Y[i]);
        x1 = max(x1, X[i] + a[i]); y1 = max(y1, Y[i] + a[i]);
    }
    return (x1 - x0) * (y1 - y0);
}

static bool take_if_better(const vector<ll>& X, const vector<ll>& Y) {
    ll ar = bbox_area(X, Y);
    if (ar < bestArea) { bestArea = ar; bestX = X; bestY = Y; return true; }
    return false;
}

static bool valid(const vector<ll>& X, const vector<ll>& Y) {
    vector<int> ord(n);
    for (int i = 0; i < n; i++) ord[i] = i;
    sort(ord.begin(), ord.end(), [&](int p, int q) { return X[p] < X[q]; });
    ll ops = 0;
    for (int u = 0; u < n; u++) {
        int i = ord[u];
        if (X[i] < 0 || Y[i] < 0) return false;
        for (int v = u + 1; v < n; v++) {
            int j = ord[v];
            // Huge inputs: stop the safety check rather than run out of time;
            // every packing here is valid by construction.
            if (++ops > 20000000) return true;
            if (X[j] >= X[i] + a[i]) break;
            if (Y[i] < Y[j] + a[j] && Y[j] < Y[i] + a[i]) return false;
        }
    }
    return true;
}

// ---- exact search for (near-)perfect boxes ----
// Always fills the lowest, leftmost skyline corner with some remaining square.
// Waste is only allowed where nothing fits and is charged against the budget
// W*H - S. With budget 0 this is a complete search for a perfect packing.
struct Seg { int x, w, h; };
static vector<vector<Seg> > dSeg;
static vector<int> dCnt, dRank, dPX, dPH;
static ll dNodes, dNodeLimit, dBudget;
static double dDeadline;
static bool dStop;
static int dHt;
static double dNoise = 0;
static vector<vector<pair<double, int> > > dCand;

static bool dfs(int depth, int placed, ll waste, int minr) {
    if (placed == n) return true;
    if (depth + 1 >= (int)dSeg.size()) return false;
    if (++dNodes > dNodeLimit) { dStop = true; return false; }
    if ((dNodes & 1023) == 0 && elapsed() > dDeadline) { dStop = true; return false; }
    const vector<Seg>& sg = dSeg[depth];
    int ns = (int)sg.size();
    while (dCnt[minr] == 0) minr++;
    int minSize = rankSize[minr];
    int best = 0;
    ll lbw = 0;
    for (int i = 0; i < ns; i++) {
        if (sg[i].h < sg[best].h) best = i;
        int hl = i ? sg[i - 1].h : dHt;
        int hr = i + 1 < ns ? sg[i + 1].h : dHt;
        int lo = min(hl, hr);
        if (dHt - sg[i].h < minSize) lbw += (ll)sg[i].w * (dHt - sg[i].h);
        else if (lo > sg[i].h && sg[i].w < minSize) lbw += (ll)sg[i].w * (lo - sg[i].h);
    }
    if (waste + lbw > dBudget) return false;
    Seg b = sg[best];
    int h = b.h, g = b.w;
    int hl = best ? sg[best - 1].h : dHt;
    int hr = best + 1 < ns ? sg[best + 1].h : dHt;
    int lim = min(g, dHt - h);
    vector<Seg>& ch = dSeg[depth + 1];
    if (lim < minSize) {
        if (h >= dHt) return false;
        int t = min(hl, hr);
        ch.clear();
        for (int i = 0; i < ns; i++) {
            Seg s = sg[i];
            if (i == best) s.h = t;
            if (!ch.empty() && ch.back().h == s.h) ch.back().w += s.w;
            else ch.push_back(s);
        }
        return dfs(depth + 1, placed, waste + (ll)g * (t - h), minr);
    }
    int rmax = (int)(upper_bound(rankSize.begin(), rankSize.end(), lim) - rankSize.begin()) - 1;
    // candidate sizes, largest first; restarts perturb the order
    vector<pair<double, int> >& cd = dCand[depth];
    cd.clear();
    for (int r = rmax; r >= minr; r--) {
        if (!dCnt[r]) continue;
        double key = (double)rankSize[r];
        if (dNoise > 0) key *= 1.0 - dNoise * rnd01();
        cd.push_back(make_pair(-key, r));
    }
    if (dNoise > 0) sort(cd.begin(), cd.end());
    for (size_t ci = 0; ci < cd.size(); ci++) {
        int r = cd[ci].second;
        int s = rankSize[r];
        int nh = h + s;
        ch.clear();
        for (int i = 0; i < best; i++) ch.push_back(sg[i]);
        if (!ch.empty() && ch.back().h == nh) ch.back().w += s;
        else { Seg q; q.x = b.x; q.w = s; q.h = nh; ch.push_back(q); }
        if (s < g) { Seg q; q.x = b.x + s; q.w = g - s; q.h = h; ch.push_back(q); }
        for (int i = best + 1; i < ns; i++) {
            if (ch.back().h == sg[i].h) ch.back().w += sg[i].w;
            else ch.push_back(sg[i]);
        }
        dCnt[r]--;
        dRank[placed] = r; dPX[placed] = b.x; dPH[placed] = h;
        if (dfs(depth + 1, placed + 1, waste, minr)) return true;
        dCnt[r]++;
        if (dStop) return false;
    }
    return false;
}

// Returns 1 = packed (positions in curX/curY), 0 = proven impossible for this
// search, -1 = stopped by the node or time limit.
static int exact_box(ll W, ll H, ll nodeLimit, double deadline) {
    dSeg.assign(3 * n + 8, vector<Seg>());
    if ((int)dCand.size() < 3 * n + 8) dCand.assign(3 * n + 8, vector<pair<double, int> >());
    dCnt = cnt0;
    dRank.assign(n, 0); dPX.assign(n, 0); dPH.assign(n, 0);
    dNodes = 0; dNodeLimit = nodeLimit; dDeadline = deadline; dStop = false;
    dHt = (int)H; dBudget = W * H - S;
    Seg s0; s0.x = 0; s0.w = (int)W; s0.h = 0;
    dSeg[0].push_back(s0);
    bool ok = dfs(0, 0, 0, 0);
    if (!ok) return dStop ? -1 : 0;
    vector<vector<int> > byRank(R);
    for (int i = 0; i < n; i++) byRank[rk[i]].push_back(i);
    for (int p = 0; p < n; p++) {
        int item = byRank[dRank[p]].back();
        byRank[dRank[p]].pop_back();
        curX[item] = dPH[p]; curY[item] = dPX[p];
    }
    return 1;
}

struct Arm {
    ll W, Hcur, LBH, Ht, cost;
    vector<int> perm;
    bool dead;
};

static double envd(const char* name, double def) {
    const char* v = getenv(name);
    return v ? atof(v) : def;
}

int main() {
    TIME_LIMIT = envd("SP_TL", TIME_LIMIT);
    TIME_BUDGET = 0.75 * TIME_LIMIT;
    exactFit = envd("SP_EX", 1) > 0.5;
    // ---- tolerant input: every integer on stdin; drop a leading count ----
    string text, line;
    {
        char buf[1 << 16];
        size_t got;
        while ((got = fread(buf, 1, sizeof buf, stdin)) > 0) text.append(buf, got);
    }
    vector<vector<ll> > rows;
    {
        vector<ll> cur;
        size_t i = 0, L = text.size();
        while (i <= L) {
            if (i == L || text[i] == '\n') {
                if (!cur.empty()) rows.push_back(cur);
                cur.clear();
                i++;
            } else if (text[i] >= '0' && text[i] <= '9') {
                ll v = 0;
                while (i < L && text[i] >= '0' && text[i] <= '9') { v = v * 10 + (text[i] - '0'); i++; }
                cur.push_back(v);
            } else i++;
        }
    }
    vector<ll> flat;
    for (auto& r : rows) for (ll v : r) flat.push_back(v);
    if (rows.size() >= 2 && rows[0].size() == 1 && rows[0][0] == (ll)flat.size() - 1) flat.erase(flat.begin());
    n = (int)flat.size();
    if (n == 0) return 0;
    stepMask = n > 20000 ? 3 : (n > 2000 ? 15 : 255);
    a.resize(n);
    ll sumSides = 0;
    for (int i = 0; i < n; i++) { a[i] = (int)flat[i]; S += (ll)a[i] * a[i]; m = max(m, a[i]); sumSides += a[i]; }

    // ---- baseline 0: one row (always valid) ----
    vector<ll> rowX(n, 0), rowY(n);
    { ll y = 0; for (int i = 0; i < n; i++) { rowY[i] = y; y += a[i]; } }
    bestX = rowX; bestY = rowY; bestArea = bbox_area(bestX, bestY);

    vector<int> desc(n);
    for (int i = 0; i < n; i++) desc[i] = i;
    stable_sort(desc.begin(), desc.end(), [&](int p, int q) { return a[p] > a[q]; });
    double root = sqrt((double)S);

    // ---- baseline 1: shelves, O(n) per width ----
    {
        vector<ll> X(n), Y(n);
        for (int k = 0; k < 24; k++) {
            ll W = max((ll)m, (ll)(root * (0.6 + 0.05 * k)));
            ll x = 0, y = 0, sh = a[desc[0]];
            for (int t = 0; t < n; t++) {
                int i = desc[t];
                if (x + a[i] > W) { y += sh; x = 0; sh = a[i]; }
                X[i] = y; Y[i] = x; x += a[i];
            }
            take_if_better(X, Y);
        }
    }

    // ---- lower bound L ----
    ll LB = S;
    {
        ll best = INF;
        ll iters = 0;
        for (ll W = m;; W++) {
            ll H = max((ll)m, (S + W - 1) / W);
            best = min(best, W * H);
            if (H == m) break;
            if (++iters > 2000000) { best = S; break; }   // too long: S is still a true bound
        }
        LB = best;
    }

    // ---- structures ----
    {
        vector<int> sizes(a.begin(), a.end());
        sort(sizes.begin(), sizes.end());
        sizes.erase(unique(sizes.begin(), sizes.end()), sizes.end());
        R = (int)sizes.size();
        rankSize = sizes;
        rk.resize(n); cnt0.assign(R, 0); cntw.assign(R, 0);
        for (int i = 0; i < n; i++) {
            rk[i] = (int)(lower_bound(sizes.begin(), sizes.end(), a[i]) - sizes.begin());
            cnt0[rk[i]]++;
        }
        segX.assign(n + 3, 0); segW.assign(n + 3, 0); segH.assign(n + 3, 0);
        segP.assign(n + 3, 0); segN.assign(n + 3, 0);
        nxt.assign(n + 3, 0); prv.assign(n + 3, 0);
        curX.assign(n, 0); curY.assign(n, 0);
        seqAt.assign(n + 1, 0); limAt.assign(n + 1, 0);
        stampRank.assign(R + 1, 0); stampItem.assign(n + 1, 0);
    }

    // ---- stage 1: greedy (largest that fits) over many strip widths ----
    struct Cand { ll W, H, area; };
    vector<Cand> cands;
    if (bestArea > LB && R > 1) {
        ll lo = max((ll)m, (ll)(root * envd("SP_WLO", 0.3333)));
        ll hi = max(lo, min((S + m - 1) / m, (ll)ceil(root * envd("SP_WHI", 1.05))));
        vector<ll> widths;
        ll cntW = hi - lo + 1;
        ll MAXW = 3000;
        if (cntW <= MAXW) for (ll W = lo; W <= hi; W++) widths.push_back(W);
        else for (ll k = 0; k < MAXW; k++) widths.push_back(lo + (ll)((double)(cntW - 1) * k / (MAXW - 1)));
        // visit in a scattered order so a time cut still covers the range
        for (size_t i = widths.size(); i > 1; i--) swap(widths[i - 1], widths[rnd_int((int)i)]);
        double stage1_end = TIME_BUDGET * envd("SP_S1", 0.2);
        for (size_t k = 0; k < widths.size(); k++) {
            if (k > 0 && elapsed() > stage1_end) break;
            ll W = widths[k], Hm;
            ll r = decode(desc.data(), W, INF, Hm);
            if (aborted) break;
            if (r != 0) continue;
            Cand c; c.W = W; c.H = Hm; c.area = W * Hm;
            cands.push_back(c);
            if (c.area < bestArea) take_if_better(curX, curY);
        }
        sort(cands.begin(), cands.end(), [&](const Cand& p, const Cand& q) {
            if (p.area != q.area) return p.area < q.area;
            return fabs(p.W - root) < fabs(q.W - root);
        });
        // spread the first arms over the width range: best greedy per bucket
        int BK = (int)envd("SP_BK", 0);
        if (BK > 0 && cands.size() > (size_t)BK) {
            ll wmin = INF, wmax = 0;
            for (auto& c : cands) { wmin = min(wmin, c.W); wmax = max(wmax, c.W); }
            double l0 = log((double)wmin), l1 = log((double)wmax + 1.0);
            vector<char> used(cands.size(), 0);
            vector<Cand> sel;
            for (int b = 0; b < BK; b++) {
                double x0 = exp(l0 + (l1 - l0) * b / BK), x1 = exp(l0 + (l1 - l0) * (b + 1) / BK);
                for (size_t t = 0; t < cands.size(); t++) {
                    if (!used[t] && cands[t].W >= x0 && cands[t].W < x1) { used[t] = 1; sel.push_back(cands[t]); break; }
                }
            }
            for (size_t t = 0; t < cands.size(); t++) if (!used[t]) sel.push_back(cands[t]);
            cands.swap(sel);
        }
    }

    // ---- stage 1.5: exact search on the boxes whose area equals the bound ----
    if (!aborted && bestArea > LB && n > 1 && n <= 600 && R > 1) {
        vector<pair<ll, ll> > boxes;
        ll iters = 0;
        for (ll W = m; W <= 1000000; W++) {
            ll H = max((ll)m, (S + W - 1) / W);
            if (W * H == LB && H <= 1000000) boxes.push_back(make_pair(W, H));
            if (H == m || ++iters > 2000000) break;
        }
        sort(boxes.begin(), boxes.end(), [&](const pair<ll, ll>& p, const pair<ll, ll>& q) {
            return fabs(p.first - root) < fabs(q.first - root);
        });
        if (boxes.size() > 16) boxes.resize(16);
        double dfsEnd = TIME_BUDGET * envd("SP_DFS", 0.75);
        vector<char> dead(boxes.size(), 0);
        bool found = false;
        double NZ = envd("SP_NZ", 0.3), GROW = envd("SP_GROW", 1.5);
        int round = 0;
        for (double limd = envd("SP_LIM0", 20000); !found && elapsed() < dfsEnd; limd *= GROW, round++) {
            ll lim = (ll)limd;
            bool anyLive = false;
            dNoise = round == 0 ? 0.0 : NZ * (0.5 + 0.5 * (round % 3));
            for (size_t b = 0; b < boxes.size() && !found && elapsed() < dfsEnd; b++) {
                if (dead[b]) continue;
                int r = exact_box(boxes[b].first, boxes[b].second, lim, dfsEnd);
                if (r == 1) { take_if_better(curX, curY); found = true; }
                else if (r == 0) dead[b] = 1;
                else anyLive = true;
            }
            if (!anyLive) break;
        }
    }

    // ---- stage 2: SA on the order for the most promising widths ----
    if (!aborted && bestArea > LB && !cands.empty() && n > 1) {
        int K = (int)envd("SP_K", 10);
        double ROUNDS = envd("SP_ROUNDS", 6);
        double T0f = envd("SP_T0", 0.05), T1f = envd("SP_T1", 0.002);
        double BF = envd("SP_BF", 0.5);
        int C = (int)envd("SP_C", 10), BMAX = (int)envd("SP_BMAX", 200);
        double Tstart = T0f * (double)S / n, Tend = T1f * (double)S / n;
        if (Tend < 0.3) Tend = 0.3;
        if (Tstart < Tend) Tstart = Tend;
        size_t nextCand = 0;
        vector<Arm> arms;
        auto make_arm = [&](const Cand& c) {
            Arm A; A.W = c.W; A.Hcur = c.H; A.LBH = max((ll)m, (S + c.W - 1) / c.W);
            A.Ht = -1; A.cost = -1; A.perm = desc; A.dead = false;
            return A;
        };
        while ((int)arms.size() < K && nextCand < cands.size()) arms.push_back(make_arm(cands[nextCand++]));
        double stage2_start = elapsed();
        double slice = max(1e-4, (TIME_BUDGET - stage2_start) / (K * ROUNDS));
        vector<int> cand(n);
        bool any = true;
        while (any && elapsed() < TIME_BUDGET && bestArea > LB && !aborted) {
            any = false;
            for (size_t ai = 0; ai < arms.size() && !aborted; ai++) {
                Arm& A = arms[ai];
                // retarget or retire
                while (true) {
                    ll Ht = min(A.Hcur - 1, (bestArea - 1) / A.W);
                    if (Ht >= A.LBH) {
                        if (Ht != A.Ht) { A.Ht = Ht; ll hm; A.cost = decode(A.perm.data(), A.W, Ht, hm); }
                        break;
                    }
                    if (nextCand < cands.size()) A = make_arm(cands[nextCand++]);
                    else { A.dead = true; break; }
                }
                if (A.dead || aborted) continue;
                any = true;
                double t_begin = elapsed();
                double sa_len = slice * (1.0 - BF);
                double t_end = min(TIME_BUDGET, t_begin + sa_len);
                double temp = Tstart;
                int it = 0;
                while (true) {
                    if (A.cost == 0) {
                        // decode again to get positions for this exact order
                        ll hm;
                        decode(A.perm.data(), A.W, A.Ht, hm);
                        take_if_better(curX, curY);
                        A.Hcur = hm;
                        ll Ht = min(A.Hcur - 1, (bestArea - 1) / A.W);
                        if (Ht < A.LBH) break;
                        A.Ht = Ht;
                        A.cost = decode(A.perm.data(), A.W, Ht, hm);
                        if (aborted) break;
                        continue;
                    }
                    if ((it & 7) == 0) {
                        double now = elapsed();
                        if (now >= t_end) break;
                        double frac = (now - t_begin) / sa_len;
                        temp = Tstart * pow(Tend / Tstart, frac);
                    }
                    ++it;
                    int mv = rnd_int(10);
                    int i = rnd_int(n), j;
                    if (mv < 4) {
                        j = i + 1 + rnd_int(3);
                        if (j >= n) continue;
                    } else {
                        j = rnd_int(n);
                        if (j == i) continue;
                    }
                    memcpy(cand.data(), A.perm.data(), sizeof(int) * n);
                    if (mv < 7) {
                        if (a[cand[i]] == a[cand[j]]) continue;
                        swap(cand[i], cand[j]);
                    } else {
                        int v = cand[i];
                        if (i < j) { for (int t = i; t < j; t++) cand[t] = cand[t + 1]; }
                        else { for (int t = i; t > j; t--) cand[t] = cand[t - 1]; }
                        cand[j] = v;
                    }
                    ll hm;
                    ll nc = decode(cand.data(), A.W, A.Ht, hm);
                    if (aborted) break;
                    ll delta = nc - A.cost;
                    if (delta <= 0 || rnd01() < exp(-(double)delta / temp)) {
                        A.perm.swap(cand);
                        A.cost = nc;
                    }
                }
                // rollout beam from the annealed order
                if (!aborted && BF > 0 && A.cost > 0 && elapsed() < TIME_BUDGET) {
                    double tdec = it > 0 ? (elapsed() - t_begin) / it : 1e-5;
                    double tb = slice * BF;
                    int B = (int)(tb / (tdec * n * C + 1e-12));
                    if (B < 1) B = 1;
                    if (B > BMAX) B = BMAX;
                    vector<int> p = A.perm;
                    ll bc = beam_run(A.W, A.Ht, p, B, C, min(TIME_BUDGET, elapsed() + 2 * tb));
                    if (!aborted && bc >= 0 && bc < A.cost) { A.perm = p; A.cost = bc; }
                    if (!aborted && A.cost == 0) {
                        ll hm;
                        decode(A.perm.data(), A.W, A.Ht, hm);
                        take_if_better(curX, curY);
                        A.Hcur = hm;
                    }
                }
            }
        }
    }

    // ---- output, with a last validity check and fallback ----
    if (!valid(bestX, bestY)) { bestX = rowX; bestY = rowY; }
    ll x0 = INF, y0 = INF;
    for (int i = 0; i < n; i++) { x0 = min(x0, bestX[i]); y0 = min(y0, bestY[i]); }
    string out;
    char tmp[64];
    for (int i = 0; i < n; i++) {
        snprintf(tmp, sizeof tmp, "%lld %lld\n", bestX[i] - x0, bestY[i] - y0);
        out += tmp;
    }
    fwrite(out.data(), 1, out.size(), stdout);
    return 0;
}
