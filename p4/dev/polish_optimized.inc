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
