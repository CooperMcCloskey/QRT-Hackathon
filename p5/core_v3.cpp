typedef unsigned long long u64;
namespace DL {
const int K = 27, MAXH = 14, GW = 64, NWD = 64, MAXPL = 320;
float lp1[K], lp2[K][K], lp3[K][K][K];
int minW = 40, maxWg = 60, minH = 10, maxH = 14, minPW = 3;
double TLIM = 1.6;
float LINK_T = 6.0f, LINK_D = 3.0f, WJ = 0.5f;
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
inline u64 mix(u64 x) { x += 0x9E3779B97F4A7C15ULL; x = (x ^ (x >> 30)) * 0xBF58476D1CE4E5B9ULL; x = (x ^ (x >> 27)) * 0x94D049BB133111EBULL; return x ^ (x >> 31); }
struct HS { vector<u64> t; u64 mask;
    void init(int lg) { t.assign(1ULL << lg, 0); mask = (1ULL << lg) - 1; }
    inline void add(u64 h, int b) { h = (mix(h) & ~7ULL) | 4; u64 i = (h >> 3) & mask; while (t[i] && (t[i] & ~3ULL) != h) i = (i + 1) & mask; if (!t[i] || (int)(t[i] & 3) > b) t[i] = h | b; }
    inline int has(u64 h) const { h = (mix(h) & ~7ULL) | 4; u64 i = (h >> 3) & mask; while (t[i]) { if ((t[i] & ~3ULL) == h) return (int)(t[i] & 3) + 1; i = (i + 1) & mask; } return 0; } } HT;
const u64 SW = 0x1111, SP = 0x2222, SS = 0x3333, SI = 0x4444, FP = 1099511628211ULL;
inline u64 hstr(const char* s, int n, u64 salt) { u64 h = salt; for (int i = 0; i < n; i++) { h ^= (unsigned char)s[i]; h *= FP; } return h; }
void initLM() {
    vector<string> words; vector<int> bk; int nb = 0;
    { // decode 5-bit symbol stream
        int L = strlen(WL); vector<int> sy; int acc = 0, nbit = 0;
        for (int i = 0; i < L; i++) { acc |= dec64(WL[i]) << nbit; nbit += 6; while (nbit >= 5) { sy.push_back(acc & 31); acc >>= 5; nbit -= 5; } }
        string prev, cur; int pp = 0; bool open = false; size_t i = 0;
        auto flush = [&]() { if (open && !cur.empty()) { words.push_back(cur); bk.push_back(nb); prev = cur; } open = false; };
        while (i < sy.size()) {
            int s = sy[i++];
            if (s < 26) { if (open) cur += char('a' + s); continue; }
            flush();
            int p;
            if (s == 31) { if (i >= sy.size()) break; int v = sy[i++]; if (v == 31) { nb++; prev.clear(); pp = 0; continue; } p = v; }
            else p = pp + (s == 26 ? 0 : s == 27 ? 1 : s == 28 ? -1 : s == 29 ? 2 : -2);
            if (p < 0 || p > (int)prev.size()) break;
            cur = prev.substr(0, p); pp = p; open = true;
        }
        flush();
    }
    int lg = 16; while ((1ULL << lg) < words.size() * 130ULL) lg++;
    HT.init(lg);
    static double c1[K], c2[K][K], c3[K][K][K], st[K]; memset(c1, 0, sizeof c1); memset(c2, 0, sizeof c2); memset(c3, 0, sizeof c3); memset(st, 0, sizeof st);
    static const double BW[6] = {300, 40, 8, 2, 2, 2};
    auto isV = [](char c) { return c == 'a' || c == 'e' || c == 'i' || c == 'o' || c == 'u'; };
    for (size_t wi = 0; wi < words.size(); wi++) {
        const string& w = words[wi]; int n = w.size(); double wt = BW[min(bk[wi], 5)];
        int bq = min(bk[wi], 3); HT.add(hstr(w.data(), n, SW), bq);
        string forms[4]; int nf = 1; forms[0] = w;
        if (n >= 3) {
            char l = w[n - 1], l2 = w[n - 2];
            bool cy = (l == 'y' && !isV(l2));
            if (l == 's' || l == 'x' || l == 'z' || (l == 'h' && (l2 == 'c' || l2 == 's'))) forms[nf++] = w + "es"; else if (cy) forms[nf++] = w.substr(0, n - 1) + "ies"; else forms[nf++] = w + "s";
            if (l == 'e') forms[nf++] = w + "d"; else if (cy) forms[nf++] = w.substr(0, n - 1) + "ied"; else forms[nf++] = w + "ed";
            if (l == 'e' && l2 != 'e') forms[nf++] = w.substr(0, n - 1) + "ing"; else forms[nf++] = w + "ing";
        }
        for (int f = 0; f < nf; f++) {
            const string& s = forms[f]; int m = s.size(); double ww = f ? wt * 0.2 : wt;
            { u64 h = SP; for (int i = 0; i < m; i++) { h ^= (unsigned char)s[i]; h *= FP; HT.add(h, bq); } }
            for (int i = 1; i < m; i++) HT.add(hstr(s.data() + i, m - i, SS), bq);
            for (int i = 0; i < m; i++) { u64 h = SI; for (int j = i; j < m && j - i < 8; j++) { h ^= (unsigned char)s[j]; h *= FP; if (j - i >= 2) HT.add(h, bq); } }
            int q[40]; int ql = 0; q[ql++] = 26; for (int i = 0; i < m && ql < 38; i++) q[ql++] = s[i] - 'a'; q[ql++] = 26;
            st[q[1]] += ww;
            for (int i = 0; i < ql; i++) { c1[q[i]] += ww; if (i >= 1) c2[q[i - 1]][q[i]] += ww; if (i >= 2) c3[q[i - 2]][q[i - 1]][q[i]] += ww; }
        }
    }
    double T = 0, ST = 0; for (int i = 0; i < K; i++) { T += c1[i]; ST += st[i]; }
    for (int b = 0; b < 26; b++) { double tot = c2[b][26]; for (int y = 0; y < 26; y++) c3[b][26][y] = tot * st[y] / max(ST, 1.0); }
    double p1[K], p2[K][K];
    for (int i = 0; i < K; i++) { p1[i] = (c1[i] + 1) / (T + K); lp1[i] = log(p1[i]); }
    for (int a = 0; a < K; a++) { double s = 0; for (int b = 0; b < K; b++) s += c2[a][b]; for (int b = 0; b < K; b++) { p2[a][b] = (c2[a][b] + 5 * p1[b]) / (s + 5); lp2[a][b] = log(p2[a][b]); } }
    for (int a = 0; a < K; a++) for (int b = 0; b < K; b++) { double s = 0; for (int c = 0; c < K; c++) s += c3[a][b][c]; for (int c = 0; c < K; c++) lp3[a][b][c] = max(-9.0, log((c3[a][b][c] + 3 * p2[b][c]) / (s + 3))); }
}
inline bool ends(const char* w, int n, const char* sf, int m) { return n - m >= 3 && memcmp(w + n - m, sf, m) == 0; }
inline int wHas(const char* w, int n) { return HT.has(hstr(w, n, SW)); }
int inDict(char* w, int n) {
    int r = wHas(w, n); if (r) return r;
    static const char* SUF[10] = {"s", "es", "ed", "d", "ing", "ly", "er", "est", "al", "ness"};
    static const int SL[10] = {1, 2, 2, 1, 3, 2, 2, 3, 2, 4};
    for (int k = 0; k < 10; k++) {
        int m = SL[k]; if (!ends(w, n, SUF[k], m)) continue;
        int sn = n - m; if ((r = wHas(w, sn))) return r;
        bool eg = (k == 2 || k == 4 || k == 6 || k == 7);
        char sv = w[sn];
        if (eg) { w[sn] = 'e'; r = wHas(w, sn + 1); w[sn] = sv; if (r) return r; if (sn >= 4 && w[sn - 1] == w[sn - 2] && (r = wHas(w, sn - 1))) return r; }
        if (w[sn - 1] == 'i' && (k == 1 || k == 2 || k == 6 || k == 7 || k == 5 || k == 9)) { w[sn - 1] = 'y'; r = wHas(w, sn); w[sn - 1] = 'i'; if (r) return r; }
    }
    return 0;
}
// calibration tables: hit [cross][type][len][bucket], miss [cross][type][len]
#ifdef CALIB
float RH[2][4][10][4], RM[2][4][10];
double CN[2][2][4][10][5];
int g_true = 0;
#else
#include "calib.inc"
#endif
// returns bucket+1 (hit) / 0 (miss) / -1 skip ; cap set if capitalised miss
inline int runHit(const char* s, int n, bool lb, bool rb, bool& cap) {
    cap = false;
    if (n > 24 || n <= 0) return -1;
    char b[32]; int nu = 0; for (int i = 0; i < n; i++) { char c = s[i]; if (isUp(c)) { c += 32; nu++; } b[i] = c; } b[n] = 0;
    if (nu > 1 || (nu == 1 && !isUp(s[0]))) return -1;
    int hit;
    if (lb && rb) hit = inDict(b, n);
    else if (lb) { hit = HT.has(hstr(b, n, SP)); if (!hit) hit = inDict(b, n); }
    else if (rb) { hit = HT.has(hstr(b, n, SS)); if (!hit) hit = inDict(b, n); }
    else { if (n < 3) return -1; hit = 1; for (int i = 0; i + 8 <= max(n, 8); i++) { int m = min(8, n - i); int q = HT.has(hstr(b + i, m, SI)); if (!q) { hit = 0; break; } hit = max(hit, q); if (n <= 8) break; } }
    if (!hit && nu == 1) cap = true;
    return hit;
}
inline float runScore(const char* s, int n, bool lb, bool rb, bool cross) {
    bool cap; int h = runHit(s, n, lb, rb, cap); if (h < 0) return 0;
    int ty = (lb && rb) ? 0 : lb ? 1 : rb ? 2 : 3; int l = min(n, 9);
#ifdef CALIB
    if (!cap) CN[g_true][cross][ty][l][h] += 1;
#endif
    if (h) return RH[cross][ty][l][h - 1];
    return cap ? 0.3f * RM[cross][ty][l] : RM[cross][ty][l];
}
inline float junc(char a, char b, char x, char y) {
    int cb = cls(b), cx = cls(x); float g = 0;
    if (cb < 27 && cx < 27) {
        int ca = a ? cls(a) : 27;
        g = WJ * ((ca < 27 ? lp3[ca][cb][cx] : lp2[cb][cx]) - lp1[cx]);
        if (y && !(x == ' ' && y == ' ')) { int cy = cls(y); if (cy < 27) g += WJ * (lp3[cb][cx][cy] - lp2[cx][cy]); }
    }
    if (isUp(x) && isLo(b)) g -= 6;
    if (b == ' ' && a && (a == '.' || a == '!' || a == '?')) { if (isLo(x)) g -= 4; else if (isUp(x)) g += 2; }
    if (b == ' ' && (x == ',' || x == '.' || x == ';' || x == ':' || x == '!' || x == '?')) g -= 8;
    if ((b == ',' || b == ';' || b == ':' || b == '!' || b == '?') && isAl(x)) g -= 6;
    if (b == '.' && isLo(x)) g -= 4;
    if (isAl(b) && isDg(x)) g -= 4;
    if (isDg(b) && isAl(x)) g -= 2;
    return g;
}
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
        if (s == 0) { char a2 = prev[e - 2]; int ca = cls(a), c2 = cls(a2); if (ca < 26 && c2 < 27) g += WJ * (lp3[c2][ca][26] - lp1[26]); }
        g += junc(a, ' ', x, y);
        int L = 0; while (L < nl && nx[L] != ' ') L++;
        if (L < nl && L + 1 <= s) g -= 12;
    }
    return g;
}
struct Span { short dr, c1, len, tr; char allsp; string txt; };
struct Frag { vector<Span> sp; vector<array<short, 3>> ps; short minc, maxc, h; u64 key; bool startOk; int cells; };
vector<Frag> fr; int NF;
struct St {
    float sc; short W, maxW, r, c, np, maxr, H, ext, ext0; char done; u64 hash;
    char g[MAXH][GW]; u64 used[NWD]; short pl[MAXPL][3];
};
struct Cand { float sc; int par; short type, p, W, mw; };

float firstWord(const St& s, int rr, const char* row, int e, int W, bool rb) {
    const char* prev = s.g[rr - 1];
    if (prev[W - 1] == '-' && isAl(prev[W - 2])) {
        int k = W - 1; while (k > 0 && isAl(prev[k - 1])) k--;
        char b[80]; int n = 0; for (int i = k; i < W - 1; i++) b[n++] = prev[i]; for (int i = 0; i < e; i++) b[n++] = row[i];
        return runScore(b, n, true, rb, true);
    }
    return runScore(row, e, true, rb, false);
}
float runCheck(const St& s, const char* row, int c1, int c2, int rr, int W) {
    float g = 0; int lim = W ? W : GW - 2;
    int p = c1; while (p > 0 && isAl(row[p - 1])) p--;
    while (p <= c2 && p < lim) {
        if (!isAl(row[p])) { p++; continue; }
        int s0 = p, e = p; while (e < lim && isAl(row[e])) e++;
        if (s0 <= c1 || e >= c2) {
            bool lb, rb, lk = true;
            if (s0 > 0) lb = row[s0 - 1] != 0; else { lb = true; lk = (rr == 0) || (rr - 1 < s.r); }
            if (W && e == W) rb = true; else { char ch = row[e]; rb = ch != 0 && !(ch == '-' && ((W && e == W - 1) || (!W && row[e + 1] == 0))); }
            bool cross = (s0 < c1 && e > c1) || (s0 < c2 && e > c2);
            if (lk) { if (s0 == 0 && rr > 0) g += firstWord(s, rr, row, e, W, rb); else g += runScore(row + s0, e - s0, lb, rb, cross); }
        }
        p = e + 1;
    }
    return g;
}
bool scoreFrag(const St& s, int f, int r, int c, float& out, short& mw) {
    const Frag& F = fr[f]; if (r + F.h > maxH || c + F.minc < 0) return false;
    int W = s.W; int lim = W ? W : s.maxW; if (c + F.maxc > lim) return false;
    float g = 0; char buf[GW + 2];
    for (const Span& sp : F.sp) {
        int rr = r + sp.dr, c1 = c + sp.c1, c2 = c1 + sp.len; const char* t = sp.txt.data(); const char* row = s.g[rr];
        for (int k = c1; k < c2; k++) if (row[k]) return false;
        char x = t[0], y = sp.len > 1 ? t[1] : 0; bool ctx = false;
        if (c1 == 0) {
            if (x == ' ') return false;
            if (rr == s.r && rr > 0) g += wrapJ(s.g[rr - 1], W, t, sp.len);
            ctx = (rr == 0) || (rr == s.r);
        } else {
            char b = row[c1 - 1];
            if (b) {
                ctx = true; char a = c1 > 1 ? row[c1 - 2] : 0;
                if (b == ' ') { if (x == ' ') { if (!sp.allsp) return false; } else { if (a == ' ') return false; g += junc(a, b, x, y); } }
                else g += junc(a, b, x, y);
            }
        }
        if (c2 < lim) {
            char R1 = row[c2];
            if (R1) {
                ctx = true; char R2 = (c2 + 1 < lim) ? row[c2 + 1] : 0; char b = t[sp.len - 1], a = sp.len > 1 ? t[sp.len - 2] : (c1 > 0 ? row[c1 - 1] : 0);
                if (b == ' ' && R1 == ' ') { if (R2 && R2 != ' ') return false; }
                else if (b == ' ' && a == ' ') return false;
                else g += junc(a, b, R1, R2);
            }
        }
        int tr = sp.tr;
        if (tr >= 2 && rr < minH - 1) { if (W) { if (W - c2 > 3 - tr) return false; } else mw = min((int)mw, c2 + 3 - tr); }
        if (W && c2 == W) { ctx = true; if (tr > 3 && rr < minH - 1) return false; if (tr == 0 && sp.len >= 2 && t[sp.len - 1] == '-' && isAl(t[sp.len - 2])) g += 1.5f; }
        if (ctx) { memcpy(buf, row, GW); memcpy(buf + c1, t, sp.len); buf[GW] = 0; g += runCheck(s, buf, c1, c2, rr, W); }
    }
    if (!W && mw < max((int)s.ext, c + F.maxc)) return false;
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
        const char* nx = s.g[r + 1];
        int nl = 0; while (nl < W && nx[nl]) nl++;
        s.sc += wrapJ(g, W, nx, nl);
        if (isAl(nx[0])) {
            int e2 = 0; while (e2 < W && isAl(nx[e2])) e2++;
            bool rb = (e2 == W) || (nx[e2] != 0 && !(nx[e2] == '-' && e2 == W - 1));
            s.sc += firstWord(s, r + 1, nx, e2, W, rb);
        }
    }
    return true;
}
inline bool advance(St& s) {
    if (s.W == 0) { int c = s.c; while (c < GW - 1 && s.g[0][c]) c++; s.c = c; return c <= maxWg; }
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
        bool gap = s.ext0 > c; int len = 999;
        if (gap) { int e = c; while (!s.g[0][e]) e++; len = e - c; }
        int t0 = 0; while (t0 < c && s.g[0][c - 1 - t0] == ' ') t0++;
        if (!gap && c + 3 - t0 >= minW) {
            int lo = max(max(minW, c), (int)s.ext), hi = min((int)s.maxW, c + 3 - t0);
            for (int W = lo; W <= hi; W++) {
                float g = 0;
                if (W > c && t0 == 0) g = junc(s.g[0][c - 2], s.g[0][c - 1], ' ', 0);
                out.push_back({s.sc + g, i, 2, 0, (short)W, 0});
            }
        }
        if (t0 >= 2) return;
        for (int f = 0; f < NF; f++) {
            if (s.used[f >> 6] >> (f & 63) & 1) continue;
            const Frag& F = fr[f];
            if (gap) { int rem = len - F.sp[0].len; if (rem < 0 || (rem > 0 && rem < minPW)) continue; }
            float g; short mw = s.maxW;
            if (!scoreFrag(s, f, 0, c, g, mw)) continue;
            out.push_back({s.sc + g, i, 0, (short)f, 0, mw});
        }
    } else {
        int W = s.W; int e = c; while (e < W && !s.g[r][e]) e++;
        int len = e - c;
        if (e == W && c > 0 && (len <= 3 || r >= minH - 1)) {
            char b = s.g[r][c - 1], a = c > 1 ? s.g[r][c - 2] : 0;
            float g = (b != ' ') ? junc(a, b, ' ', 0) : 0;
            out.push_back({s.sc + g, i, 1, 0, 0, 0});
        }
        for (int f = 0; f < NF; f++) {
            if (s.used[f >> 6] >> (f & 63) & 1) continue;
            const Frag& F = fr[f];
            int rem = len - F.sp[0].len;
            if (rem < 0) continue;
            if (rem > 0 && rem < minPW && !(e == W && rem <= 3)) continue;
            float g; short mw = 0;
            if (!scoreFrag(s, f, r, c, g, mw)) continue;
            out.push_back({s.sc + g, i, 0, (short)f, 0, 0});
        }
    }
}
inline bool applyC(St& t, const Cand& cd) {
    t.sc = cd.sc;
    if (cd.type == 0) {
        const Frag& F = fr[cd.p]; int r = t.r, c = t.c;
        for (const Span& sp : F.sp) { memcpy(t.g[r + sp.dr] + c + sp.c1, sp.txt.data(), sp.len); if (sp.dr == 0 && c + sp.c1 + sp.len > t.ext0) t.ext0 = c + sp.c1 + sp.len; }
        t.used[cd.p >> 6] |= 1ULL << (cd.p & 63);
        if (t.np >= MAXPL) return false;
        t.pl[t.np][0] = cd.p; t.pl[t.np][1] = r; t.pl[t.np][2] = c; t.np++;
        if (r + F.h > t.maxr) t.maxr = r + F.h;
        if (c + F.maxc > t.ext) t.ext = c + F.maxc;
        t.hash ^= mix(F.key * 4096 + r * 64 + c);
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
    { Cand cd{0, 0, 0, (short)start, 0, (short)maxWg}; if (!applyC(s0, cd)) { res.valid = false; return res; } }
    vector<Cand> cands; St bestFin; bool haveFin = false; St bestPart = s0; int bestCells = 0;
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
        { int cells = 0; for (int i = 0; i < beam[0].np; i++) cells += fr[beam[0].pl[i][0]].cells; if (cells > bestCells) { bestCells = cells; bestPart = beam[0]; } }
        if ((step & 3) == 3 && elapsed() > TLIM) break;
    }
    const St& f = haveFin ? bestFin : bestPart;
    res.finished = haveFin; res.sc = f.sc; res.np = f.np; res.W = f.W; res.H = f.H;
    for (int i = 0; i < f.np; i++) res.pl.push_back({f.pl[i][0], f.pl[i][1], f.pl[i][2]});
    return res;
}
// pair score: A left, B right, dr = topB - topA
bool pairScore(int A, int B, int dr, float& out) {
    const Piece& a = pc[A]; const Piece& b = pc[B]; float g = 0; int n = 0; char wb[40];
    for (int i = max(0, dr); i < a.h && i - dr < b.h; i++) {
        int j = i - dr; const string& L = a.rows[i]; const string& R = b.rows[j];
        if (a.tr[i] >= 2) return false;
        char lb = L[a.w - 1], la = a.w > 1 ? L[a.w - 2] : 0, x = R[0], y = b.w > 1 ? R[1] : 0;
        if (lb == ' ' && x == ' ') return false;
        g += junc(la, lb, x, y);
        int na = 0; while (na < a.w && isAl(L[a.w - 1 - na])) na++;
        int nb = 0; while (nb < b.w && isAl(R[nb])) nb++;
        bool bL = na < a.w, bR = nb < b.w && !(R[nb] == '-' && nb == b.w - 1);
        if (na + nb > 0 && na + nb < 30) {
            if (na == 0) bL = true; if (nb == 0) bR = !(x == '-' && b.w == 1);
            memcpy(wb, L.data() + a.w - na, na); memcpy(wb + na, R.data(), nb);
            g += runScore(wb, na + nb, bL, bR, na > 0 && nb > 0);
        }
        n++;
    }
    if (!n) return false; out = g; return true;
}
struct Cfg { int A, B, dr; float sc; };
void buildFrags() {
    vector<int> rs(N + 1, 0); for (int i = 0; i < N; i++) rs[i + 1] = rs[i] + pc[i].h;
    int NR = rs[N];
    vector<float> bR(NR, -1e9), sR(NR, -1e9), bL(NR, -1e9), sL(NR, -1e9); vector<int> bRi(NR, -1), bLi(NR, -1);
    vector<Cfg> cf;
    for (int A = 0; A < N; A++) for (int B = 0; B < N; B++) if (A != B) {
        for (int dr = -pc[B].h + 1; dr < pc[A].h; dr++) {
            float sc; if (!pairScore(A, B, dr, sc)) continue;
            if (sc < 1.5f) continue;
            int id = cf.size(); cf.push_back({A, B, dr, sc});
            for (int i = max(0, dr); i < pc[A].h && i - dr < pc[B].h; i++) {
                int ra = rs[A] + i, rb = rs[B] + i - dr;
                if (sc > bR[ra]) { if (bRi[ra] < 0 || !(pc[cf[bRi[ra]].B].cid == pc[B].cid && cf[bRi[ra]].dr == dr)) sR[ra] = bR[ra]; bR[ra] = sc; bRi[ra] = id; }
                else if (sc > sR[ra] && !(pc[cf[bRi[ra]].B].cid == pc[B].cid && cf[bRi[ra]].dr == dr)) sR[ra] = sc;
                if (sc > bL[rb]) { if (bLi[rb] < 0 || !(pc[cf[bLi[rb]].A].cid == pc[A].cid && cf[bLi[rb]].dr == dr)) sL[rb] = bL[rb]; bL[rb] = sc; bLi[rb] = id; }
                else if (sc > sL[rb] && !(pc[cf[bLi[rb]].A].cid == pc[A].cid && cf[bLi[rb]].dr == dr)) sL[rb] = sc;
            }
        }
    }
    vector<int> acc;
    for (int id = 0; id < (int)cf.size(); id++) {
        Cfg& c = cf[id]; if (c.sc < LINK_T) continue; bool ok = true;
        for (int i = max(0, c.dr); ok && i < pc[c.A].h && i - c.dr < pc[c.B].h; i++) {
            int ra = rs[c.A] + i, rb = rs[c.B] + i - c.dr;
            if (bRi[ra] != id || bLi[rb] != id) ok = false;
            else if (c.sc - max(sR[ra], 1.5f) < LINK_D || c.sc - max(sL[rb], 1.5f) < LINK_D) ok = false;
        }
        if (ok) acc.push_back(id);
    }
    sort(acc.begin(), acc.end(), [&](int x, int y) { return cf[x].sc > cf[y].sc; });
    // union with offsets
    vector<int> grp(N); vector<array<int, 2>> pos(N, {0, 0}); vector<vector<int>> mem(N);
    for (int i = 0; i < N; i++) { grp[i] = i; mem[i] = {i}; }
    for (int id : acc) {
        Cfg& c = cf[id]; int ga = grp[c.A], gb = grp[c.B]; if (ga == gb) continue;
        int sr = pos[c.A][0] + c.dr - pos[c.B][0], scc = pos[c.A][1] + pc[c.A].w - pos[c.B][1];
        bool ok = true; int r0 = 1e9, r1 = -1e9, c0 = 1e9, c1 = -1e9;
        for (int q : mem[gb]) { int qr = pos[q][0] + sr, qc = pos[q][1] + scc;
            r0 = min(r0, qr); r1 = max(r1, qr + pc[q].h); c0 = min(c0, qc); c1 = max(c1, qc + pc[q].w);
            for (int p : mem[ga]) { if (qr < pos[p][0] + pc[p].h && pos[p][0] < qr + pc[q].h && qc < pos[p][1] + pc[p].w && pos[p][1] < qc + pc[q].w) { ok = false; break; } }
            if (!ok) break; }
        if (!ok) continue;
        for (int p : mem[ga]) { r0 = min(r0, pos[p][0]); r1 = max(r1, pos[p][0] + pc[p].h); c0 = min(c0, pos[p][1]); c1 = max(c1, pos[p][1] + pc[p].w); }
        if (r1 - r0 > maxH || c1 - c0 > maxWg) continue;
        for (int q : mem[gb]) { pos[q][0] += sr; pos[q][1] += scc; grp[q] = ga; mem[ga].push_back(q); }
        mem[gb].clear();
    }
    fr.clear();
    for (int gi = 0; gi < N; gi++) if (!mem[gi].empty()) {
        Frag F; vector<int>& m = mem[gi];
        int an = m[0]; for (int p : m) if (pos[p][0] < pos[an][0] || (pos[p][0] == pos[an][0] && pos[p][1] < pos[an][1])) an = p;
        int ar = pos[an][0], ac = pos[an][1]; int r1 = 0, c0 = 0, c1 = 0; F.cells = 0;
        map<int, vector<pair<int, int>>> rows; // row -> (col, piece)
        vector<array<int, 3>> key;
        for (int p : m) { int pr = pos[p][0] - ar, pcx = pos[p][1] - ac; F.ps.push_back({(short)p, (short)pr, (short)pcx}); r1 = max(r1, pr + pc[p].h); c0 = min(c0, pcx); c1 = max(c1, pcx + pc[p].w);
            for (int j = 0; j < pc[p].h; j++) rows[pr + j].push_back({pcx, p * 16 + j}); key.push_back({pc[p].cid, pr, pcx}); F.cells += pc[p].h * pc[p].w; }
        sort(key.begin(), key.end()); u64 kh = 1469598103934665603ULL; for (auto& k : key) kh = mix(kh ^ ((u64)k[0] * 1000003 + (k[1] + 20) * 131 + k[2] + 70));
        F.key = kh; F.h = r1; F.minc = c0; F.maxc = c1;
        bool sok = pc[an].up0 && c0 == 0;
        for (auto& kv : rows) {
            auto v = kv.second; sort(v.begin(), v.end());
            size_t i = 0;
            while (i < v.size()) {
                Span sp; sp.dr = kv.first; sp.c1 = v[i].first; sp.txt = "";
                int cur = v[i].first;
                while (i < v.size() && v[i].first == cur) { int p = v[i].second / 16, j = v[i].second % 16; sp.txt += pc[p].rows[j]; cur += pc[p].w; i++; }
                sp.len = sp.txt.size(); int t = 0; while (t < sp.len && sp.txt[sp.len - 1 - t] == ' ') t++; sp.tr = t; sp.allsp = (t == sp.len);
                if (sp.c1 == 0 && sp.txt[0] == ' ') sok = false;
                F.sp.push_back(sp);
            }
        }
        // anchor span must be first
        for (size_t i = 0; i < F.sp.size(); i++) if (F.sp[i].dr == 0 && F.sp[i].c1 == 0) { swap(F.sp[0], F.sp[i]); break; }
        F.startOk = sok; fr.push_back(F);
    }
    NF = fr.size();
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
    buildFrags();
#ifdef LOCAL
    { int big = 0, mx = 0; for (auto& F : fr) { if (F.ps.size() > 1) big++; mx = max(mx, (int)F.ps.size()); } fprintf(stderr, "N %d frags %d multi %d maxsz %d t=%.2f\n", N, NF, big, mx, elapsed()); }
#endif
    vector<vector<int>> ans;
#ifdef LOCAL
    if (getenv("DUMPFRAG")) { for (int f = 0; f < NF; f++) for (auto& q : fr[f].ps) ans.push_back({q[0], f, q[1], q[2]}); return ans; }
#endif
    vector<u64> usedG(NWD, 0);
    vector<int> starts; for (int i = 0; i < NF; i++) if (fr[i].startOk) starts.push_back(i);
    vector<PageRes> cache(NF); for (auto& c : cache) c.valid = false;
    vector<char> tried(NF, 0);
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
        for (auto& a : R.pl) { for (auto& q : fr[a[0]].ps) ans.push_back({q[0], page, a[1] + q[1], a[2] + q[2]}); usedG[a[0] >> 6] |= 1ULL << (a[0] & 63); }
#ifdef LOCAL
        fprintf(stderr, "page %d start %d sc %.1f fin %d np %d W %d H %d t=%.2f\n", page, best, R.sc, (int)R.finished, R.np, R.W, R.H, elapsed());
#endif
        page++; R.valid = false;
    }
    return ans;
}
} // namespace DL
