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
    ans = ans_greedy = solve_greedy(n, v, w, max_w);
    ans_dp = solve_dp(n, v, w, max_w);
    if (ans_dp.first > ans_greedy.first) {
        ans = ans_dp;
    }
    cout << ans.first << endl;
    for (int i = 0; i < n; ++i) {
        cout << ans.second[i] << " ";
    }
    cout << endl;
}
