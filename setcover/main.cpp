#include <vector>
#include <iostream>
#include <sstream>
#include <string>
#include <algorithm>
#include <random>
using namespace std;

pair<long long, vector<short>> solve_greedy(int n, int m, const vector<long long>& costs, const vector<vector<int>>& sets) {
    vector<bool> covered(n, false);
    vector<short> chosen(m, 0);
    long long total_cost = 0;
    int uncovered = n;
    while (uncovered > 0) {
        int best = -1;
        int best_gain = 0;
        for (int i = 0; i < m; ++i) {
            if (chosen[i]) continue;
            int gain = 0;
            for (auto element : sets[i]) {
                if (!covered[element]) ++gain;
            }
            if (gain > 0 && (best == -1 || costs[i] * best_gain < costs[best] * gain)) {
                best = i;
                best_gain = gain;
            }
        }
        if (best == -1) break;
        chosen[best] = 1;
        total_cost += costs[best];
        for (auto element : sets[best]) {
            if (!covered[element]) {
                covered[element] = 1;
                --uncovered;
            }
        }
    }
    return {total_cost, chosen};
}

pair<long long, vector<short>> reverse_delete(pair<long long, vector<short>> ans, int n, int m, const vector<long long>& costs, const vector<vector<int>>& sets) {
    long long total_cost = ans.first;
    vector<short> chosen = ans.second;
    vector<int> cover_cnt(n, 0);
    vector<pair<long long, int>> order;
    for (int i = 0; i < m; ++i) {
        if (!chosen[i]) continue;
        order.push_back( {-costs[i], i} );
        for (auto item : sets[i]) {
            ++cover_cnt[item];
        }
    }
    sort(order.begin(), order.end());
    for (auto p : order) {
        int idx = p.second;
        bool rm = true;
        for (auto item : sets[idx]) {
            if (cover_cnt[item] <= 1) {
                rm = false;
                break;
            }
        }
        if (!rm) continue;
        chosen[idx] = 0;
        total_cost -= costs[idx];
        for (auto item : sets[idx]) {
            --cover_cnt[item];
        }
    }
    return {total_cost, chosen};
}

pair<long long, vector<short>> solve_reverse_delete(int n, int m, const vector<long long>& costs, const vector<vector<int>>& sets) {
    return reverse_delete(solve_greedy(n, m, costs, sets), n, m, costs, sets);
}

pair<long long, vector<short>> local_search(pair<long long, vector<short>> ans, int n, int m, const vector<long long>& costs, const vector<vector<int>>& sets) {
    long long total_cost = ans.first;
    vector<short> chosen = ans.second;
    vector<int> cover_cnt(n, 0);
    vector<vector<int>> containing(n);
    for (int i = 0; i < m; ++i) {
        for (auto item : sets[i]) {
            containing[item].push_back(i);
            if (chosen[i]) ++cover_cnt[item];
        }
    }

    vector<int> removed_cnt(n, 0);
    vector<int> candidate_cnt(m, 0);
    bool improved = true;
    while (improved) {
        improved = false;
        for (int first = 0; first < m && !improved; ++first) {
            if (!chosen[first]) continue;
            for (int second = first + 1; second < m && !improved; ++second) {
                if (!chosen[second]) continue;
                vector<int> used_items;
                vector<int> lost_items;
                for (auto item : sets[first]) {
                    if (removed_cnt[item]++ == 0) used_items.push_back(item);
                }
                for (auto item : sets[second]) {
                    if (removed_cnt[item]++ == 0) used_items.push_back(item);
                }
                for (auto item : used_items) {
                    if (cover_cnt[item] == removed_cnt[item]) lost_items.push_back(item);
                }

                vector<int> candidates;
                for (auto item : lost_items) {
                    for (auto idx : containing[item]) {
                        if (candidate_cnt[idx]++ == 0) candidates.push_back(idx);
                    }
                }
                int add = -1;
                for (auto idx : candidates) {
                    if (!chosen[idx] && candidate_cnt[idx] == lost_items.size() &&
                                    costs[idx] < costs[first] + costs[second] &&
                                    (add == -1 || costs[idx] < costs[add])) {
                        add = idx;
                    }
                }
                for (auto item : used_items) removed_cnt[item] = 0;
                for (auto idx : candidates) candidate_cnt[idx] = 0;
                if (add == -1) continue;
                chosen[first] = 0;
                chosen[second] = 0;
                chosen[add] = 1;
                total_cost += costs[add] - costs[first] - costs[second];
                for (auto item : sets[first]) --cover_cnt[item];
                for (auto item : sets[second]) --cover_cnt[item];
                for (auto item : sets[add]) ++cover_cnt[item];
                improved = true;
            }
        }
    }
    return {total_cost, chosen};
}

pair<long long, vector<short>> solve_local_search(int n, int m, const vector<long long>& costs, const vector<vector<int>>& sets) {
    auto ans = solve_reverse_delete(n, m, costs, sets);
    return local_search(ans, n, m, costs, sets);
}

pair<long long, vector<short>> randomized_greedy(int n, int m, const vector<long long>& costs, const vector<vector<int>>& sets, int candidate_percent, mt19937& generator) {
    vector<bool> covered(n, false);
    vector<short> chosen(m, 0);
    vector<int> gains(m, 0);
    long long total_cost = 0;
    int uncovered = n;

    while (uncovered > 0) {
        int best = -1;
        int best_gain = 0;
        for (int i = 0; i < m; ++i) {
            if (chosen[i]) continue;
            gains[i] = 0;
            for (auto item : sets[i]) {
                if (!covered[item]) ++gains[i];
            }
            if (gains[i] > 0 && (best == -1 || costs[i] * best_gain < costs[best] * gains[i])) {
                best = i;
                best_gain = gains[i];
            }
        }

        vector<int> candidates;
        for (int i = 0; i < m; ++i) {
            if (!chosen[i] && gains[i] > 0 &&
                costs[i] * best_gain * 100 <= costs[best] * gains[i] * candidate_percent) {
                candidates.push_back(i);
            }
        }

        uniform_int_distribution<int> distribution(0, candidates.size() - 1);
        int add = candidates[distribution(generator)];
        chosen[add] = 1;
        total_cost += costs[add];
        for (auto item : sets[add]) {
            if (!covered[item]) {
                covered[item] = true;
                --uncovered;
            }
        }
    }

    return {total_cost, chosen};
}

pair<long long, vector<short>> solve_grasp(int n, int m, const vector<long long>& costs, const vector<vector<int>>& sets) {
    auto best = solve_reverse_delete(n, m, costs, sets);
    mt19937 generator(111111);
    vector<int> candidate_percents = {105, 115, 130};
    for (auto candidate_percent : candidate_percents) {
        int iters = 100;
        while (iters--) {
            auto ans = randomized_greedy(n, m, costs, sets, candidate_percent, generator);
            ans = reverse_delete(ans, n, m, costs, sets);
            ans = local_search(ans, n, m, costs, sets);
            if (ans.first < best.first) best = ans;
        }
    }

    return local_search(best, n, m, costs, sets);
}

int main() {
    int n, m;
    cin >> n >> m;
    string ln;
    getline(cin, ln);
    vector<vector<int>> sets(m);
    vector<long long> costs(m);
    for (int i = 0; i < m; ++i) {
        getline(cin, ln);
        stringstream input(ln);
        input >> costs[i];
        int item;
        while (input >> item) {
            if (item >= 0) sets[i].push_back(item);
        }
    }
    auto ans_greedy = solve_greedy(n, m, costs, sets);
    auto ans_reverse_delete = solve_reverse_delete(n, m, costs, sets);
    auto ans_local_search = solve_local_search(n, m, costs, sets);
    auto ans_grasp = solve_grasp(n, m, costs, sets);
    auto ans = ans_greedy;
    if (ans_reverse_delete.first < ans.first) {
        ans = ans_reverse_delete;
    }
    if (ans_local_search.first < ans.first) {
        ans = ans_local_search;
    }
    if (ans_grasp.first < ans.first) {
        ans = ans_grasp;
    }
    cout << ans.first << endl;
    for (auto val : ans.second) {
        cout << val << " ";
    }
    cout << endl;
    return 0;
}
