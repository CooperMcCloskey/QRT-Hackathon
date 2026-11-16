typedef unsigned long long u64;
namespace DL {
const int K = 28, MAXH = 14, GW = 64, NWD = 64, MAXPL = 320;
float lp1[K], lp2[K][K], lp3[K][K][K];
int minW = 40, maxWg = 60, minH = 10, maxH = 14, minPW = 3;
double TLIM = 1.6;
struct Piece { int h, w; vector<string> rows; vector<int> tr; vector<char> allsp; bool lead, up0; int cid; };
vector<Piece> pc; int N;
chrono::steady_clock::time_point T0;
double elapsed() { return chrono::duration<double>(chrono::steady_clock::now() - T0).count(); }
inline int dec64(char ch) { if (ch >= 'A' && ch <= 'Z') return ch - 'A'; if (ch >= 'a' && ch <= 'z') return ch - 'a' + 26; if (ch >= '0' && ch <= '9') return ch - '0' + 52; return ch == '+' ? 62 : 63; }
inline int cls(char ch) { if (ch >= 'a' && ch <= 'z') return ch - 'a'; if (ch >= 'A' && ch <= 'Z') return ch - 'A'; if (ch == ' ') return 26; return 27; }
inline bool isUp(char c) { return c >= 'A' && c <= 'Z'; }
inline bool isLo(char c) { return c >= 'a' && c <= 'z'; }
inline bool isAl(char c) { return isUp(c) || isLo(c); }
inline bool isDg(char c) { return c >= '0' && c <= '9'; }
void initLM() {
    for (int i = 0; i < K; i++) lp1[i] = -dec64(LM1[i]) / 5.5f;
    for (int i = 0; i < K * K; i++) lp2[i / K][i % K] = -dec64(LM2[i]) / 5.5f;
    for (int i = 0; i < K * K * K; i++) lp3[i / (K * K)][(i / K) % K][i % K] = -dec64(LM3[i]) / 5.5f;
}
// PMI of right chars (x,y) given left chars (a,b). a,y may be 0 (unknown).
inline float junc(char a, char b, char x, char y) {
    int cb = cls(b), cx = cls(x);
    float g = (a ? lp3[cls(a)][cb][cx] : lp2[cb][cx]) - lp1[cx];
    if (y && !(x == ' ' && y == ' ')) { int cy = cls(y); g += lp3[cb][cx][cy] - lp2[cx][cy]; }
    if (isUp(x) && isLo(b)) g -= 6;
    if (b == ' ' && a && (a == '.' || a == '!' || a == '?')) { if (isLo(x)) g -= 4; else if (isUp(x)) g += 2; }
    if (b == ' ' && (x == ',' || x == '.' || x == ';' || x == ':' || x == '!' || x == '?')) g -= 8;
    if ((b == ',' || b == ';' || b == ':' || b == '!' || b == '?') && isAl(x)) g -= 6;
    if (b == '.' && isLo(x)) g -= 4;
    if (isAl(b) && isDg(x)) g -= 4;
    if (isDg(b) && isAl(x)) g -= 2;
    return g;
}
// wrap junction: prev row (complete, width W) -> next row start (nl known chars)
float wrapJ(const char* prev, int W, const char* nx, int nl) {
    int e = W; while (e > 0 && prev[e - 1] == ' ') e--;
    int s = W - e; if (e < 2) return -20;
    char x = nx[0], y = nl > 1 ? nx[1] : 0; float g = 0;
    if (prev[e - 1] == '-' && isAl(prev[e - 2])) {
        char b = prev[e - 2], a = e >= 3 ? prev[e - 3] : 0;
        g = junc(a, b, x, y);
        if (!isLo(x)) g -= 5;
        if (s > 0) g -= 5; else g += 1.5f;
    } else {
        char a = prev[e - 1];
        if (s == 0) { char a2 = prev[e - 2]; int ca = cls(a); g += lp3[cls(a2)][ca][26] - lp1[26]; }
        g += junc(a, ' ', x, y);
        int L = 0; while (L < nl && nx[L] != ' ') L++;
        if (L < nl && L + 1 <= s) g -= 12;
    }
    return g;
}
struct St {
    float sc; short W, maxW, r, c, np, maxr, H; char done; u64 hash;
    char g[MAXH][GW]; u64 used[NWD]; short pl[MAXPL][3];
};
struct Cand { float sc; int par; short type, p, W, mw; };
inline u64 mix(u64 x) { x += 0x9E3779B97F4A7C15ULL; x = (x ^ (x >> 30)) * 0xBF58476D1CE4E5B9ULL; x = (x ^ (x >> 27)) * 0x94D049BB133111EBULL; return x ^ (x >> 31); }

// score placing piece p at (r,c) in state s. returns false if structurally invalid.
inline bool scorePlace(const St& s, int p, int r, int c, float& out, short& mw) {
    const Piece& P = pc[p]; float g = 0; int w = P.w; int W = s.W;
    for (int j = 0; j < P.h; j++) {
        int rr = r + j; const string& row = P.rows[j]; char x = row[0], y = w > 1 ? row[1] : 0;
        if (c == 0) {
            if (x == ' ') return false;
            if (j == 0 && r > 0) g += wrapJ(s.g[r - 1], W, row.data(), w);
        } else {
            char b = s.g[rr][c - 1];
            if (b) {
                char a = c > 1 ? s.g[rr][c - 2] : 0;
                if (b == ' ') {
                    if (x == ' ') { if (!P.allsp[j]) return false; }
                    else { if (a == ' ') return false; g += junc(a, b, x, y); }
                } else g += junc(a, b, x, y);
            }
        }
        if (W && c + w < W) {
            char R1 = s.g[rr][c + w];
            if (R1) {
                char R2 = c + w + 1 < W ? s.g[rr][c + w + 1] : 0;
                char b = row[w - 1], a = w > 1 ? row[w - 2] : (c > 0 ? s.g[rr][c - 1] : 0);
                if (b == ' ' && R1 == ' ') { if (R2 && R2 != ' ') return false; }
                else if (b == ' ' && a == ' ') return false;
                else g += junc(a, b, R1, R2);
            }
        }
        int t = P.tr[j];
        if (t >= 2 && rr < minH - 1) {
            if (W) { if (W - (c + w) > 3 - t) return false; }
            else mw = min((int)mw, c + w + 3 - t);
        }
        if (W && c + w == W) {
            if (t > 3 && rr < minH - 1) return false;
            if (t == 0 && w >= 2 && row[w - 1] == '-' && isAl(row[w - 2])) g += 1.5f;
        }
    }
    out = g; return true;
}
inline bool rowDone(St& s, int r) {
    int W = s.W; const char* g = s.g[r];
    int e = W; while (e > 0 && g[e - 1] == ' ') e--;
    int sp = W - e; if (e == 0) return false;
    for (int c = 1; c < e; c++) if (g[c] == ' ' && g[c - 1] == ' ') return false;
    if (g[0] == ' ') return false;
    if (sp > 3) { if (s.maxr > r + 1 || r + 1 < minH) return false; return true; }
    if (r + 1 < maxH && s.g[r + 1][0]) {
        int nl = 0; while (nl < W && s.g[r + 1][nl]) nl++;
        s.sc += wrapJ(g, W, s.g[r + 1], nl);
    }
    return true;
}
inline bool advance(St& s) {
    if (s.W == 0) return true;
    while (true) {
        if (s.r >= maxH) { s.done = 1; s.H = s.r; return true; }
        int c = s.c; while (c < s.W && s.g[s.r][c]) c++;
        if (c < s.W) { s.c = c; return true; }
        if (!rowDone(s, s.r)) return false;
        s.r++; s.c = 0;
        if (s.maxr <= s.r && s.r >= minH) { s.done = 1; s.H = s.r; return true; }
    }
}
struct PageRes { float sc; bool finished; int np; int W, H; vector<array<short, 3>> pl; bool valid; };

void genActions(int i, const St& s, vector<Cand>& out) {
    int r = s.r, c = s.c;
    if (s.W == 0) {
        int t0 = 0; while (t0 < c && s.g[0][c - 1 - t0] == ' ') t0++;
        if (c + 3 - t0 >= minW) {
            int lo = max(minW, c), hi = min((int)s.maxW, c + 3 - t0);
            for (int W = lo; W <= hi; W++) {
                float g = 0;
                if (W > c && t0 == 0) g = junc(s.g[0][c - 2], s.g[0][c - 1], ' ', 0);
                out.push_back({s.sc + g, i, 2, 0, (short)W, 0});
            }
        }
        if (t0 >= 2) return;
        for (int p = 0; p < N; p++) {
            if (s.used[p >> 6] >> (p & 63) & 1) continue;
            const Piece& P = pc[p];
            if (c + P.w > s.maxW) continue;
            float g; short mw = s.maxW;
            if (!scorePlace(s, p, 0, c, g, mw)) continue;
            out.push_back({s.sc + g, i, 0, (short)p, 0, mw});
        }
    } else {
        int W = s.W; int e = c; while (e < W && !s.g[r][e]) e++;
        int len = e - c;
        if (e == W && c > 0 && (len <= 3 || r >= minH - 1)) {
            char b = s.g[r][c - 1], a = c > 1 ? s.g[r][c - 2] : 0;
            float g = (b != ' ') ? junc(a, b, ' ', 0) : 0;
            out.push_back({s.sc + g, i, 1, 0, 0, 0});
        }
        for (int p = 0; p < N; p++) {
            if (s.used[p >> 6] >> (p & 63) & 1) continue;
            const Piece& P = pc[p];
            if (P.w > len || r + P.h > maxH) continue;
            int rem = len - P.w;
            if (rem > 0 && rem < minPW && !(e == W && rem <= 3)) continue;
            float g; short mw = 0;
            if (!scorePlace(s, p, r, c, g, mw)) continue;
            out.push_back({s.sc + g, i, 0, (short)p, 0, 0});
        }
    }
}
inline bool applyC(St& t, const Cand& cd) {
    t.sc = cd.sc;
    if (cd.type == 0) {
        const Piece& P = pc[cd.p]; int r = t.r, c = t.c;
        for (int j = 0; j < P.h; j++) memcpy(t.g[r + j] + c, P.rows[j].data(), P.w);
        t.used[cd.p >> 6] |= 1ULL << (cd.p & 63);
        if (t.np >= MAXPL) return false;
        t.pl[t.np][0] = cd.p; t.pl[t.np][1] = r; t.pl[t.np][2] = c; t.np++;
        if (r + P.h > t.maxr) t.maxr = r + P.h;
        t.hash ^= mix((u64)P.cid * 4096 + r * 64 + c);
        t.c = c + P.w;
        if (t.W == 0) t.maxW = cd.mw;
    } else if (cd.type == 1) {
        for (int c = t.c; c < t.W; c++) t.g[t.r][c] = ' ';
        t.hash ^= mix(999999 + t.r * 64 + t.c);
    } else {
        t.W = cd.W; for (int c = t.c; c < t.W; c++) t.g[0][c] = ' ';
        t.hash ^= mix(777777 + t.W); t.c = 0;
    }
    return advance(t);
}

PageRes runBeam(int start, const u64* usedG, int B) {
    PageRes res; res.valid = true; res.finished = false; res.sc = -1e9; res.np = 0; res.W = res.H = 0;
    vector<St> beam(1), nb; St& s0 = beam[0];
    memset(&s0, 0, sizeof(St)); memcpy(s0.used, usedG, sizeof(s0.used));
    s0.maxW = maxWg;
    { Cand cd{0, 0, 0, (short)start, 0, (short)maxWg}; applyC(s0, cd); }
    vector<Cand> cands; St bestFin; bool haveFin = false; St bestPart = s0;
    unordered_set<u64> seen;
    for (int step = 0; step < MAXPL + 40; step++) {
        cands.clear();
        for (int i = 0; i < (int)beam.size(); i++) genActions(i, beam[i], cands);
        if (cands.empty()) break;
        int M = min((int)cands.size(), 2 * B);
        if ((int)cands.size() > M) nth_element(cands.begin(), cands.begin() + M, cands.end(), [](const Cand& a, const Cand& b) { return a.sc > b.sc; });
        sort(cands.begin(), cands.begin() + M, [](const Cand& a, const Cand& b) { return a.sc > b.sc; });
        nb.clear(); seen.clear();
        for (int k = 0; k < M; k++) {
            nb.push_back(beam[cands[k].par]); St& t = nb.back();
            if (!applyC(t, cands[k]) || !seen.insert(t.hash).second) { nb.pop_back(); continue; }
            if (t.done) { if (!haveFin || t.sc > bestFin.sc) { bestFin = t; haveFin = true; } nb.pop_back(); continue; }
        }
        if (nb.empty()) break;
        sort(nb.begin(), nb.end(), [](const St& a, const St& b) { return a.sc > b.sc; });
        if ((int)nb.size() > B) nb.resize(B);
        beam.swap(nb);
        if (beam[0].np > bestPart.np) bestPart = beam[0];
        if ((step & 7) == 7 && elapsed() > TLIM) break;
    }
    const St& f = haveFin ? bestFin : bestPart;
    res.finished = haveFin; res.sc = f.sc; res.np = f.np; res.W = f.W; res.H = f.H;
    for (int i = 0; i < f.np; i++) res.pl.push_back({f.pl[i][0], f.pl[i][1], f.pl[i][2]});
    return res;
}

vector<vector<int>> solve(const vector<string>& raw) {
    T0 = chrono::steady_clock::now();
    initLM();
    N = min((int)raw.size(), NWD * 64); pc.assign(N, Piece());
    map<string, int> cidm; int total = 0;
    for (int i = 0; i < N; i++) {
        string s = raw[i]; while (!s.empty() && (s.back() == '\r' || s.back() == '\n' || s.back() == ' ')) s.pop_back();
        Piece& P = pc[i]; string cur;
        for (char ch : s) { if (ch == '|') { P.rows.push_back(cur); cur.clear(); } else cur += (ch == '_' ? ' ' : ch); }
        P.rows.push_back(cur);
        size_t w = 0; for (auto& r : P.rows) w = max(w, r.size());
        if (w == 0) { w = 1; }
        for (auto& r : P.rows) r.resize(w, ' ');
        P.h = P.rows.size(); P.w = w; P.lead = true;
        for (auto& r : P.rows) { int t = 0; while (t < (int)w && r[w - 1 - t] == ' ') t++; P.tr.push_back(t); P.allsp.push_back(t == (int)w); if (r[0] == ' ') P.lead = false; }
        char f = P.rows[0][0]; P.up0 = isUp(f) || isDg(f) || f == '"' || f == '\'';
        auto it = cidm.find(s); if (it == cidm.end()) it = cidm.insert({s, (int)cidm.size()}).first; P.cid = it->second;
        total += P.h * P.w;
    }
    minPW = 99; for (auto& P : pc) minPW = min(minPW, P.w);
    if (total < 1200) { minW = 8; minH = 2; }
    vector<vector<int>> ans;
    vector<u64> usedG(NWD, 0);
    vector<int> starts; for (int i = 0; i < N; i++) if (pc[i].lead && pc[i].up0 && pc[i].h <= maxH && pc[i].w <= maxWg) starts.push_back(i);
    vector<PageRes> cache(N); for (auto& c : cache) c.valid = false;
    int B = 60; int page = 0;
    while (elapsed() < TLIM) {
        int best = -1;
        for (int s : starts) {
            if (usedG[s >> 6] >> (s & 63) & 1) continue;
            if (cache[s].valid) { bool ok = true; for (auto& a : cache[s].pl) if (usedG[a[0] >> 6] >> (a[0] & 63) & 1) { ok = false; break; } if (!ok) cache[s].valid = false; }
            if (!cache[s].valid) { if (elapsed() > TLIM) continue; cache[s] = runBeam(s, usedG.data(), B); }
            if (!cache[s].valid) continue;
            float v = cache[s].sc + (cache[s].finished ? 1000.f : 0.f);
            if (best < 0 || v > cache[best].sc + (cache[best].finished ? 1000.f : 0.f)) best = s;
        }
        if (best < 0) break;
        PageRes& R = cache[best];
        if (!R.finished && R.np < 4) { usedG[best >> 6] |= 1ULL << (best & 63); continue; }
        for (auto& a : R.pl) { ans.push_back({a[0], page, a[1], a[2]}); usedG[a[0] >> 6] |= 1ULL << (a[0] & 63); }
#ifdef LOCAL
        fprintf(stderr, "page %d start %d sc %.1f fin %d np %d W %d H %d t=%.2f\n", page, best, R.sc, (int)R.finished, R.np, R.W, R.H, elapsed());
#endif
        page++; R.valid = false;
    }
    return ans;
}
} // namespace DL
