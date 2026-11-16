// Timed simulated annealing skeleton (C++17).
//
// Runs as-is on a toy problem: order numbers to minimise the sum of gaps
// between neighbours. Replace the parts marked REPLACE. Keep the time guard
// and the "best answer so far" logic.
//
// Build: g++ -O2 -std=c++17 -o sa sa.cpp
// Uses explicit headers because <bits/stdc++.h> does not exist on macOS.
#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <numeric>
#include <vector>
using namespace std;

static const double TIME_LIMIT = 2.0;                  // REPLACE: judge limit, seconds
static const double TIME_BUDGET = 0.75 * TIME_LIMIT;   // stop here; the judge may be slower

static const chrono::steady_clock::time_point T0 = chrono::steady_clock::now();
static double elapsed() {
    return chrono::duration<double>(chrono::steady_clock::now() - T0).count();
}

// Fast fixed-seed random numbers, so results repeat.
static uint64_t rng_state = 88172645463325252ULL;
static inline uint64_t rnd() {
    rng_state ^= rng_state << 7;
    rng_state ^= rng_state >> 9;
    return rng_state;
}
static inline int rnd_int(int n) { return (int)(rnd() % (uint64_t)n); }
static inline double rnd01() { return (double)(rnd() >> 11) * (1.0 / 9007199254740992.0); }

// ---- REPLACE: input, state, cost, output ----
static int n;
static vector<long long> a;
typedef vector<int> State;

static void read_input() {
    cin >> n;
    a.resize(n);
    for (auto& x : a) cin >> x;
}

static State initial_state() {   // any valid answer (the baseline)
    State s(n);
    iota(s.begin(), s.end(), 0);
    return s;
}

static double cost(const State& s) {   // lower is better
    double c = 0;
    for (int i = 0; i + 1 < n; i++) c += (double)llabs(a[s[i]] - a[s[i + 1]]);
    return c;
}

static void write_output(const State& s) {   // exact format from the statement
    for (int i = 0; i < n; i++) cout << a[s[i]] << (i + 1 < n ? ' ' : '\n');
}
// ---- end REPLACE ----

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    read_input();

    State cur = initial_state();
    if (n < 2) {
        write_output(cur);
        return 0;
    }
    double cur_cost = cost(cur);
    State best = cur;
    double best_cost = cur_cost;

    const double T_START = max(1.0, cur_cost / n), T_END = 1e-3;   // REPLACE: tune
    double temp = T_START;
    long long it = 0;
    while (true) {
        if ((it & 255) == 0) {
            double frac = elapsed() / TIME_BUDGET;
            if (frac >= 1.0) break;
            temp = T_START * pow(T_END / T_START, frac);
        }
        ++it;

        // REPLACE: one small random change, and a way to undo it.
        int i = rnd_int(n), j = rnd_int(n);
        if (i == j) continue;
        swap(cur[i], cur[j]);
        double new_cost = cost(cur);   // REPLACE with a delta if cost() is slow

        double delta = new_cost - cur_cost;
        if (delta <= 0 || rnd01() < exp(-delta / temp)) {
            cur_cost = new_cost;
            if (cur_cost < best_cost) {
                best_cost = cur_cost;
                best = cur;
            }
        } else {
            swap(cur[i], cur[j]);   // undo
        }
    }

    write_output(best);
    cerr << "iters=" << it << " best=" << best_cost << "\n";
    return 0;
}
