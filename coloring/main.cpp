#include <vector>
#include <iostream>
#include <algorithm>
#include <numeric>
#include <random>
using namespace std;

pair<int, vector<int>> solve_greedy(int n, const vector<vector<int>>& e, const vector<int>& order) {
    vector<int> color(n, -1);
    vector<int> used(n, -1);
    int cnt = 0;
    for (auto v : order) {
        for (auto u : e[v]) {
            if (color[u] != -1) used[color[u]] = v;
        }
        int c = 0;
        while (used[c] == v) ++c;
        color[v] = c;
        cnt = max(cnt, c + 1);
    }
    return {cnt, color};
}

pair<int, vector<int>> solve_dsatur(int n, const vector<vector<int>>& e) {
    vector<int> color(n, -1);
    vector<int> saturation(n, 0);
    vector<int> uncolored(n);
    vector<vector<bool>> seen(n, vector<bool>(n, 0));
    for (int v = 0; v < n; ++v) uncolored[v] = e[v].size();
    int cnt = 0;

    for (int i = 0; i < n; ++i) {
        int best = -1;
        for (int v = 0; v < n; ++v) {
            if (color[v] != -1) continue;
            if (best == -1 || saturation[v] > saturation[best] ||
                (saturation[v] == saturation[best] && uncolored[v] > uncolored[best])) {
                best = v;
            }
        } // выбираем у кого больше всего ограничений

        int c = 0;
        while (seen[best][c]) ++c;
        color[best] = c;
        cnt = max(cnt, c + 1);
        for (auto u : e[best]) {
            if (color[u] != -1) continue;
            --uncolored[u];
            if (!seen[u][c]) {
                seen[u][c] = true;
                ++saturation[u];
            }
        }
    }
    return {cnt, color};
}

pair<int, vector<int>> solve_local_search(int n, const vector<vector<int>>& e, pair<int, vector<int>> ans) {
    mt19937 rand(123);
    for (int it = 0; it < 2000; ++it) {
        vector<vector<int>> classes(ans.first);
        for (int v = 0; v < n; ++v) classes[ans.second[v]].push_back(v);

        vector<int> class_order(ans.first);
        for (int c = 0; c < ans.first; ++c) {
            class_order[c] = c;
        }
        if (it % 3 == 0) {
            vector<pair<int, int>> sizes;
            for (int c = 0; c < ans.first; ++c) {
                sizes.push_back({-classes[c].size(), c});
            }
            sort(sizes.begin(), sizes.end());
            for (int c = 0; c < ans.first; ++c) class_order[c] = sizes[c].second;
        } else if (it % 3 == 1) {
            reverse(class_order.begin(), class_order.end());
        } else {
            shuffle(class_order.begin(), class_order.end(), rand);
        }
        vector<int> order;
        for (auto c : class_order) {
            for (auto v : classes[c]) order.push_back(v);
        }
        auto next = solve_greedy(n, e, order);
        if (next.first <= ans.first) ans = next;
    }
    return ans;
}

int main() {

    int n, m;
    cin >> n >> m;
    vector<vector<int>> e(n);
    for (int i = 0; i < m; ++i) {
        int u, v;
        cin >> u >> v;
        e[u].push_back(v);
        e[v].push_back(u);
    }
    vector<int> order(n);
    for (int i = 0; i < n; ++i) {
        order[i] = i;
    }
    auto ans = solve_greedy(n, e, order);
    auto ans_dsatur = solve_dsatur(n, e);
    if (ans_dsatur.first < ans.first) ans = ans_dsatur;
    ans = solve_local_search(n, e, ans);

    cout << ans.first << endl;
    for (auto c : ans.second) cout << c << " ";
    cout << endl;
    return 0;
}
