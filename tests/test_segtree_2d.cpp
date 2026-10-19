#include "../data_structure/segtree_2d.hpp"

using namespace std;

namespace {

long long sum_op(long long a, long long b) { return a + b; }
long long sum_e() { return 0LL; }

long long min_op(long long a, long long b) { return min(a, b); }
long long min_e() { return (1LL << 60); }

using StaticSumSegTree = StaticSegTree2D<long long, sum_op, sum_e>;
using StaticMinSegTree = StaticSegTree2D<long long, min_op, min_e>;
using DenseSumSegTree = DenseSegTree2D<long long, sum_op, sum_e>;
using DenseMinSegTree = DenseSegTree2D<long long, min_op, min_e>;

vector<pair<long long, long long>> unique_points(const vector<pair<long long, long long>>& points) {
    vector<pair<long long, long long>> res = points;
    sort(res.begin(), res.end());
    res.erase(unique(res.begin(), res.end()), res.end());
    return res;
}

long long static_sum_naive_prod(
    const vector<pair<long long, long long>>& points,
    const vector<long long>& values,
    long long xl,
    long long xr,
    long long yl,
    long long yr
) {
    long long ans = 0;
    for (int i = 0; i < static_cast<int>(points.size()); ++i) {
        auto [x, y] = points[i];
        if (xl <= x && x < xr && yl <= y && y < yr) ans += values[i];
    }
    return ans;
}

long long static_min_naive_prod(
    const vector<pair<long long, long long>>& points,
    const vector<long long>& values,
    long long xl,
    long long xr,
    long long yl,
    long long yr
) {
    long long ans = min_e();
    for (int i = 0; i < static_cast<int>(points.size()); ++i) {
        auto [x, y] = points[i];
        if (xl <= x && x < xr && yl <= y && y < yr) ans = min(ans, values[i]);
    }
    return ans;
}

long long dense_sum_naive_prod(const vector<vector<long long>>& grid, int il, int ir, int jl, int jr) {
    long long ans = 0;
    for (int i = il; i < ir; ++i) {
        for (int j = jl; j < jr; ++j) ans += grid[i][j];
    }
    return ans;
}

long long dense_min_naive_prod(const vector<vector<long long>>& grid, int il, int ir, int jl, int jr) {
    long long ans = min_e();
    for (int i = il; i < ir; ++i) {
        for (int j = jl; j < jr; ++j) ans = min(ans, grid[i][j]);
    }
    return ans;
}

void test_static_fixed_sum() {
    vector<pair<long long, long long>> points = {
        {-3, 5},
        {1, 2},
        {-3, 5},
        {4, -1},
        {1, 7},
        {0, 0},
    };
    StaticSumSegTree seg(points);

    seg.set(-3, 5, 10);
    seg.set(1, 2, 4);
    seg.set(4, -1, -2);
    seg.add(1, 2, 3);
    seg.add(1, 7, 8);

    assert(seg.get(-3, 5) == 10);
    assert(seg.get(1, 2) == 7);
    assert(seg.get(4, -1) == -2);
    assert(seg.get(1, 7) == 8);
    assert(seg.get(0, 0) == 0);

    assert(seg.prod(-10, 10, -10, 10) == 23);
    assert(seg.prod(-3, -2, 0, 10) == 10);
    assert(seg.prod(1, 2, 2, 8) == 15);
    assert(seg.prod(1, 4, -5, 3) == 7);
    assert(seg.prod(-100, 100, 100, 200) == 0);
    assert(seg.prod(1, 1, -10, 10) == 0);
    assert(seg.prod(-3, 1, 5, 6) == 10);
    assert(seg.prod(-3, 1, 6, 7) == 0);
}

void test_static_fixed_min() {
    vector<pair<long long, long long>> raw_points = {
        {-2, -2},
        {-2, 3},
        {5, 1},
        {7, 7},
        {5, 1},
    };
    vector<pair<long long, long long>> points = unique_points(raw_points);
    vector<long long> values(points.size(), min_e());
    StaticMinSegTree seg(raw_points);

    seg.set(-2, -2, 6);
    values[0] = 6;
    seg.set(-2, 3, 4);
    values[1] = 4;
    seg.set(5, 1, 9);
    values[2] = 9;
    seg.set(7, 7, -1);
    values[3] = -1;

    assert(seg.prod(-10, 10, -10, 10) == static_min_naive_prod(points, values, -10, 10, -10, 10));
    assert(seg.prod(-10, 6, -10, 10) == static_min_naive_prod(points, values, -10, 6, -10, 10));
    assert(seg.prod(5, 6, 1, 2) == static_min_naive_prod(points, values, 5, 6, 1, 2));
    assert(seg.prod(5, 7, 2, 6) == static_min_naive_prod(points, values, 5, 7, 2, 6));
    assert(seg.prod(8, 8, -10, 10) == static_min_naive_prod(points, values, 8, 8, -10, 10));
}

void test_static_random() {
    mt19937_64 rng(123456789);
    for (int tc = 0; tc < 200; ++tc) {
        int n = uniform_int_distribution<int>(1, 12)(rng);
        vector<pair<long long, long long>> raw_points;
        raw_points.reserve(n);
        for (int i = 0; i < n; ++i) {
            long long x = uniform_int_distribution<int>(-4, 4)(rng);
            long long y = uniform_int_distribution<int>(-4, 4)(rng);
            raw_points.push_back({x, y});
        }
        vector<pair<long long, long long>> points = unique_points(raw_points);
        StaticSumSegTree seg(raw_points);
        vector<long long> values(points.size(), 0);

        for (int step = 0; step < 200; ++step) {
            int type = uniform_int_distribution<int>(0, 3)(rng);
            if (type <= 2) {
                int idx = uniform_int_distribution<int>(0, static_cast<int>(points.size()) - 1)(rng);
                auto [x, y] = points[idx];
                if (type == 0) {
                    long long v = uniform_int_distribution<int>(-20, 20)(rng);
                    seg.set(x, y, v);
                    values[idx] = v;
                    assert(seg.get(x, y) == values[idx]);
                } else if (type == 1) {
                    long long delta = uniform_int_distribution<int>(-10, 10)(rng);
                    seg.add(x, y, delta);
                    values[idx] += delta;
                    assert(seg.get(x, y) == values[idx]);
                } else {
                    assert(seg.get(x, y) == values[idx]);
                }
            } else {
                long long xl = uniform_int_distribution<int>(-5, 5)(rng);
                long long xr = uniform_int_distribution<int>(-5, 5)(rng);
                long long yl = uniform_int_distribution<int>(-5, 5)(rng);
                long long yr = uniform_int_distribution<int>(-5, 5)(rng);
                if (xl > xr) swap(xl, xr);
                if (yl > yr) swap(yl, yr);
                assert(seg.prod(xl, xr, yl, yr) == static_sum_naive_prod(points, values, xl, xr, yl, yr));
            }
        }
    }
}

void test_dense_fixed_sum() {
    DenseSumSegTree one(1, 1);
    assert(one.prod(0, 1, 0, 1) == 0);
    one.set(0, 0, 5);
    assert(one.get(0, 0) == 5);
    assert(one.prod(0, 1, 0, 1) == 5);
    assert(one.prod(0, 0, 0, 1) == 0);
    assert(one.prod(0, 1, 1, 1) == 0);

    vector<vector<long long>> grid = {
        {1, 2, 3, 4},
        {5, 6, 7, 8},
        {9, 10, 11, 12},
    };
    DenseSumSegTree seg(grid);
    assert(seg.prod(0, 3, 0, 4) == 78);
    assert(seg.prod(1, 3, 1, 3) == 34);
    assert(seg.prod(1, 1, 0, 4) == 0);
    assert(seg.prod(0, 3, 2, 2) == 0);

    seg.set(2, 3, -5);
    grid[2][3] = -5;
    assert(seg.get(2, 3) == -5);
    assert(seg.prod(0, 3, 0, 4) == dense_sum_naive_prod(grid, 0, 3, 0, 4));
    assert(seg.prod(2, 3, 2, 4) == dense_sum_naive_prod(grid, 2, 3, 2, 4));
}

void test_dense_fixed_min() {
    vector<vector<long long>> grid = {
        {8, 6, 7},
        {5, 3, 0},
    };
    DenseMinSegTree seg(grid);
    assert(seg.prod(0, 2, 0, 3) == dense_min_naive_prod(grid, 0, 2, 0, 3));
    assert(seg.prod(0, 1, 0, 2) == dense_min_naive_prod(grid, 0, 1, 0, 2));
    assert(seg.prod(1, 2, 1, 3) == dense_min_naive_prod(grid, 1, 2, 1, 3));
    assert(seg.prod(0, 0, 0, 3) == dense_min_naive_prod(grid, 0, 0, 0, 3));
    assert(seg.prod(0, 2, 2, 2) == dense_min_naive_prod(grid, 0, 2, 2, 2));
}

void test_dense_random() {
    mt19937_64 rng(987654321);
    for (int tc = 0; tc < 150; ++tc) {
        int h = uniform_int_distribution<int>(1, 5)(rng);
        int w = uniform_int_distribution<int>(1, 5)(rng);
        DenseSumSegTree seg(h, w);
        vector<vector<long long>> grid(h, vector<long long>(w, 0));

        for (int step = 0; step < 200; ++step) {
            int type = uniform_int_distribution<int>(0, 2)(rng);
            if (type <= 1) {
                int i = uniform_int_distribution<int>(0, h - 1)(rng);
                int j = uniform_int_distribution<int>(0, w - 1)(rng);
                long long v = uniform_int_distribution<int>(-20, 20)(rng);
                seg.set(i, j, v);
                grid[i][j] = v;
                assert(seg.get(i, j) == grid[i][j]);
            } else {
                int il = uniform_int_distribution<int>(0, h)(rng);
                int ir = uniform_int_distribution<int>(0, h)(rng);
                int jl = uniform_int_distribution<int>(0, w)(rng);
                int jr = uniform_int_distribution<int>(0, w)(rng);
                if (il > ir) swap(il, ir);
                if (jl > jr) swap(jl, jr);
                assert(seg.prod(il, ir, jl, jr) == dense_sum_naive_prod(grid, il, ir, jl, jr));
            }
        }
    }
}

}  // namespace

int main() {
    test_static_fixed_sum();
    test_static_fixed_min();
    test_static_random();
    test_dense_fixed_sum();
    test_dense_fixed_min();
    test_dense_random();
    cout << "SegTree2D tests passed" << '\n';
    return 0;
}
