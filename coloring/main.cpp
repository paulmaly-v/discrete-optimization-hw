#include <vector>
#include <iostream>
#include <algorithm>
#include <numeric>
#include <random>
#include <chrono>
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

pair<int, vector<int>> solve_tabucol(int n, const vector<vector<int>>& e, pair<int, vector<int>> ans) {
    mt19937 rand(123);
    auto start = chrono::steady_clock::now();
    while (ans.first > 1) {
        int k = ans.first - 1;
        vector<int> sizes(ans.first, 0);
        for (auto c : ans.second) ++sizes[c];
        int removed = min_element(sizes.begin(), sizes.end()) - sizes.begin();
        vector<int> color = ans.second;
        vector<int> order;
        for (int v = 0; v < n; ++v) {
            if (color[v] == removed) {
                color[v] = -1;
                order.push_back(v);
            } else if (color[v] == k) {
                color[v] = removed;
            }
        }
        shuffle(order.begin(), order.end(), rand);
        vector<vector<int>> count(n, vector<int>(k, 0));
        vector<vector<int>> tabu(n, vector<int>(k, 0));
        for (int v = 0; v < n; ++v) {
            if (color[v] == -1) continue;
            for (auto u : e[v]) ++count[u][color[v]];
        }
        for (auto v : order) {
            int c = min_element(count[v].begin(), count[v].end()) - count[v].begin();
            color[v] = c;
            for (auto u : e[v]) ++count[u][c];
        }

        int conflicts = 0;
        for (int i = 0; i < n; ++i) conflicts += count[i][color[i]];
        conflicts /= 2;
        int best = conflicts;
        int last = 0;
        for (int it = 1; conflicts > 0 && it <= 100000 && it - last <= 10000; ++it) {
            if (it % 256 == 1 && chrono::steady_clock::now() - start >= chrono::seconds(40)) return ans;
            int v = -1;
            int next = -1;
            int delta = 0;
            int ties = 0;
            for (int i = 0; i < n; ++i) {
                if (count[i][color[i]] == 0) continue;
                for (int c = 0; c < k; ++c) {
                    if (c == color[i]) continue;
                    int change = count[i][c] - count[i][color[i]];
                    if (tabu[i][c] > it && conflicts + change >= best) continue;
                    if (v == -1 || change < delta) {
                        v = i;
                        next = c;
                        delta = change;
                        ties = 1;
                    } else if (change == delta) {
                        ++ties;
                        if (rand() % ties == 0) {
                            v = i;
                            next = c;
                        }
                    }
                }
            }
            if (v == -1) break;
            int old = color[v];
            tabu[v][old] = it + 1 + 6 * conflicts / 10 + rand() % 10;
            conflicts += delta;
            color[v] = next;
            for (auto u : e[v]) {
                ++count[u][next];
                --count[u][old];
            }
            if (conflicts < best) {
                best = conflicts;
                last = it;
            }
        }
        if (conflicts != 0) { 
            break;
        }
        vector<int> names(k, -1);
        int cnt = 0;
        for (auto& c : color) {
            if (names[c] == -1) {
                names[c] = cnt++;
            }
            c = names[c];
        }
        ans = {cnt, color};
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
    ans = solve_tabucol(n, e, ans);
    cout << ans.first << endl;
    for (auto c : ans.second) cout << c << " ";
    cout << endl;
    return 0;
}