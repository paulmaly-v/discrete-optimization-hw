#include <vector>
#include <iostream>
#include <cmath>
#include <iomanip>
#include <algorithm>
#include <chrono>
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

int main() {
    int n;
    cin >> n;
    vector<Point> points(n);
    for (int i = 0; i < n; ++i) cin >> points[i].x >> points[i].y;

    auto ans = solve_greedy(n, points);
    ans = solve_local_search(n, points, ans);
    cout << fixed << setprecision(6) << ans.first << endl;
    for (auto v : ans.second) cout << v << " ";
    cout << endl;
    return 0;
}
