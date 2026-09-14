#include <vector>
#include <iostream>
#include <cmath>
#include <iomanip>
#include <algorithm>
#include <chrono>
#include <random>
using namespace std;

struct Point {
    double x;
    double y;
};

double distance2(const Point& a, const Point& b) {
    return (a.x - b.x) * (a.x - b.x) + (a.y - b.y) * (a.y - b.y);
}

double path_length(const vector<Point>& points, const vector<int>& path) {
    double length = 0;
    int n = path.size();
    for (int i = 0; i < n; ++i) {
        length += sqrt(distance2(points[path[i]], points[path[(i + 1) % n]]));
    }
    return length;
}

pair<double, vector<int>> solve_greedy(int n, const vector<Point>& points) {
    vector<bool> used(n, false);
    vector<int> path;
    int v = 0;
    for (int i = 0; i < n; ++i) {
        path.push_back(v);
        used[v] = true;
        if (i == n - 1) break;

        int best = -1;
        double best_dist = 0;
        for (int u = 0; u < n; ++u) {
            if (used[u]) continue;
            double dist = distance2(points[v], points[u]);
            if (best == -1 || dist < best_dist) {
                best = u;
                best_dist = dist;
            }
        }
        v = best;
    }
    return {path_length(points, path), path};
}

pair<double, vector<int>> solve_local_search(int n, const vector<Point>& points, pair<double, vector<int>> ans) {
    vector<int> path = ans.second;
    auto start = chrono::steady_clock::now();
    int checked = 0;
    bool improved = true;
    while (improved) {
        improved = false;
        for (int i = 0; i < n - 2; ++i) {
            for (int j = i + 2; j < n; ++j) {
                if (i == 0 && j == n - 1) {
                    continue;
                }
                if (++checked % 100000 == 0 && chrono::steady_clock::now() - start >= chrono::seconds(60)) {
                    return {path_length(points, path), path};
                }
                int a = path[i];
                int b = path[i + 1];
                int c = path[j];
                int d = path[(j + 1) % n];
                double delta = sqrt(distance2(points[a], points[c])) + sqrt(distance2(points[b], points[d]))
                             - sqrt(distance2(points[a], points[b])) - sqrt(distance2(points[c], points[d]));
                if (delta < -1e-9) {
                    reverse(path.begin() + i + 1, path.begin() + j + 1); // замена двух ребер ~ реверс подпути
                    improved = true;
                    break;
                }
            }
        }
    }
    return {path_length(points, path), path};
}

vector<vector<int>> build_candidates(int n, const vector<Point>& points) {
    int limit = min(40, n - 1);
    vector<vector<int>> near(n);
    vector<pair<double, int>> distances;
    distances.reserve(n - 1);
    for (int v = 0; v < n; ++v) {
        distances.clear();
        for (int u = 0; u < n; ++u) {
            if (u != v) distances.push_back({distance2(points[v], points[u]), u});
        }
        if (limit < n - 1) nth_element(distances.begin(), distances.begin() + limit, distances.end());
        sort(distances.begin(), distances.begin() + limit);
        for (int i = 0; i < limit; ++i) near[v].push_back(distances[i].second);
    }
    return near;
}

pair<double, vector<int>> improve_candidates(int n, const vector<Point>& points, pair<double, vector<int>> ans, const vector<vector<int>>& near, chrono::steady_clock::time_point start) {
    vector<int> path = ans.second;
    vector<int> pos(n);
    for (int i = 0; i < n; ++i) pos[path[i]] = i;
    long long checked = 0;
    bool improved = true;
    while (improved) {
        improved = false;
        for (int i = 0; i < n; ++i) {
            int v = path[i];
            int best_left = -1;
            int best_right = -1;
            double best_delta = -1e-9;
            for (auto u : near[v]) {
                if (++checked % 10000 == 0 && chrono::steady_clock::now() - start >= chrono::seconds(40)) {
                    return {path_length(points, path), path};
                }
                int left = min(i, pos[u]);
                int right = max(i, pos[u]);
                if (right - left < 2 || (left == 0 && right == n - 1)) continue;

                int a = path[left], b = path[left + 1];
                int c = path[right], d = path[(right + 1) % n];
                double delta = sqrt(distance2(points[a], points[c])) + sqrt(distance2(points[b], points[d]))
                             - sqrt(distance2(points[a], points[b])) - sqrt(distance2(points[c], points[d]));
                if (delta < best_delta) {
                    best_delta = delta;
                    best_left = left;
                    best_right = right;
                }
            }
            if (best_left != -1) {
                reverse(path.begin() + best_left + 1, path.begin() + best_right + 1);
                for (int j = best_left + 1; j <= best_right; ++j) pos[path[j]] = j;
                improved = true;
            }
        }
    }
    return {path_length(points, path), path};
}

pair<double, vector<int>> improve_or_opt(int n, const vector<Point>& points, pair<double, vector<int>> ans, const vector<vector<int>>& near, chrono::steady_clock::time_point start) {
    vector<int> next(n), prev(n); // храним предыдущую и следующую точку, чтобы переносить участок без сдвига всего массива
    for (int i = 0; i < n; ++i) {
        int v = ans.second[i], u = ans.second[(i + 1) % n];
        next[v] = u;
        prev[u] = v;
    }
    bool improved = true;
    while (improved && chrono::steady_clock::now() - start < chrono::seconds(40)) {
        improved = false;
        for (int v = 0; v < n; ++v) {
            if (chrono::steady_clock::now() - start >= chrono::seconds(40)) break;
            int a = prev[v];
            int last = v;
            bool moved = false;
            vector<int> block;
            for (int length = 1; length <= 3 && length < n && !moved; ++length) {
                block.push_back(last);
                int b = next[last];
                vector<int> places = near[v];
                for (auto u : near[last]) places.push_back(prev[u]);
                for (auto c : places) {
                    if (c == a || c == v || c == last || (length == 3 && c == next[v])) continue;
                    int d = next[c];
                    double delta = sqrt(distance2(points[a], points[b])) + sqrt(distance2(points[c], points[v]))
                                 + sqrt(distance2(points[last], points[d])) - sqrt(distance2(points[a], points[v]))
                                 - sqrt(distance2(points[last], points[b])) - sqrt(distance2(points[c], points[d]));
                    double reversed = delta - sqrt(distance2(points[c], points[v])) - sqrt(distance2(points[last], points[d]))
                                            + sqrt(distance2(points[c], points[last])) + sqrt(distance2(points[v], points[d]));
                    bool flip = reversed < delta;
                    if (flip) delta = reversed;
                    if (delta >= -1e-9) continue;

                    next[a] = b;
                    prev[b] = a;
                    if (flip) reverse(block.begin(), block.end());
                    int at = c;
                    for (auto u : block) {
                        next[at] = u;
                        prev[u] = at;
                        at = u;
                    }
                    next[at] = d;
                    prev[d] = at;
                    moved = true;
                    improved = true;
                    break;
                }
                last = next[last];
            }
        }
    }
    vector<int> path;
    int v = 0;
    for (int i = 0; i < n; ++i) {
        path.push_back(v);
        v = next[v];
    }
    return {path_length(points, path), path};
}

pair<double, vector<int>> improve_route(int n, const vector<Point>& points, pair<double, vector<int>> ans, const vector<vector<int>>& near, chrono::steady_clock::time_point start) {
    while (chrono::steady_clock::now() - start < chrono::seconds(40)) {
        double old = ans.first;
        ans = improve_candidates(n, points, ans, near, start);
        ans = improve_or_opt(n, points, ans, near, start);
        if (ans.first >= old - 1e-9) break;
    }
    return ans;
}

pair<double, vector<int>> solve_iterated(int n, const vector<Point>& points, pair<double, vector<int>> ans) {
    if (n < 4) return ans;
    auto start = chrono::steady_clock::now();
    auto near = build_candidates(n, points);
    ans = improve_route(n, points, ans, near, start);
    mt19937 rand(123123);
    for (int it = 0; it < 300; ++it) {
        if (chrono::steady_clock::now() - start >= chrono::seconds(40)) break;
        vector<int> cuts;
        while (cuts.size() < 3) {
            int cut = 1 + rand() % (n - 1);
            if (find(cuts.begin(), cuts.end(), cut) == cuts.end()) {
                cuts.push_back(cut);
            }
        }
        sort(cuts.begin(), cuts.end());
        int a = cuts[0], b = cuts[1], c = cuts[2];
        vector<int> path;
        path.insert(path.end(), ans.second.begin(), ans.second.begin() + a);
        path.insert(path.end(), ans.second.begin() + c, ans.second.end());
        path.insert(path.end(), ans.second.begin() + b, ans.second.begin() + c);
        path.insert(path.end(), ans.second.begin() + a, ans.second.begin() + b);

        auto next = improve_route(n, points, {path_length(points, path), path}, near, start);
        if (next.first < ans.first) ans = next;
    }
    return ans;
}

int main() {
    int n;
    cin >> n;
    vector<Point> points(n);
    for (int i = 0; i < n; ++i) cin >> points[i].x >> points[i].y;

    auto ans = solve_greedy(n, points);
    if (n <= 2000) ans = solve_local_search(n, points, ans);
    ans = solve_iterated(n, points, ans);
    cout << fixed << setprecision(6) << ans.first << endl;
    for (auto v : ans.second) cout << v << " ";
    cout << endl;
    return 0;
}
