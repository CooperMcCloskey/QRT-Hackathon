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
