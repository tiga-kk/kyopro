#include "../data_structure/wavelet_matrix.hpp"

using namespace std;

namespace {

int naive_rank(const vector<long long>& a, long long x, int l, int r) {
    return static_cast<int>(count(a.begin() + l, a.begin() + r, x));
}

int naive_select(const vector<long long>& a, long long x, int k) {
    int seen = 0;
    for (int i = 0; i < static_cast<int>(a.size()); ++i) {
        if (a[i] != x) continue;
        if (seen == k) return i;
        ++seen;
    }
    return -1;
}

long long naive_kth_smallest(const vector<long long>& a, int l, int r, int k) {
    vector<long long> b(a.begin() + l, a.begin() + r);
    nth_element(b.begin(), b.begin() + k, b.end());
    return b[k];
}

long long naive_kth_largest(const vector<long long>& a, int l, int r, int k) {
    return naive_kth_smallest(a, l, r, r - l - 1 - k);
}

int naive_range_freq(const vector<long long>& a, int l, int r, long long lower, long long upper) {
    int cnt = 0;
    for (int i = l; i < r; ++i) {
        cnt += (lower <= a[i] && a[i] < upper);
    }
    return cnt;
}

optional<long long> naive_prev_value(const vector<long long>& a, int l, int r, long long upper) {
    optional<long long> ans;
    for (int i = l; i < r; ++i) {
        if (a[i] >= upper) continue;
        if (!ans || *ans < a[i]) ans = a[i];
    }
    return ans;
}

optional<long long> naive_next_value(const vector<long long>& a, int l, int r, long long lower) {
    optional<long long> ans;
    for (int i = l; i < r; ++i) {
        if (a[i] < lower) continue;
        if (!ans || a[i] < *ans) ans = a[i];
    }
    return ans;
}

void check_all_queries(const vector<long long>& a) {
    WaveletMatrix wm(a);
    assert(wm.size() == static_cast<int>(a.size()));
    for (int i = 0; i < static_cast<int>(a.size()); ++i) {
        assert(wm.access(i) == a[i]);
    }

    vector<long long> values = a;
    sort(values.begin(), values.end());
    values.erase(unique(values.begin(), values.end()), values.end());

    vector<long long> probes = values;
    if (!values.empty()) {
        probes.push_back(values.front() - 1);
        probes.push_back(values.back() + 1);
    }
    probes.push_back(-100);
    probes.push_back(0);
    probes.push_back(100);
    sort(probes.begin(), probes.end());
    probes.erase(unique(probes.begin(), probes.end()), probes.end());

    for (long long x : probes) {
        for (int r = 0; r <= static_cast<int>(a.size()); ++r) {
            assert(wm.rank(x, r) == naive_rank(a, x, 0, r));
        }
        for (int l = 0; l < static_cast<int>(a.size()); ++l) {
            for (int r = l; r <= static_cast<int>(a.size()); ++r) {
                assert(wm.rank(x, l, r) == naive_rank(a, x, l, r));
            }
        }
        int total = naive_rank(a, x, 0, static_cast<int>(a.size()));
        for (int k = 0; k < total; ++k) {
            assert(wm.select(x, k) == naive_select(a, x, k));
        }
        assert(wm.select(x, total) == -1);
    }

    for (int l = 0; l < static_cast<int>(a.size()); ++l) {
        for (int r = l + 1; r <= static_cast<int>(a.size()); ++r) {
            int len = r - l;
            for (int k = 0; k < len; ++k) {
                assert(wm.kth_smallest(l, r, k) == naive_kth_smallest(a, l, r, k));
                assert(wm.kth_largest(l, r, k) == naive_kth_largest(a, l, r, k));
            }
            for (long long upper : probes) {
                assert(wm.range_freq(l, r, upper) == naive_range_freq(a, l, r, numeric_limits<long long>::min(), upper));
            }
            for (long long lower : probes) {
                for (long long upper : probes) {
                    assert(wm.range_freq(l, r, lower, upper) == naive_range_freq(a, l, r, lower, upper));
                }
                assert(wm.next_value(l, r, lower) == naive_next_value(a, l, r, lower));
            }
            for (long long upper : probes) {
                assert(wm.prev_value(l, r, upper) == naive_prev_value(a, l, r, upper));
            }
            assert(wm.range_min(l, r) == *min_element(a.begin() + l, a.begin() + r));
            assert(wm.range_max(l, r) == *max_element(a.begin() + l, a.begin() + r));
        }
    }
}

void check_sampled_queries(const vector<long long>& a, mt19937_64& rng) {
    WaveletMatrix wm(a);
    assert(wm.size() == static_cast<int>(a.size()));
    for (int i = 0; i < static_cast<int>(a.size()); ++i) {
        assert(wm.access(i) == a[i]);
    }

    vector<long long> values = a;
    sort(values.begin(), values.end());
    values.erase(unique(values.begin(), values.end()), values.end());

    vector<long long> probes = values;
    probes.push_back(values.front() - 1);
    probes.push_back(values.back() + 1);
    probes.push_back(-100);
    probes.push_back(0);
    probes.push_back(100);
    sort(probes.begin(), probes.end());
    probes.erase(unique(probes.begin(), probes.end()), probes.end());

    uniform_int_distribution<int> idx_dist(0, static_cast<int>(a.size()) - 1);
    uniform_int_distribution<int> probe_dist(0, static_cast<int>(probes.size()) - 1);

    for (int iter = 0; iter < 80; ++iter) {
        int l = idx_dist(rng);
        int r = idx_dist(rng);
        if (l > r) swap(l, r);
        ++r;
        assert(1 <= r - l);

        long long x = probes[probe_dist(rng)];
        int rr = uniform_int_distribution<int>(0, static_cast<int>(a.size()))(rng);
        assert(wm.rank(x, rr) == naive_rank(a, x, 0, rr));
        assert(wm.rank(x, l, r) == naive_rank(a, x, l, r));

        int total = naive_rank(a, x, 0, static_cast<int>(a.size()));
        int k_sel = total == 0 ? 0 : uniform_int_distribution<int>(0, total)(rng);
        assert(wm.select(x, k_sel) == naive_select(a, x, k_sel));

        int len = r - l;
        int k = uniform_int_distribution<int>(0, len - 1)(rng);
        assert(wm.kth_smallest(l, r, k) == naive_kth_smallest(a, l, r, k));
        assert(wm.kth_largest(l, r, k) == naive_kth_largest(a, l, r, k));

        long long lower = probes[probe_dist(rng)];
        long long upper = probes[probe_dist(rng)];
        if (lower > upper) swap(lower, upper);
        assert(wm.range_freq(l, r, upper) == naive_range_freq(a, l, r, numeric_limits<long long>::min(), upper));
        assert(wm.range_freq(l, r, lower, upper) == naive_range_freq(a, l, r, lower, upper));
        assert(wm.prev_value(l, r, upper) == naive_prev_value(a, l, r, upper));
        assert(wm.next_value(l, r, lower) == naive_next_value(a, l, r, lower));
        assert(wm.range_min(l, r) == *min_element(a.begin() + l, a.begin() + r));
        assert(wm.range_max(l, r) == *max_element(a.begin() + l, a.begin() + r));
    }
}

void test_fixed_cases() {
    check_all_queries(vector<long long>{5});
    check_all_queries(vector<long long>{7, 7, 7, 7, 7});
    check_all_queries(vector<long long>{-5, -1, -5, 3, 0, 3, 9, -2});

    vector<long long> a = {4, -1, 4, 2, -1, 7, 4};
    WaveletMatrix wm(a);
    assert(wm.select(4, 0) == 0);
    assert(wm.select(4, 1) == 2);
    assert(wm.select(4, 2) == 6);
    assert(wm.select(4, 3) == -1);
    assert(wm.select(100, 0) == -1);

    assert(wm.prev_value(1, 6, 2) == optional<long long>(-1));
    assert(wm.prev_value(1, 6, -5) == nullopt);
    assert(wm.next_value(1, 6, 3) == optional<long long>(4));
    assert(wm.next_value(1, 6, 8) == nullopt);
}

void test_random_cases() {
    mt19937_64 rng(20260704);
    uniform_int_distribution<int> n_dist(1, 80);
    uniform_int_distribution<long long> v_dist(-20, 20);

    for (int tc = 0; tc < 600; ++tc) {
        int n = n_dist(rng);
        vector<long long> a(n);
        for (int i = 0; i < n; ++i) a[i] = v_dist(rng);
        check_sampled_queries(a, rng);
    }
}

}  // namespace

int main() {
    test_fixed_cases();
    test_random_cases();
    cout << "WaveletMatrix tests passed\n";
    return 0;
}
