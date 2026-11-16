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

static double TIME_LIMIT = 2.0;       // ASSUMED judge limit, seconds
static double TIME_BUDGET = 0.75 * 2.0;
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
static bool aborted = false;

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
    while (rem > 0) {
        if ((++steps & 4095) == 0 && elapsed() > TIME_LIMIT * 0.85) { aborted = true; return -1; }
        int best = head;
        for (int s = segN[head]; s != -1; s = segN[s]) if (segH[s] < segH[best]) best = s;
        ll h = segH[best], g = segW[best];
        ll lim = min(g, Hcap - h);
        int pl = segP[best], pr = segN[best];
        ll hl = pl == -1 ? Hcap : segH[pl];
        ll hr = pr == -1 ? Hcap : segH[pr];
        if (lim >= minSize) {
            int p = nxt[0];
            while (a[perm[p - 1]] > lim) p = nxt[p];
            int item = perm[p - 1];
            ll s = a[item];
            nxt[prv[p]] = nxt[p]; prv[nxt[p]] = prv[p];
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
    return left_area;
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
    for (int u = 0; u < n; u++) {
        int i = ord[u];
        if (X[i] < 0 || Y[i] < 0) return false;
        for (int v = u + 1; v < n; v++) {
            int j = ord[v];
            if (X[j] >= X[i] + a[i]) break;
            if (Y[i] < Y[j] + a[j] && Y[j] < Y[i] + a[i]) return false;
        }
    }
    return true;
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
            if (H == m || ++iters > 20000000) break;
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
    }

    // ---- stage 1: greedy (largest that fits) over many strip widths ----
    struct Cand { ll W, H, area; };
    vector<Cand> cands;
    if (bestArea > LB && R > 1) {
        ll lo = max((ll)m, (ll)(root / 3.0));
        ll hi = max(lo, min((S + m - 1) / m, (ll)ceil(root * 3.0)));
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
    }

    // ---- stage 2: SA on the order for the most promising widths ----
    if (!aborted && bestArea > LB && !cands.empty() && n > 1) {
        int K = (int)envd("SP_K", 10);
        double ROUNDS = envd("SP_ROUNDS", 6);
        double T0f = envd("SP_T0", 0.05), T1f = envd("SP_T1", 0.002);
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
                double t_end = min(TIME_BUDGET, t_begin + slice);
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
                        double frac = (now - t_begin) / slice;
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
