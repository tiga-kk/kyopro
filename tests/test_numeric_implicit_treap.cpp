#include "../data_structure/implicit_treap_numeric.hpp"

using namespace std;

namespace {

using Treap = NumericImplicitTreap;
using RangeInfo = NumericImplicitTreap::RangeInfo;

long long range_sum_naive(const vector<long long>& a, int l, int r) {
    return accumulate(a.begin() + l, a.begin() + r, 0LL);
}

long long range_min_naive(const vector<long long>& a, int l, int r) {
    return *min_element(a.begin() + l, a.begin() + r);
}

long long range_max_naive(const vector<long long>& a, int l, int r) {
    return *max_element(a.begin() + l, a.begin() + r);
}

RangeInfo range_info_naive(const vector<long long>& a, int l, int r) {
    return RangeInfo{range_sum_naive(a, l, r), range_min_naive(a, l, r), range_max_naive(a, l, r), r - l};
}

void check_same(Treap& treap, const vector<long long>& a) {
    assert(treap.size() == static_cast<int>(a.size()));
    assert(treap.empty() == a.empty());
    assert(treap.to_vector() == a);
}

void erase_range(vector<long long>& a, int l, int r) {
    a.erase(a.begin() + l, a.begin() + r);
}

void move_range(vector<long long>& a, int l, int r, int pos_after_erase) {
    vector<long long> cut(a.begin() + l, a.begin() + r);
    erase_range(a, l, r);
    a.insert(a.begin() + pos_after_erase, cut.begin(), cut.end());
}

int max_right_sum_limit_naive(const vector<long long>& a, int l, long long limit) {
    long long sum = 0;
    int r = l;
    while (r < static_cast<int>(a.size()) && sum + a[r] <= limit) {
        sum += a[r];
        ++r;
    }
    return r;
}

int min_left_sum_limit_naive(const vector<long long>& a, int r, long long limit) {
    long long sum = 0;
    int l = r;
    while (l > 0 && sum + a[l - 1] <= limit) {
        sum += a[l - 1];
        --l;
    }
    return l;
}

int lower_bound_prefix_sum_naive(const vector<long long>& a, long long x) {
    long long sum = 0;
    for (int i = 0; i < static_cast<int>(a.size()); ++i) {
        sum += a[i];
        if (sum >= x) return i;
    }
    return static_cast<int>(a.size());
}

void test_basic_operations() {
    vector<long long> a = {5, 1, 4};
    Treap treap(a);
    check_same(treap, a);

    treap.insert(1, 9);
    a.insert(a.begin() + 1, 9);
    check_same(treap, a);

    assert(treap.get(2) == a[2]);
    treap.set_value(2, 7);
    a[2] = 7;
    check_same(treap, a);

    treap.push_front(3);
    a.insert(a.begin(), 3);
    treap.push_back(8);
    a.push_back(8);
    check_same(treap, a);

    assert(treap.pop_front() == 3);
    a.erase(a.begin());
    assert(treap.pop_back() == 8);
    a.pop_back();
    check_same(treap, a);

    assert(treap.erase(1) == a[1]);
    a.erase(a.begin() + 1);
    check_same(treap, a);

    treap.insert(1, 6);
    a.insert(a.begin() + 1, 6);
    treap.insert(3, 2);
    a.insert(a.begin() + 3, 2);
    check_same(treap, a);

    treap.reverse(1, 4);
    reverse(a.begin() + 1, a.begin() + 4);
    check_same(treap, a);

    treap.range_add(0, 3, 5);
    for (int i = 0; i < 3; ++i) a[i] += 5;
    check_same(treap, a);

    treap.range_assign(2, 4, -2);
    for (int i = 2; i < 4; ++i) a[i] = -2;
    check_same(treap, a);

    assert(treap.range_sum(0, static_cast<int>(a.size())) == range_sum_naive(a, 0, static_cast<int>(a.size())));
    assert(treap.range_min(1, 4) == range_min_naive(a, 1, 4));
    assert(treap.range_max(0, 3) == range_max_naive(a, 0, 3));

    RangeInfo info = treap.range_info(1, 4);
    RangeInfo expected = range_info_naive(a, 1, 4);
    assert(info.sum == expected.sum);
    assert(info.min == expected.min);
    assert(info.max == expected.max);
    assert(info.size == expected.size);
}

void test_lazy_assign_and_add_ordering() {
    {
        vector<long long> a = {3, 1, 4, 1, 5, 9, 2, 6};
        Treap treap(a);

        treap.range_assign(2, 6, 7);
        for (int i = 2; i < 6; ++i) a[i] = 7;
        treap.range_add(4, 8, 3);
        for (int i = 4; i < 8; ++i) a[i] += 3;

        check_same(treap, a);

        RangeInfo info = treap.range_info(0, static_cast<int>(a.size()));
        RangeInfo expected = range_info_naive(a, 0, static_cast<int>(a.size()));
        assert(info.sum == expected.sum);
        assert(info.min == expected.min);
        assert(info.max == expected.max);
        assert(info.size == expected.size);
    }

    {
        vector<long long> a = {8, 6, 7, 5, 3, 0, 9, 1};
        Treap treap(a);

        treap.range_add(1, 6, 4);
        for (int i = 1; i < 6; ++i) a[i] += 4;
        treap.range_assign(3, 7, 2);
        for (int i = 3; i < 7; ++i) a[i] = 2;

        check_same(treap, a);

        RangeInfo info = treap.range_info(1, 7);
        RangeInfo expected = range_info_naive(a, 1, 7);
        assert(info.sum == expected.sum);
        assert(info.min == expected.min);
        assert(info.max == expected.max);
        assert(info.size == expected.size);
    }
}

void test_ownership_boundaries() {
    {
        vector<long long> a = {4, 1, 7, 3};
        Treap treap(a);

        Treap empty_prefix = treap.split_prefix(0);
        check_same(empty_prefix, {});
        check_same(treap, a);

        Treap whole_prefix = treap.split_prefix(static_cast<int>(a.size()));
        check_same(whole_prefix, a);
        check_same(treap, {});

        treap.push_back(11);
        a = {11};
        check_same(treap, a);
    }

    {
        vector<long long> a = {9, 2, 5, 8, 1};
        Treap treap(a);

        Treap empty_cut = treap.cut(2, 2);
        check_same(empty_cut, {});
        check_same(treap, a);
    }

    {
        Treap treap(vector<long long>{1, 4});
        Treap donor(vector<long long>{7, 8});
        treap.insert_treap(1, std::move(donor));
        assert(donor.empty());
        donor.push_back(9);
        donor.push_front(6);
        check_same(donor, {6, 9});
    }

    {
        Treap treap(vector<long long>{2, 3});
        Treap donor(vector<long long>{5, 6});
        treap.append(std::move(donor));
        assert(donor.empty());
        donor.push_back(10);
        check_same(donor, {10});
    }
}

void test_rangeinfo_predicates() {
    vector<long long> a = {2, 4, 1, 6, 3, 5, 7};
    Treap treap(a);

    int l = 1;
    assert(treap.max_right(l, [](const RangeInfo& info) { return info.size <= 3; }) == 4);
    assert(treap.max_right(0, [](const RangeInfo& info) { return info.max < 6; }) == 3);

    int r = 6;
    assert(treap.min_left(r, [](const RangeInfo& info) { return info.size <= 3; }) == 3);
    assert(treap.min_left(5, [](const RangeInfo& info) { return info.min >= 3; }) == 3);
}

void test_cut_merge_move_and_search() {
    vector<long long> a = {2, 1, 3, 4, 2, 5, 1, 6};
    Treap treap(a);

    Treap prefix = treap.split_prefix(3);
    vector<long long> prefix_vec(a.begin(), a.begin() + 3);
    erase_range(a, 0, 3);
    check_same(prefix, prefix_vec);
    check_same(treap, a);

    treap.insert_treap(2, std::move(prefix));
    a.insert(a.begin() + 2, prefix_vec.begin(), prefix_vec.end());
    check_same(treap, a);

    Treap middle = treap.cut(1, 5);
    vector<long long> middle_vec(a.begin() + 1, a.begin() + 5);
    erase_range(a, 1, 5);
    check_same(middle, middle_vec);
    check_same(treap, a);

    treap.append(std::move(middle));
    a.insert(a.end(), middle_vec.begin(), middle_vec.end());
    check_same(treap, a);

    treap.move(2, 6, 1);
    move_range(a, 2, 6, 1);
    check_same(treap, a);

    long long limit = 8;
    int l = 1;
    assert(treap.max_right(l, [&](const RangeInfo& x) { return x.sum <= limit; }) == max_right_sum_limit_naive(a, l, limit));

    int r = static_cast<int>(a.size()) - 1;
    long long suffix_limit = 7;
    assert(treap.min_left(r, [&](const RangeInfo& x) { return x.sum <= suffix_limit; }) == min_left_sum_limit_naive(a, r, suffix_limit));

    assert(treap.lower_bound_prefix_sum(10) == lower_bound_prefix_sum_naive(a, 10));
}

void test_random_operations() {
    mt19937 rng(123456789);
    vector<long long> a;
    Treap treap;

    auto rand_int = [&](int lo, int hi) -> int {
        uniform_int_distribution<int> dist(lo, hi);
        return dist(rng);
    };

    for (int step = 0; step < 3500; ++step) {
        int op = rand_int(0, 20);
        if (a.empty() && op != 0 && op != 4 && op != 5) op = rand_int(0, 2);

        if (op == 0) {
            int pos = rand_int(0, static_cast<int>(a.size()));
            long long x = rand_int(0, 20);
            treap.insert(pos, x);
            a.insert(a.begin() + pos, x);
        } else if (op == 1 && !a.empty()) {
            int pos = rand_int(0, static_cast<int>(a.size()) - 1);
            long long got = treap.erase(pos);
            assert(got == a[pos]);
            a.erase(a.begin() + pos);
        } else if (op == 2 && !a.empty()) {
            int pos = rand_int(0, static_cast<int>(a.size()) - 1);
            assert(treap.get(pos) == a[pos]);
        } else if (op == 3 && !a.empty()) {
            int pos = rand_int(0, static_cast<int>(a.size()) - 1);
            long long x = rand_int(0, 20);
            treap.set_value(pos, x);
            a[pos] = x;
        } else if (op == 4) {
            long long x = rand_int(0, 20);
            treap.push_front(x);
            a.insert(a.begin(), x);
        } else if (op == 5) {
            long long x = rand_int(0, 20);
            treap.push_back(x);
            a.push_back(x);
        } else if (op == 6 && !a.empty()) {
            assert(treap.pop_front() == a.front());
            a.erase(a.begin());
        } else if (op == 7 && !a.empty()) {
            assert(treap.pop_back() == a.back());
            a.pop_back();
        } else if (op == 8 && !a.empty()) {
            int l = rand_int(0, static_cast<int>(a.size()) - 1);
            int r = rand_int(l + 1, static_cast<int>(a.size()));
            treap.reverse(l, r);
            reverse(a.begin() + l, a.begin() + r);
        } else if (op == 9 && !a.empty()) {
            int l = rand_int(0, static_cast<int>(a.size()) - 1);
            int r = rand_int(l + 1, static_cast<int>(a.size()));
            long long x = rand_int(0, 5);
            treap.range_add(l, r, x);
            for (int i = l; i < r; ++i) a[i] += x;
        } else if (op == 10 && !a.empty()) {
            int l = rand_int(0, static_cast<int>(a.size()) - 1);
            int r = rand_int(l + 1, static_cast<int>(a.size()));
            long long x = rand_int(0, 20);
            treap.range_assign(l, r, x);
            for (int i = l; i < r; ++i) a[i] = x;
        } else if (op == 11 && !a.empty()) {
            int l = rand_int(0, static_cast<int>(a.size()) - 1);
            int r = rand_int(l + 1, static_cast<int>(a.size()));
            assert(treap.range_sum(l, r) == range_sum_naive(a, l, r));
        } else if (op == 12 && !a.empty()) {
            int l = rand_int(0, static_cast<int>(a.size()) - 1);
            int r = rand_int(l + 1, static_cast<int>(a.size()));
            assert(treap.range_min(l, r) == range_min_naive(a, l, r));
        } else if (op == 13 && !a.empty()) {
            int l = rand_int(0, static_cast<int>(a.size()) - 1);
            int r = rand_int(l + 1, static_cast<int>(a.size()));
            assert(treap.range_max(l, r) == range_max_naive(a, l, r));
        } else if (op == 14 && !a.empty()) {
            int l = rand_int(0, static_cast<int>(a.size()) - 1);
            int r = rand_int(l + 1, static_cast<int>(a.size()));
            RangeInfo info = treap.range_info(l, r);
            RangeInfo expected = range_info_naive(a, l, r);
            assert(info.sum == expected.sum);
            assert(info.min == expected.min);
            assert(info.max == expected.max);
            assert(info.size == expected.size);
        } else if (op == 15 && a.size() >= 2) {
            int l = rand_int(0, static_cast<int>(a.size()) - 1);
            int r = rand_int(l + 1, static_cast<int>(a.size()));
            int pos = rand_int(0, static_cast<int>(a.size()) - (r - l));
            treap.move(l, r, pos);
            move_range(a, l, r, pos);
        } else if (op == 16) {
            int k = rand_int(0, static_cast<int>(a.size()));
            Treap prefix = treap.split_prefix(k);
            vector<long long> prefix_vec(a.begin(), a.begin() + k);
            erase_range(a, 0, k);
            check_same(prefix, prefix_vec);
            int pos = rand_int(0, static_cast<int>(a.size()));
            treap.insert_treap(pos, std::move(prefix));
            a.insert(a.begin() + pos, prefix_vec.begin(), prefix_vec.end());
        } else if (op == 17 && !a.empty()) {
            int l = rand_int(0, static_cast<int>(a.size()) - 1);
            int r = rand_int(l + 1, static_cast<int>(a.size()));
            Treap part = treap.cut(l, r);
            vector<long long> part_vec(a.begin() + l, a.begin() + r);
            erase_range(a, l, r);
            int pos = rand_int(0, static_cast<int>(a.size()));
            treap.insert_treap(pos, std::move(part));
            a.insert(a.begin() + pos, part_vec.begin(), part_vec.end());
        } else if (op == 18) {
            int pos = rand_int(0, static_cast<int>(a.size()));
            Treap other(vector<long long>{static_cast<long long>(rand_int(0, 20)), static_cast<long long>(rand_int(0, 20))});
            vector<long long> b = other.to_vector();
            treap.insert_treap(pos, std::move(other));
            a.insert(a.begin() + pos, b.begin(), b.end());
        } else if (op == 19) {
            Treap other(vector<long long>{static_cast<long long>(rand_int(0, 20))});
            vector<long long> b = other.to_vector();
            treap.append(std::move(other));
            a.insert(a.end(), b.begin(), b.end());
        } else if (op == 20 && !a.empty()) {
            int l = rand_int(0, static_cast<int>(a.size()) - 1);
            long long limit = rand_int(0, 60);
            assert(treap.max_right(l, [&](const RangeInfo& x) { return x.sum <= limit; }) == max_right_sum_limit_naive(a, l, limit));

            int r = rand_int(0, static_cast<int>(a.size()));
            long long suffix_limit = rand_int(0, 60);
            assert(treap.min_left(r, [&](const RangeInfo& x) { return x.sum <= suffix_limit; }) == min_left_sum_limit_naive(a, r, suffix_limit));

            long long need = rand_int(1, 80);
            assert(treap.lower_bound_prefix_sum(need) == lower_bound_prefix_sum_naive(a, need));
        }

        check_same(treap, a);
    }
}

}  // namespace

int main() {
    test_basic_operations();
    test_lazy_assign_and_add_ordering();
    test_ownership_boundaries();
    test_rangeinfo_predicates();
    test_cut_merge_move_and_search();
    test_random_operations();
    cout << "NumericImplicitTreap tests passed\n";
    return 0;
}
