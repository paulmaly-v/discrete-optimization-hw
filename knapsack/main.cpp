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

int main() {
    int n;
    long long max_w;
    cin >> n >> max_w;
    vector<long long> w(n);
    vector<long long> v(n);
    for (int i = 0; i < n; ++i) {
        cin >> v[i] >> w[i];
    }
    pair<long long, vector<short>> ans;
    ans = solve_greedy(n, v, w, max_w);
    cout << ans.first << endl;
    for (int i = 0; i < n; ++i) {
        cout << ans.second[i] << " ";
    }
    cout << endl;
}