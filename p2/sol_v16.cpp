// Word Search Generation on a torus -- single-file C++17.
//
// Method: for a fixed side N the grid is kept consistent at all times.  Every
// word is inserted at the conflict-free placement (all 8 directions, with
// wrap) that shares the most letters with the grid; for small inputs the next
// word is chosen best-first by that overlap.  Words that do not fit drive a
// large-neighbourhood search: remove a few words (random, a window, or the
// ones blocking a stuck word), re-insert, keep the result if no more letters
// are left out.  N then shrinks, warm-started from the last packing.
// Forbidden words are checked with a trie on every placement, free cells are
// filled by min-conflicts, and every grid is re-checked by a brute-force
// validator before it can become the answer.
//
// Entry point for the judge: generate_grid(required_words, forbidden_words).
// main() is only a local adapter (JSON on stdin, rows on stdout).
#include <algorithm>
#include <array>
#include <chrono>
#include <climits>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <iterator>
#include <set>
#include <string>
#include <vector>
using namespace std;

static double TIME_BUDGET = 1.5;  // seconds of search; set to 75% of the judge limit

static chrono::steady_clock::time_point T0;
static inline double now() {
    return chrono::duration<double>(chrono::steady_clock::now() - T0).count();
}
static uint64_t rngs = 88172645463325252ULL;
static uint64_t SEED_ADD = 0;
static inline uint64_t rnd64() {
    rngs ^= rngs << 7;
    rngs ^= rngs >> 9;
    return rngs;
}
static inline uint32_t rnd(uint32_t n) { return (uint32_t)((rnd64() >> 11) % n); }
static inline double rnd01() { return (double)(rnd64() >> 11) * (1.0 / 9007199254740992.0); }

static const int DR[8] = {0, 1, 1, 1, 0, -1, -1, -1};
static const int DC[8] = {1, 0, 1, -1, -1, 0, -1, 1};
static const int EMPTY = 26;
static int MERGE_MIN = 2;
static double MERGE_WAIT = 0.25;
static double U_T = 0.0;
static long long RS_IT = 0;
static int BF_MODE = 1, BF_LNS = 0;
static double BF_NOISE = 2.0;
static int EJ_P = 40;
static const int WC = 2;  // weight of a letter conflict against one forbidden occurrence

// ---- instance ----
static vector<vector<uint8_t>> Wd;        // deduplicated required words
static vector<string> reqAll, forbAll;    // unique originals, for validation
static vector<array<int, 26>> trie;
static vector<uint8_t> term;
static int maxF = 0;
static bool hasForb = false;
static bool allowedL[26];
static int safeLetter = -1, firstAllowed = 0;
static vector<pair<int, int>> occ[26];
static int W = 0;

// ---- state for the current N ----
static int N, NN;
static vector<int> PR, PC, PD;
static vector<uint8_t> G;

static inline int wrapi(int x) { return x < 0 ? x + N : (x >= N ? x - N : x); }

// Forbidden occurrences (start, one of 4 line directions, trie word) covering cell.
static int cover(int cell) {
    if (G[cell] == EMPTY) return 0;
    int r = cell / N, c = cell % N, total = 0;
    int lim = min(maxF, N);
    for (int d = 0; d < 4; d++) {
        int sr = r, sc = c;
        for (int off = 0; off < lim; off++) {
            if (G[sr * N + sc] == EMPTY) break;
            int node = 0, rr = sr, cc = sc;
            for (int depth = 1; depth <= maxF; depth++) {
                int l = G[rr * N + cc];
                if (l == EMPTY) break;
                node = trie[node][l];
                if (node < 0) break;
                if (term[node] && depth > off) total++;
                rr = wrapi(rr + DR[d]);
                cc = wrapi(cc + DC[d]);
            }
            sr = wrapi(sr - DR[d]);
            sc = wrapi(sc - DC[d]);
        }
    }
    return total;
}

static long long fullF(vector<uint8_t>* mark) {
    long long total = 0;
    if (!hasForb) return 0;
    for (int cell = 0; cell < NN; cell++) {
        if (G[cell] == EMPTY) continue;
        int r = cell / N, c = cell % N;
        if (trie[0][G[cell]] < 0) continue;
        for (int d = 0; d < 4; d++) {
            int node = 0, rr = r, cc = c;
            for (int depth = 1; depth <= maxF; depth++) {
                int l = G[rr * N + cc];
                if (l == EMPTY) break;
                node = trie[node][l];
                if (node < 0) break;
                if (term[node]) {
                    total++;
                    if (mark) {
                        int r2 = r, c2 = c;
                        for (int k = 0; k < depth && k < N; k++) {
                            (*mark)[r2 * N + c2] = 1;
                            r2 = wrapi(r2 + DR[d]);
                            c2 = wrapi(c2 + DC[d]);
                        }
                    }
                }
                rr = wrapi(rr + DR[d]);
                cc = wrapi(cc + DC[d]);
            }
        }
    }
    return total;
}

// ---- consistent partial grid with ejection ----
static vector<vector<int>> cellWords;
static vector<float> cellW, weight, wplaced;
static vector<char> placed;
static vector<int> strikes;

static void unplaceWord(int j) {
    int r = PR[j], c = PC[j], dr = DR[PD[j]], dc = DC[PD[j]];
    float wl = wplaced[j];
    for (size_t k = 0; k < Wd[j].size(); k++) {
        int cell = r * N + c;
        vector<int>& v = cellWords[cell];
        for (size_t q = 0; q < v.size(); q++)
            if (v[q] == j) {
                v[q] = v.back();
                v.pop_back();
                break;
            }
        cellW[cell] -= wl;
        if (v.empty()) {
            G[cell] = EMPTY;
            cellW[cell] = 0;
        }
        r = wrapi(r + dr);
        c = wrapi(c + dc);
    }
    placed[j] = 0;
}

static void placeWord(int i, int r0, int c0, int d, vector<int>& ejected) {
    const vector<uint8_t>& w = Wd[i];
    int dr = DR[d], dc = DC[d];
    int r = r0, c = c0;
    for (size_t k = 0; k < w.size(); k++) {
        int cell = r * N + c;
        if (G[cell] != EMPTY && G[cell] != w[k]) {
            while (!cellWords[cell].empty()) {
                int j = cellWords[cell].back();
                unplaceWord(j);
                ejected.push_back(j);
            }
        }
        r = wrapi(r + dr);
        c = wrapi(c + dc);
    }
    PR[i] = r0;
    PC[i] = c0;
    PD[i] = d;
    wplaced[i] = weight[i] * (float)w.size();
    r = r0;
    c = c0;
    for (size_t k = 0; k < w.size(); k++) {
        int cell = r * N + c;
        G[cell] = w[k];
        cellWords[cell].push_back(i);
        cellW[cell] += wplaced[i];
        r = wrapi(r + dr);
        c = wrapi(c + dc);
    }
    placed[i] = 1;
}

// Would writing word i here create a forbidden occurrence? (conservative)
static bool createsForbidden(int i, int r0, int c0, int d) {
    if (!hasForb) return false;
    const vector<uint8_t>& w = Wd[i];
    int dr = DR[d], dc = DC[d];
    static vector<uint8_t> old;
    static vector<int> cells;
    old.clear();
    cells.clear();
    int r = r0, c = c0;
    for (size_t k = 0; k < w.size(); k++) {
        int cell = r * N + c;
        cells.push_back(cell);
        old.push_back(G[cell]);
        r = wrapi(r + dr);
        c = wrapi(c + dc);
    }
    for (size_t k = 0; k < w.size(); k++) G[cells[k]] = w[k];
    bool bad = false;
    for (size_t k = 0; k < w.size() && !bad; k++)
        if (old[k] != w[k] && cover(cells[k]) > 0) bad = true;
    for (size_t k = 0; k < w.size(); k++) G[cells[k]] = old[k];
    return bad;
}

struct Cand {
    int r, c, d;
};
static const int KC = 6;
static Cand cands[KC];
static int ncand;
static long long nties;
static float bestCost;
static int bestOv;
static int dropP;  // per-mille chance to ignore a candidate (diversification)

static inline void evalCand(int i, int r0, int c0, int d) {
    const vector<uint8_t>& w = Wd[i];
    int dr = DR[d], dc = DC[d], r = r0, c = c0, ov = 0;
    float cost = 0;
    for (size_t k = 0; k < w.size(); k++) {
        int cell = r * N + c;
        int g = G[cell];
        if (g != EMPTY) {
            if (g == w[k]) {
                ov++;
            } else {
                cost += cellW[cell];
                if (cost > bestCost) return;
            }
        }
        r += dr;
        if (r == N) r = 0; else if (r < 0) r = N - 1;
        c += dc;
        if (c == N) c = 0; else if (c < 0) c = N - 1;
    }
    if (cost == bestCost && ov < bestOv) return;
    if (dropP && (int)rnd(1000) < dropP) return;
    if (cost < bestCost || ov > bestOv) {
        bestCost = cost;
        bestOv = ov;
        ncand = 0;
        nties = 0;
    }
    nties++;
    if (ncand < KC) {
        cands[ncand++] = Cand{r0, c0, d};
    } else {
        long long q = (long long)(rnd64() % (uint64_t)nties);
        if (q < KC) cands[q] = Cand{r0, c0, d};
    }
}

static vector<vector<vector<int>>> posOf;  // posOf[word][letter] -> positions
static bool bigMode;

static void scanWord(int i) {
    ncand = 0;
    nties = 0;
    bestOv = -1;
    int L = (int)Wd[i].size();
    int nd = (L == 1) ? 1 : 8;
    if (!bigMode) {
        for (int d = 0; d < nd; d++)
            for (int r = 0; r < N; r++)
                for (int c = 0; c < N; c++) evalCand(i, r, c, d);
        return;
    }
    const vector<vector<int>>& po = posOf[i];
    for (int cell = 0; cell < NN; cell++) {
        int g = G[cell];
        if (g == EMPTY) continue;
        const vector<int>& ks = po[g];
        if (ks.empty()) continue;
        int cr = cell / N, cc = cell % N;
        for (int k : ks)
            for (int d = 0; d < nd; d++) {
                int r = wrapi(cr - k * DR[d]);
                int c = wrapi(cc - k * DC[d]);
                evalCand(i, r, c, d);
            }
    }
    int samples = (bestCost == 0 && bestOv > 0) ? 0 : 3000;
    for (int s = 0; s < samples; s++) evalCand(i, rnd(N), rnd(N), rnd(nd));
}

// ---- brute-force validation of a completely filled G ----
static vector<vector<int>> cellsOf;
static bool occursWord(const string& w) {
    int L = (int)w.size();
    if (L == 0) return true;
    const vector<int>& starts = cellsOf[w[0] - 'A'];
    for (int cell : starts) {
        int r0 = cell / N, c0 = cell % N;
        for (int d = 0; d < 8; d++) {
            int r = r0, c = c0, k = 1;
            for (; k < L; k++) {
                r = wrapi(r + DR[d]);
                c = wrapi(c + DC[d]);
                if (G[r * N + c] != w[k] - 'A') break;
            }
            if (k >= L) return true;
        }
    }
    return false;
}
static bool validateG() {
    cellsOf.assign(26, vector<int>());
    for (int cell = 0; cell < NN; cell++) {
        if (G[cell] >= 26) return false;
        cellsOf[G[cell]].push_back(cell);
    }
    for (const string& w : reqAll) {
        if ((int)w.size() > N) return false;
        if (!occursWord(w)) return false;
    }
    for (const string& w : forbAll)
        if (occursWord(w)) return false;
    return true;
}

// ---- fill the free cells without creating a forbidden word ----
static vector<int> filled;
static bool fillGrid(double deadline) {
    filled.clear();
    for (int cell = 0; cell < NN; cell++)
        if (G[cell] == EMPTY) filled.push_back(cell);
    if (filled.empty()) return true;
    if (!hasForb || safeLetter >= 0) {
        int l = hasForb ? safeLetter : firstAllowed;
        for (int cell : filled) G[cell] = (uint8_t)l;
        return true;
    }
    vector<int> al;
    for (int l = 0; l < 26; l++)
        if (allowedL[l]) al.push_back(l);
    if (al.empty()) al.push_back(0);
    for (int k = (int)filled.size() - 1; k > 0; k--) swap(filled[k], filled[rnd(k + 1)]);
    long long f = 0;
    auto choose = [&](int cell, bool noisy) {
        int old = G[cell];
        int bestv = INT_MAX, cnt = 0, pick = al[0];
        for (int l : al) {
            if (noisy && l == old && al.size() > 1) continue;
            G[cell] = (uint8_t)l;
            int v = cover(cell);
            if (v < bestv) {
                bestv = v;
                cnt = 1;
                pick = l;
            } else if (v == bestv) {
                cnt++;
                if (rnd(cnt) == 0) pick = l;
            }
        }
        if (noisy && rnd(10) == 0) pick = al[rnd((uint32_t)al.size())];
        G[cell] = (uint8_t)pick;
        return cover(cell);
    };
    for (int cell : filled) f += choose(cell, false);
    vector<uint8_t> isFill(NN, 0), mark;
    for (int cell : filled) isFill[cell] = 1;
    for (int round = 0; round < 300 && f > 0; round++) {
        if (round >= 25 && now() > deadline) break;
        mark.assign(NN, 0);
        f = fullF(&mark);
        if (f == 0) break;
        vector<int> bad;
        for (int cell : filled)
            if (mark[cell]) bad.push_back(cell);
        if (bad.empty()) break;
        for (int k = (int)bad.size() - 1; k > 0; k--) swap(bad[k], bad[rnd(k + 1)]);
        for (int cell : bad) {
            if (cover(cell) == 0) continue;
            choose(cell, true);
        }
        f = fullF(nullptr);
    }
    if (f == 0) return true;
    for (int cell : filled) G[cell] = EMPTY;
    return false;
}

// ---- one attempt at side n ----
static bool attempt(int n, double deadline, const vector<int>* wr, const vector<int>* wc,
                    const vector<int>* wd, vector<string>& out) {
    N = n;
    NN = n * n;
    G.assign(NN, EMPTY);
    PR.assign(W, 0);
    PC.assign(W, 0);
    PD.assign(W, 0);
    cellWords.assign(NN, vector<int>());
    cellW.assign(NN, 0.0f);
    weight.assign(W, 1.0f);
    wplaced.assign(W, 0.0f);
    placed.assign(W, 0);
    strikes.assign(W, 0);
    long long S = 0;
    for (auto& w : Wd) S += (long long)w.size();
    bigMode = (double)NN * 8.0 * (double)S > 6.0e7;
    vector<int> pending, ejected;
    if (wr) {
        // Warm start: keep every old placement that still fits modulo n.
        vector<int> order(W);
        for (int i = 0; i < W; i++) order[i] = i;
        for (int k = W - 1; k > 0; k--) swap(order[k], order[rnd(k + 1)]);
        for (int i : order) {
            int r = (*wr)[i] % n, c = (*wc)[i] % n, d = (*wd)[i];
            bestCost = 0;
            bestOv = -1;
            ncand = 0;
            nties = 0;
            dropP = 0;
            evalCand(i, r, c, d);
            if (ncand == 1 && !createsForbidden(i, r, c, d)) {
                ejected.clear();
                placeWord(i, r, c, d, ejected);
            } else {
                pending.push_back(i);
            }
        }
    } else {
        for (int i = 0; i < W; i++) pending.push_back(i);
    }
    // Greedy insert without ejection: place every pending word that has a
    // conflict-free spot, preferring the largest overlap.
    auto insertAll = [&](vector<int>& list, vector<int>& placedNow, int noise) -> long long {
        // order: longest first with noise
        vector<pair<float, int>> ord;
        for (int j : list)
            ord.push_back(make_pair(-(float)Wd[j].size() - (float)rnd01() * (float)noise, j));
        sort(ord.begin(), ord.end());
        vector<int> rest;
        long long un = 0;
        for (auto& pr : ord) {
            int i = pr.second;
            dropP = (noise > 2 && rnd(4) == 0) ? 300 : 0;
            int pick = -1;
            // If every top candidate would spell a forbidden word, rescan with
            // random candidate dropout to reach lower-overlap alternatives.
            for (int retry = 0; retry < (hasForb ? 4 : 1) && pick < 0; retry++) {
                if (retry) dropP = retry == 1 ? 500 : (retry == 2 ? 850 : 970);
                bestCost = 0;  // only conflict-free candidates survive the scan
                scanWord(i);
                if (ncand == 0 && retry == 0) break;
                if (ncand > 0) {
                    int st = rnd(ncand);
                    for (int q = 0; q < ncand; q++) {
                        int x = (st + q) % ncand;
                        if (!createsForbidden(i, cands[x].r, cands[x].c, cands[x].d)) {
                            pick = x;
                            break;
                        }
                    }
                }
            }
            if (pick < 0) {
                rest.push_back(i);
                un += (long long)Wd[i].size();
                continue;
            }
            ejected.clear();
            placeWord(i, cands[pick].r, cands[pick].c, cands[pick].d, ejected);
            placedNow.push_back(i);
        }
        list.swap(rest);
        return un;
    };
    // Best-first insert: always place the pending word whose best conflict-free
    // spot shares the most letters with the grid (jigsaw-style assembly).
    auto insertBF = [&](vector<int>& list, vector<int>& placedNow) -> long long {
        int m = (int)list.size();
        vector<int> ov(m, -1);
        vector<char> done(m, 0);
        vector<float> tie(m);
        for (int q = 0; q < m; q++) tie[q] = (float)Wd[list[q]].size() + (float)rnd01() * (float)BF_NOISE;
        auto rescan = [&](int q) {
            dropP = 0;
            bestCost = 0;
            scanWord(list[q]);
            ov[q] = ncand > 0 ? bestOv : -1;
            if (ov[q] < 0) done[q] = 2;
        };
        for (int q = 0; q < m; q++) rescan(q);
        int since = 0, every = max(1, m / 10);
        while (true) {
            int bq = -1;
            for (int q = 0; q < m; q++)
                if (!done[q] && (bq < 0 || ov[q] > ov[bq] || (ov[q] == ov[bq] && tie[q] > tie[bq]))) bq = q;
            if (bq < 0) break;
            int before = ov[bq];
            rescan(bq);
            if (done[bq]) continue;
            if (ov[bq] < before) continue;  // stale estimate, pick again
            int i = list[bq], pick = -1;
            int st = rnd(ncand);
            for (int z = 0; z < ncand; z++) {
                int x = (st + z) % ncand;
                if (!createsForbidden(i, cands[x].r, cands[x].c, cands[x].d)) {
                    pick = x;
                    break;
                }
            }
            if (pick < 0) {
                done[bq] = 2;
                continue;
            }
            ejected.clear();
            placeWord(i, cands[pick].r, cands[pick].c, cands[pick].d, ejected);
            placedNow.push_back(i);
            done[bq] = 1;
            if (++since >= every) {
                since = 0;
                for (int q = 0; q < m; q++)
                    if (!done[q]) rescan(q);
            }
        }
        vector<int> rest;
        long long un = 0;
        for (int q = 0; q < m; q++)
            if (done[q] != 1) {
                rest.push_back(list[q]);
                un += (long long)Wd[list[q]].size();
            }
        list.swap(rest);
        return un;
    };
    bool useBF = BF_MODE && W <= 150 && !bigMode;
    vector<int> placedNow, removed, sr, sc, sd;
    long long U = useBF ? insertBF(pending, placedNow) : insertAll(pending, placedNow, 1);
    int fillFails = 0;
    long long it = 0, sinceImp = 0;
    vector<char> inRem(W, 0);
    while (true) {
        if (U == 0) {
            if (fillGrid(deadline)) {
                if (validateG()) {
                    out.assign(n, string(n, 'A'));
                    for (int r = 0; r < n; r++)
                        for (int c = 0; c < n; c++) out[r][c] = (char)('A' + G[r * n + c]);
                    return true;
                }
                for (int cell : filled) G[cell] = EMPTY;
            }
            if (W == 0 || ++fillFails > 40) return false;
        }
        if (now() > deadline) {
            if (getenv("P2_DEBUG")) fprintf(stderr, "  fail n=%d U=%lld pending=%d iters=%lld\n", n, U, (int)pending.size(), it);
            return false;
        }
        it++;
        if (RS_IT > 0 && sinceImp > RS_IT && !bigMode) {
            // Stagnation: rebuild from scratch for a different basin.
            sinceImp = 0;
            for (int j = 0; j < W; j++)
                if (placed[j]) unplaceWord(j);
            pending.clear();
            for (int j = 0; j < W; j++) pending.push_back(j);
            placedNow.clear();
            U = (useBF && rnd(2)) ? insertBF(pending, placedNow) : insertAll(pending, placedNow, 3);
            continue;
        }
        // ---- destroy ----
        removed.clear();
        placedNow.clear();
        vector<int> oldPending = pending;
        int kmax = bigMode ? 4 : max(2, min(W / 3, 14));
        uint32_t mode = rnd(100);
        if (!pending.empty() && mode < (uint32_t)(bigMode ? 70 : EJ_P)) {
            // Targeted: force one stuck word in where it displaces the fewest
            // letters, then try to re-home what it displaced.
            int q = rnd((uint32_t)pending.size());
            int i = pending[q];
            dropP = 100 * (int)rnd(8);
            bestCost = 1e30f;
            scanWord(i);
            int pick = -1;
            if (ncand > 0) {
                int st = rnd(ncand);
                for (int z = 0; z < ncand; z++) {
                    int x = (st + z) % ncand;
                    if (!createsForbidden(i, cands[x].r, cands[x].c, cands[x].d)) {
                        pick = x;
                        break;
                    }
                }
            }
            if (pick < 0) continue;
            pending[q] = pending.back();
            pending.pop_back();
            ejected.clear();
            placeWord(i, cands[pick].r, cands[pick].c, cands[pick].d, ejected);
            placedNow.push_back(i);
            removed = ejected;
            sr.clear();
            sc.clear();
            sd.clear();
            for (int j : removed) {
                sr.push_back(PR[j]);
                sc.push_back(PC[j]);
                sd.push_back(PD[j]);
                pending.push_back(j);
            }
        } else {
            if (mode & 1) {
                int k = 1 + rnd(kmax);
                for (int t = 0; t < k; t++) {
                    int j = rnd(W);
                    if (placed[j] && !inRem[j]) {
                        inRem[j] = 1;
                        removed.push_back(j);
                    }
                }
            } else {
                int side = 2 + rnd(4);
                int r0 = rnd(n), c0 = rnd(n);
                for (int dr = 0; dr < side && dr < n; dr++)
                    for (int dc = 0; dc < side && dc < n; dc++) {
                        int cell = ((r0 + dr) % n) * n + (c0 + dc) % n;
                        for (int j : cellWords[cell])
                            if (!inRem[j] && (int)removed.size() < kmax + 4) {
                                inRem[j] = 1;
                                removed.push_back(j);
                            }
                    }
            }
            if (removed.empty()) continue;
            sr.clear();
            sc.clear();
            sd.clear();
            for (int j : removed) {
                sr.push_back(PR[j]);
                sc.push_back(PC[j]);
                sd.push_back(PD[j]);
                unplaceWord(j);
                inRem[j] = 0;
                pending.push_back(j);
            }
        }
        long long U2 = (useBF && (int)rnd(100) < BF_LNS) ? insertBF(pending, placedNow) : insertAll(pending, placedNow, 2 + (int)rnd(4));
        if (U2 < U) sinceImp = 0; else sinceImp++;
        if (U2 <= U || (U_T > 0 && rnd01() < exp(-(double)(U2 - U) / U_T))) {
            U = U2;
        } else {
            for (int j : placedNow) unplaceWord(j);
            for (size_t q = 0; q < removed.size(); q++) {
                ejected.clear();
                placeWord(removed[q], sr[q], sc[q], sd[q], ejected);
            }
            pending = oldPending;
        }
    }
}

vector<string> generate_grid(vector<string> required_words, vector<string> forbidden_words) {
    T0 = chrono::steady_clock::now();
    rngs = 88172645463325252ULL + SEED_ADD * 7919ULL;
    long long Stotal = 0;
    for (auto& w : required_words) Stotal += (long long)w.size();
    // unique originals
    {
        set<string> a, b;
        for (auto& w : required_words)
            if (!w.empty()) a.insert(w);
        for (auto& w : forbidden_words)
            if (!w.empty()) b.insert(w);
        reqAll.assign(a.begin(), a.end());
        forbAll.assign(b.begin(), b.end());
    }
    // deduplicate required words: reversals and contained words are free
    vector<string> canon;
    {
        set<string> s;
        for (auto& w : reqAll) {
            string rv(w.rbegin(), w.rend());
            s.insert(min(w, rv));
        }
        canon.assign(s.begin(), s.end());
        stable_sort(canon.begin(), canon.end(),
                    [](const string& x, const string& y) { return x.size() > y.size(); });
        vector<string> kept;
        for (auto& w : canon) {
            string rv(w.rbegin(), w.rend());
            bool inside = false;
            for (auto& o : kept) {
                if (o.size() < w.size()) continue;
                if (o.find(w) != string::npos || o.find(rv) != string::npos) {
                    inside = true;
                    break;
                }
            }
            if (!inside) kept.push_back(w);
        }
        canon = kept;
    }
    W = (int)canon.size();
    for (int l = 0; l < 26; l++) occ[l].clear();
    int maxlen = 0;
    long long Sd = 0;
    int need[26] = {0};
    for (int i = 0; i < W; i++) {
        int cntl[26] = {0};
        for (size_t k = 0; k < canon[i].size(); k++) {
            int a = canon[i][k] - 'A';

            cntl[a]++;
        }
        for (int l = 0; l < 26; l++) need[l] = max(need[l], cntl[l]);
        maxlen = max(maxlen, (int)canon[i].size());
        Sd += (long long)canon[i].size();
    }
    auto loadUnits = [&](const vector<string>& u) {
        W = (int)u.size();
        Wd.assign(W, vector<uint8_t>());
        for (int i = 0; i < W; i++)
            for (char ch : u[i]) Wd[i].push_back((uint8_t)(ch - 'A'));
        posOf.assign(W, vector<vector<int>>(26));
        for (int i = 0; i < W; i++)
            for (size_t k = 0; k < Wd[i].size(); k++) posOf[i][Wd[i][k]].push_back((int)k);
    };
    loadUnits(canon);
    // Chains: glue words end to end where they share MERGE_MIN+ letters (either
    // reading direction).  A chain contains its words, so placing chains is
    // still a valid answer, and the shared letters are saved for certain.
    vector<string> chains = canon;
    int mergedSaved = 0;
    if (MERGE_MIN > 0 && W >= 2 && W <= 140) {
        int cap = max(maxlen, (int)floor(sqrt((double)Sd / 1.45)));
        auto ovl = [](const string& a, const string& b) {
            int m = 0, lim = (int)min(a.size(), b.size()) - 1;
            for (int k = 1; k <= lim; k++)
                if (a.compare(a.size() - k, k, b, 0, k) == 0) m = k;
            return m;
        };
        while (true) {
            int bk = 0, bi = -1, bj = -1, bo = 0;
            int U = (int)chains.size();
            vector<string> rev(U);
            for (int i = 0; i < U; i++) rev[i] = string(chains[i].rbegin(), chains[i].rend());
            for (int i = 0; i < U; i++)
                for (int j = 0; j < U; j++) {
                    if (i == j) continue;
                    for (int o = 0; o < 4; o++) {
                        const string& a = (o & 1) ? rev[i] : chains[i];
                        const string& b = (o & 2) ? rev[j] : chains[j];
                        int k = ovl(a, b);
                        if (k >= MERGE_MIN && k > bk && (int)(a.size() + b.size()) - k <= cap) {
                            bk = k;
                            bi = i;
                            bj = j;
                            bo = o;
                        }
                    }
                }
            if (bi < 0) break;
            string a = (bo & 1) ? rev[bi] : chains[bi];
            string b = (bo & 2) ? rev[bj] : chains[bj];
            string m = a + b.substr(bk);
            vector<string> nx;
            for (int i = 0; i < U; i++)
                if (i != bi && i != bj) nx.push_back(chains[i]);
            nx.push_back(m);
            chains.swap(nx);
            mergedSaved += bk;
            if (now() > 0.2 * TIME_BUDGET) break;
        }
        // a chain must not itself spell a forbidden word
        if (mergedSaved > 0) {
            for (auto& c : chains) {
                string rv(c.rbegin(), c.rend());
                for (auto& f : forbAll)
                    if (c.find(f) != string::npos || rv.find(f) != string::npos) mergedSaved = 0;
            }
        }
    }
    // forbidden trie (both reading orientations)
    for (int l = 0; l < 26; l++) allowedL[l] = true;
    trie.assign(1, array<int, 26>());
    trie[0].fill(-1);
    term.assign(1, 0);
    maxF = 0;
    hasForb = false;
    bool inForb[26] = {false};
    for (auto& w : forbAll) {
        if (w.size() == 1) {
            allowedL[w[0] - 'A'] = false;
            continue;
        }
        string rv(w.rbegin(), w.rend());
        for (int t = 0; t < 2; t++) {
            const string& x = t ? rv : w;
            int node = 0;
            for (char ch : x) {
                int a = ch - 'A';
                inForb[a] = true;
                if (trie[node][a] < 0) {
                    trie[node][a] = (int)trie.size();
                    trie.push_back(array<int, 26>());
                    trie.back().fill(-1);
                    term.push_back(0);
                }
                node = trie[node][a];
            }
            term[node] = 1;
        }
        maxF = max(maxF, (int)w.size());
        hasForb = true;
    }
    safeLetter = -1;
    firstAllowed = 0;
    for (int l = 25; l >= 0; l--)
        if (allowedL[l]) {
            firstAllowed = l;
            if (!inForb[l]) safeLetter = l;
        }

    vector<string> best, cur;
    vector<int> br, bc, bd;
    int bestN = 0;
    if (W == 0) {
        for (int n = 1; n <= 40; n++) {
            if (attempt(n, now() + 0.3, nullptr, nullptr, nullptr, cur)) return cur;
        }
        return vector<string>(1, string(1, (char)('A' + firstAllowed)));
    }
    int cellsNeeded = 0;
    for (int l = 0; l < 26; l++) cellsNeeded += need[l];
    int lower = max(1, maxlen);
    while (lower * lower < cellsNeeded) lower++;
    int target = (int)floor(sqrt((double)Stotal / 1.7) + 1e-9);
    int stopN = max(lower, target);
    int n = max(lower, (int)ceil(sqrt((double)Sd / 0.85)));
    if ((double)n * n * 8.0 * (double)Sd > 6.0e7) n = max(lower, (int)ceil(sqrt((double)Sd / 0.95)));
    // Phase A: first valid grid.  Slices are generous: with nothing in hand a
    // late valid answer beats a punctual invalid one.
    int tries = 0;
    while (best.empty()) {
        double slice = max((tries == 0 ? 0.6 : 0.4) * TIME_BUDGET, 0.05);
        bool okA = attempt(n, now() + slice, nullptr, nullptr, nullptr, cur);
        if (getenv("P2_DEBUG")) fprintf(stderr, "A n=%d ok=%d t=%.3f big=%d\n", n, (int)okA, now(), (int)bigMode);
        if (okA) {
            best = cur;
            bestN = n;
            br = PR;
            bc = PC;
            bd = PD;
            break;
        }
        n += max(1, n / 6);
        if (++tries > 8) break;
    }
    if (best.empty()) {
        // Last resort: one word per row. Not validated against forbidden words.
        int m = max(maxlen, W);
        best.assign(m, string(m, (char)('A' + firstAllowed)));
        for (int i = 0; i < W; i++)
            for (size_t k = 0; k < canon[i].size(); k++) best[i][k] = canon[i][k];
        return best;
    }
    // Phase B: shrink.  A size that fails before its time is up (for example a
    // parity obstruction from the forbidden list) is skipped, not fatal.
    double lastDur = 0.0, lastOk = 0.0;
    bool haveChains = mergedSaved > 0;
    bool toChains = false;
    set<int> failedFull, failedShort;
    while (bestN > stopN && now() < TIME_BUDGET) {
        int step = 1;
        if (!bigMode && lastDur < 0.03 * TIME_BUDGET) step = max(1, (bestN - stopN) / 3);
        if (bigMode && lastDur < 1e8) step = max(step, min(bestN - stopN, bestN / 40));
        int cand = bestN - step;
        double deadline = TIME_BUDGET;
        if (step > 1 && !failedShort.count(cand) && !failedFull.count(cand)) {
            deadline = min(TIME_BUDGET, now() + max(0.08 * TIME_BUDGET, 6 * lastDur));
            if (bigMode) deadline = min(TIME_BUDGET, now() + max(0.3 * TIME_BUDGET, 2 * lastDur));
        } else {
            step = 1;
            cand = bestN - 1;
            while (cand >= stopN && failedFull.count(cand)) cand--;
            if (cand < stopN) break;
            if (haveChains) deadline = min(TIME_BUDGET, now() + max(MERGE_WAIT * TIME_BUDGET, 4 * lastOk));
        }
        double t = now();
        bool okk = attempt(cand, deadline, &br, &bc, &bd, cur);
        if (getenv("P2_DEBUG")) fprintf(stderr, "N=%d step=%d ok=%d t=%.3f dur=%.3f\n", cand, step, (int)okk, now(), now() - t);
        if (okk) {
            best = cur;
            bestN = cand;
            br = PR;
            bc = PC;
            bd = PD;
            lastDur = now() - t;
            lastOk = lastDur;
        } else if (deadline < TIME_BUDGET) {
            if (step == 1 && haveChains) {
                toChains = true;
                break;
            }
            failedShort.insert(cand);
            lastDur = 1e9;
        } else {
            failedFull.insert(cand);
        }
    }
    // Phase C: the plain search has stalled; pack the chains one size down.
    if (toChains && haveChains) {
        loadUnits(chains);
        int floorN = stopN;
        for (auto& c : chains) floorN = max(floorN, (int)c.size());
        set<int> failedC;
        bool warm = false;
        while (now() < TIME_BUDGET) {
            int cand = bestN - 1;
            while (cand >= floorN && failedC.count(cand)) cand--;
            if (cand < floorN) break;
            double t = now();
            bool okk = warm ? attempt(cand, TIME_BUDGET, &br, &bc, &bd, cur)
                            : attempt(cand, TIME_BUDGET, nullptr, nullptr, nullptr, cur);
            if (getenv("P2_DEBUG")) fprintf(stderr, "C N=%d ok=%d t=%.3f dur=%.3f units=%d saved=%d\n", cand, (int)okk, now(), now() - t, W, mergedSaved);
            if (okk) {
                best = cur;
                bestN = cand;
                br = PR;
                bc = PC;
                bd = PD;
                warm = true;
            } else {
                failedC.insert(cand);
            }
        }
    }
    return best;
}

// ---- local adapter: {"required_words": [...], "forbidden_words": [...]} on stdin ----
static vector<string> parseList(const string& s, const string& key) {
    vector<string> res;
    size_t p = s.find("\"" + key + "\"");
    if (p == string::npos) return res;
    p = s.find('[', p);
    if (p == string::npos) return res;
    size_t e = s.find(']', p);
    size_t q = p;
    while (true) {
        size_t a = s.find('"', q + 1);
        if (a == string::npos || a > e) break;
        size_t b = s.find('"', a + 1);
        res.push_back(s.substr(a + 1, b - a - 1));
        q = b;
    }
    return res;
}

int main() {
    string s((istreambuf_iterator<char>(cin)), istreambuf_iterator<char>());
    if (getenv("P2_BUDGET")) TIME_BUDGET = atof(getenv("P2_BUDGET"));
    if (getenv("P2_MM")) MERGE_MIN = atoi(getenv("P2_MM"));
    if (getenv("P2_MW")) MERGE_WAIT = atof(getenv("P2_MW"));
    if (getenv("P2_UT")) U_T = atof(getenv("P2_UT"));
    if (getenv("P2_RS")) RS_IT = atoll(getenv("P2_RS"));
    if (getenv("P2_BF")) BF_MODE = atoi(getenv("P2_BF"));
    if (getenv("P2_BFL")) BF_LNS = atoi(getenv("P2_BFL"));
    if (getenv("P2_EJ")) EJ_P = atoi(getenv("P2_EJ"));
    if (getenv("P2_SEED")) SEED_ADD = strtoull(getenv("P2_SEED"), 0, 10);
    vector<string> g = generate_grid(parseList(s, "required_words"), parseList(s, "forbidden_words"));
    string o;
    for (auto& row : g) o += row + "\n";
    fputs(o.c_str(), stdout);
    return 0;
}
