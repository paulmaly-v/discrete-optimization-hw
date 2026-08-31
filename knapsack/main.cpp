#include <vector>
#include <iostream>
#include <algorithm>

struct Unit {
    long long v;
    long long w;
    int idx;
    friend bool operator<(const Unit& a, const Unit& b) {
        return a.v * b.w == b.v * a.w ? a.idx > b.idx : a.v * b.w > b.v * a.w; 
    }
};

using namespace std;
pair<long long, vector<short>> solve_greedy(const int n, const vector<long long>& v, const vector<long long>& w, const long long max_w) {
    vector<Unit> units(n);
    for (int i = 0; i < n; ++i) {
        units[i] = {v[i], w[i], i};
    }
    sort(units.begin(), units.end());
    long long cur_w = 0;
    long long cur_v = 0;
    vector<short> chosen(n, 0);
    for (int i = 0; i < n; ++i) {
        if (cur_w + units[i].w <= max_w) {
            cur_w += units[i].w;
            cur_v += units[i].v;
            chosen[units[i].idx] = 1;
        }
    }
    return {cur_v, chosen};
}

pair<long long, vector<short>> solve_dp(const int n, const vector<long long>& v, const vector<long long>& w, const long long max_w) {
    if (n*max_w > 120000000) return {-1, {}};

    vector<long long> dp(max_w + 1, 0);
    vector<vector<bool>> take(n, vector<bool>(max_w + 1, false));

    for (int i = 0; i < n; ++i) {
        for (int cur_w = max_w; cur_w >= w[i]; --cur_w) {
            long long new_v = dp[cur_w - w[i]] + v[i];
            if (new_v > dp[cur_w]) {
                dp[cur_w] = new_v;
                take[i][cur_w] = true;
            }
        }
    }

    vector<short> chosen(n, 0);
    int cur_w = max_w;
    for (int i = n - 1; i >= 0; --i) {
        if (take[i][cur_w]) {
            chosen[i] = 1;
            cur_w -= w[i];
        }
    }

    return {dp[max_w], chosen};
}

const long long max_bb_nodes = 1000000;

long long bb_upper_bound(const vector<Unit>& units, int pos, long long cur_w, long long cur_v, long long max_w) {
    long long remaining = max_w - cur_w;
    long long bound = cur_v;
    for (int i = pos; i < units.size(); ++i) {
        if (units[i].w <= remaining) {
            remaining -= units[i].w;
            bound += units[i].v;
        } else {
            bound += (units[i].v * remaining + units[i].w - 1) / units[i].w;
            break;
        }
    }
    return bound;
}

void bb_dfs(const vector<Unit>& units, int pos, long long cur_w, long long cur_v, long long max_w,
            long long& nodes, long long& best_v, vector<short>& chosen, vector<short>& best_chosen) {
    if (++nodes > max_bb_nodes) return;
    if (cur_v > best_v) {
        best_v = cur_v;
        best_chosen = chosen;
    }
    if (pos == units.size() || bb_upper_bound(units, pos, cur_w, cur_v, max_w) <= best_v) return;

    if (cur_w + units[pos].w <= max_w) {
        chosen[units[pos].idx] = 1;
        bb_dfs(units, pos + 1, cur_w + units[pos].w, cur_v + units[pos].v,
               max_w, nodes, best_v, chosen, best_chosen);
        chosen[units[pos].idx] = 0;
    }
    bb_dfs(units, pos + 1, cur_w, cur_v, max_w,
           nodes, best_v, chosen, best_chosen);
}

pair<long long, vector<short>> solve_bb(const int n, const vector<long long>& v, const vector<long long>& w, const long long max_w) {
    vector<Unit> units(n);
    for (int i = 0; i < n; ++i) {
        units[i] = {v[i], w[i], i};
    }
    sort(units.begin(), units.end());
    auto greedy = solve_greedy(n, v, w, max_w);
    long long best_v = greedy.first;
    vector<short> best_chosen = greedy.second;
    vector<short> chosen(n, 0);
    long long nodes = 0;

    bb_dfs(units, 0, 0, 0, max_w, nodes, best_v, chosen, best_chosen);

    return {best_v, best_chosen};
}

int main() {
    int n;
    long long max_w;
    cin >> n >> max_w;
    vector<long long> w(n);
    vector<long long> v(n);
    for (int i = 0; i < n; ++i) {
        cin >> v[i] >> w[i];
    }
    using Solution = pair<long long, vector<short>>;
    Solution ans;
    Solution ans_greedy;
    Solution ans_dp;
    Solution ans_bb;
    ans = ans_greedy = solve_greedy(n, v, w, max_w);
    ans_dp = solve_dp(n, v, w, max_w);
    ans_bb = ans_dp.first == -1 ? solve_bb(n, v, w, max_w) : Solution{-1, {}};
    if (ans_dp.first > ans.first) {
        ans = ans_dp;
    }
    if (ans_bb.first > ans.first) {
        ans = ans_bb;
    }
    cout << ans.first << endl;
    for (int i = 0; i < n; ++i) {
        cout << ans.second[i] << " ";
    }
    cout << endl;
}
