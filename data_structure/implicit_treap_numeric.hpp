#pragma once

#include <bits/stdc++.h>

struct NumericImplicitTreap {
    struct RangeInfo {
        long long sum;
        long long min;
        long long max;
        int size;
    };

private:
    struct Node {
        Node* l;
        Node* r;
        uint32_t pri;
        int sz;
        long long val;
        long long sum;
        long long mn;
        long long mx;
        long long lazy_add;
        bool has_assign;
        long long lazy_assign;
        bool rev;

        explicit Node(long long x)
            : l(nullptr),
              r(nullptr),
              pri(random_priority()),
              sz(1),
              val(x),
              sum(x),
              mn(x),
              mx(x),
              lazy_add(0),
              has_assign(false),
              lazy_assign(0),
              rev(false) {}
    };

    Node* root_;

    static uint32_t random_priority() {
        static uint64_t state = 0x9e3779b97f4a7c15ULL;
        state += 0x9e3779b97f4a7c15ULL;
        uint64_t z = state;
        z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
        z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
        z ^= z >> 31;
        return static_cast<uint32_t>(z);
    }

    static int node_size(Node* t) {
        return t ? t->sz : 0;
    }

    static long long node_sum(Node* t) {
        return t ? t->sum : 0LL;
    }

    static long long node_min(Node* t) {
        return t ? t->mn : std::numeric_limits<long long>::max();
    }

    static long long node_max(Node* t) {
        return t ? t->mx : std::numeric_limits<long long>::min();
    }

    static RangeInfo empty_info() {
        return RangeInfo{0, std::numeric_limits<long long>::max(), std::numeric_limits<long long>::min(), 0};
    }

    static RangeInfo single_info(long long x) {
        return RangeInfo{x, x, x, 1};
    }

    static RangeInfo info_of(Node* t) {
        if (!t) return empty_info();
        return RangeInfo{t->sum, t->mn, t->mx, t->sz};
    }

    static RangeInfo combine_info(const RangeInfo& a, const RangeInfo& b) {
        if (a.size == 0) return b;
        if (b.size == 0) return a;
        return RangeInfo{a.sum + b.sum, std::min(a.min, b.min), std::max(a.max, b.max), a.size + b.size};
    }

    static void apply_reverse(Node* t) {
        if (!t) return;
        std::swap(t->l, t->r);
        t->rev = !t->rev;
    }

    static void apply_assign(Node* t, long long x) {
        if (!t) return;
        t->val = x;
        t->sum = x * t->sz;
        t->mn = x;
        t->mx = x;
        t->has_assign = true;
        t->lazy_assign = x;
        t->lazy_add = 0;
    }

    static void apply_add(Node* t, long long x) {
        if (!t) return;
        t->val += x;
        t->sum += x * t->sz;
        t->mn += x;
        t->mx += x;
        if (t->has_assign) {
            t->lazy_assign += x;
        } else {
            t->lazy_add += x;
        }
    }

    static void push(Node* t) {
        if (!t) return;
        if (t->rev) {
            apply_reverse(t->l);
            apply_reverse(t->r);
            t->rev = false;
        }
        if (t->has_assign) {
            apply_assign(t->l, t->lazy_assign);
            apply_assign(t->r, t->lazy_assign);
            t->has_assign = false;
        }
        if (t->lazy_add != 0) {
            apply_add(t->l, t->lazy_add);
            apply_add(t->r, t->lazy_add);
            t->lazy_add = 0;
        }
    }

    static void pull(Node* t) {
        if (!t) return;
        t->sz = 1 + node_size(t->l) + node_size(t->r);
        t->sum = node_sum(t->l) + t->val + node_sum(t->r);
        t->mn = std::min(t->val, std::min(node_min(t->l), node_min(t->r)));
        t->mx = std::max(t->val, std::max(node_max(t->l), node_max(t->r)));
    }

    static std::pair<Node*, Node*> split(Node* t, int k) {
        if (!t) return {nullptr, nullptr};
        push(t);
        if (k <= node_size(t->l)) {
            auto [a, b] = split(t->l, k);
            t->l = b;
            pull(t);
            return {a, t};
        }
        auto [a, b] = split(t->r, k - node_size(t->l) - 1);
        t->r = a;
        pull(t);
        return {t, b};
    }

    static Node* merge(Node* a, Node* b) {
        if (!a) return b;
        if (!b) return a;
        if (a->pri > b->pri) {
            push(a);
            a->r = merge(a->r, b);
            pull(a);
            return a;
        }
        push(b);
        b->l = merge(a, b->l);
        pull(b);
        return b;
    }

    struct Split3 {
        Node* left;
        Node* mid;
        Node* right;
    };

    static Split3 split3(Node* root, int l, int r) {
        auto [ab, c] = split(root, r);
        auto [a, b] = split(ab, l);
        return Split3{a, b, c};
    }

    static Node* clone(Node* t) {
        if (!t) return nullptr;
        Node* res = new Node(t->val);
        res->pri = t->pri;
        res->sz = t->sz;
        res->sum = t->sum;
        res->mn = t->mn;
        res->mx = t->mx;
        res->lazy_add = t->lazy_add;
        res->has_assign = t->has_assign;
        res->lazy_assign = t->lazy_assign;
        res->rev = t->rev;
        res->l = clone(t->l);
        res->r = clone(t->r);
        return res;
    }

    static void destroy(Node* t) {
        if (!t) return;
        destroy(t->l);
        destroy(t->r);
        delete t;
    }

    static void collect(Node* t, std::vector<long long>& out) {
        if (!t) return;
        push(t);
        collect(t->l, out);
        out.push_back(t->val);
        collect(t->r, out);
        pull(t);
    }

    template <class Pred>
    static int max_right_node(Node* t, RangeInfo& acc, Pred& pred) {
        if (!t) return 0;
        RangeInfo whole = combine_info(acc, info_of(t));
        if (pred(whole)) {
            acc = whole;
            return node_size(t);
        }
        push(t);
        RangeInfo left_whole = combine_info(acc, info_of(t->l));
        if (!pred(left_whole)) {
            return max_right_node(t->l, acc, pred);
        }
        acc = left_whole;
        RangeInfo with_self = combine_info(acc, single_info(t->val));
        if (!pred(with_self)) {
            return node_size(t->l);
        }
        acc = with_self;
        return node_size(t->l) + 1 + max_right_node(t->r, acc, pred);
    }

    template <class Pred>
    static int min_left_node(Node* t, RangeInfo& acc, Pred& pred) {
        if (!t) return 0;
        RangeInfo whole = combine_info(info_of(t), acc);
        if (pred(whole)) {
            acc = whole;
            return node_size(t);
        }
        push(t);
        RangeInfo right_whole = combine_info(info_of(t->r), acc);
        if (!pred(right_whole)) {
            return min_left_node(t->r, acc, pred);
        }
        acc = right_whole;
        RangeInfo with_self = combine_info(single_info(t->val), acc);
        if (!pred(with_self)) {
            return node_size(t->r);
        }
        acc = with_self;
        return node_size(t->r) + 1 + min_left_node(t->l, acc, pred);
    }

    explicit NumericImplicitTreap(Node* root) : root_(root) {}

public:
    NumericImplicitTreap() : root_(nullptr) {}

    explicit NumericImplicitTreap(const std::vector<long long>& values) : root_(nullptr) {
        for (long long x : values) {
            root_ = merge(root_, new Node(x));
        }
    }

    NumericImplicitTreap(const NumericImplicitTreap& other) : root_(clone(other.root_)) {}

    NumericImplicitTreap(NumericImplicitTreap&& other) noexcept : root_(other.root_) {
        other.root_ = nullptr;
    }

    NumericImplicitTreap& operator=(const NumericImplicitTreap& other) {
        if (this == &other) return *this;
        NumericImplicitTreap tmp(other);
        swap(tmp);
        return *this;
    }

    NumericImplicitTreap& operator=(NumericImplicitTreap&& other) noexcept {
        if (this == &other) return *this;
        destroy(root_);
        root_ = other.root_;
        other.root_ = nullptr;
        return *this;
    }

    ~NumericImplicitTreap() {
        destroy(root_);
    }

    void swap(NumericImplicitTreap& other) noexcept {
        std::swap(root_, other.root_);
    }

    int size() const {
        return node_size(root_);
    }

    bool empty() const {
        return root_ == nullptr;
    }

    void insert(int pos, long long x) {
        assert(0 <= pos && pos <= size());
        auto [a, b] = split(root_, pos);
        root_ = merge(merge(a, new Node(x)), b);
    }

    long long erase(int pos) {
        assert(0 <= pos && pos < size());
        Split3 sp = split3(root_, pos, pos + 1);
        long long res = sp.mid->val;
        destroy(sp.mid);
        root_ = merge(sp.left, sp.right);
        return res;
    }

    long long get(int pos) {
        assert(0 <= pos && pos < size());
        Split3 sp = split3(root_, pos, pos + 1);
        long long res = sp.mid->val;
        root_ = merge(sp.left, merge(sp.mid, sp.right));
        return res;
    }

    void set_value(int pos, long long x) {
        assert(0 <= pos && pos < size());
        Split3 sp = split3(root_, pos, pos + 1);
        sp.mid->val = x;
        sp.mid->sum = x;
        sp.mid->mn = x;
        sp.mid->mx = x;
        sp.mid->lazy_add = 0;
        sp.mid->has_assign = false;
        sp.mid->rev = false;
        pull(sp.mid);
        root_ = merge(sp.left, merge(sp.mid, sp.right));
    }

    void push_front(long long x) {
        insert(0, x);
    }

    void push_back(long long x) {
        insert(size(), x);
    }

    long long pop_front() {
        assert(!empty());
        return erase(0);
    }

    long long pop_back() {
        assert(!empty());
        return erase(size() - 1);
    }

    NumericImplicitTreap split_prefix(int k) {
        assert(0 <= k && k <= size());
        auto [a, b] = split(root_, k);
        root_ = b;
        return NumericImplicitTreap(a);
    }

    NumericImplicitTreap cut(int l, int r) {
        assert(0 <= l && l <= r && r <= size());
        Split3 sp = split3(root_, l, r);
        root_ = merge(sp.left, sp.right);
        return NumericImplicitTreap(sp.mid);
    }

    void insert_treap(int pos, NumericImplicitTreap other) {
        assert(0 <= pos && pos <= size());
        auto [a, b] = split(root_, pos);
        Node* middle = other.root_;
        other.root_ = nullptr;
        root_ = merge(merge(a, middle), b);
    }

    void append(NumericImplicitTreap other) {
        Node* tail = other.root_;
        other.root_ = nullptr;
        root_ = merge(root_, tail);
    }

    void move(int l, int r, int pos_after_erase) {
        assert(0 <= l && l <= r && r <= size());
        assert(0 <= pos_after_erase && pos_after_erase <= size() - (r - l));
        Split3 sp = split3(root_, l, r);
        root_ = merge(sp.left, sp.right);
        auto [a, b] = split(root_, pos_after_erase);
        root_ = merge(merge(a, sp.mid), b);
    }

    void reverse(int l, int r) {
        assert(0 <= l && l <= r && r <= size());
        Split3 sp = split3(root_, l, r);
        apply_reverse(sp.mid);
        root_ = merge(sp.left, merge(sp.mid, sp.right));
    }

    void range_add(int l, int r, long long x) {
        assert(0 <= l && l <= r && r <= size());
        Split3 sp = split3(root_, l, r);
        apply_add(sp.mid, x);
        root_ = merge(sp.left, merge(sp.mid, sp.right));
    }

    void range_assign(int l, int r, long long x) {
        assert(0 <= l && l <= r && r <= size());
        Split3 sp = split3(root_, l, r);
        apply_assign(sp.mid, x);
        root_ = merge(sp.left, merge(sp.mid, sp.right));
    }

    long long range_sum(int l, int r) {
        assert(0 <= l && l <= r && r <= size());
        Split3 sp = split3(root_, l, r);
        long long res = node_sum(sp.mid);
        root_ = merge(sp.left, merge(sp.mid, sp.right));
        return res;
    }

    long long range_min(int l, int r) {
        assert(0 <= l && l < r && r <= size());
        Split3 sp = split3(root_, l, r);
        long long res = node_min(sp.mid);
        root_ = merge(sp.left, merge(sp.mid, sp.right));
        return res;
    }

    long long range_max(int l, int r) {
        assert(0 <= l && l < r && r <= size());
        Split3 sp = split3(root_, l, r);
        long long res = node_max(sp.mid);
        root_ = merge(sp.left, merge(sp.mid, sp.right));
        return res;
    }

    RangeInfo range_info(int l, int r) {
        assert(0 <= l && l <= r && r <= size());
        Split3 sp = split3(root_, l, r);
        RangeInfo res = info_of(sp.mid);
        root_ = merge(sp.left, merge(sp.mid, sp.right));
        return res;
    }

    template <class Pred>
    int max_right(int l, Pred pred) {
        assert(0 <= l && l <= size());
        assert(pred(empty_info()));
        auto [a, b] = split(root_, l);
        RangeInfo acc = empty_info();
        int taken = max_right_node(b, acc, pred);
        root_ = merge(a, b);
        return l + taken;
    }

    template <class Pred>
    int min_left(int r, Pred pred) {
        assert(0 <= r && r <= size());
        assert(pred(empty_info()));
        auto [a, b] = split(root_, r);
        RangeInfo acc = empty_info();
        int taken = min_left_node(a, acc, pred);
        root_ = merge(a, b);
        return r - taken;
    }

    int lower_bound_prefix_sum(long long x) {
        if (x <= 0) return 0;
        if (node_sum(root_) < x) return size();
        int idx = 0;
        Node* cur = root_;
        while (cur) {
            push(cur);
            long long left_sum = node_sum(cur->l);
            if (left_sum >= x) {
                cur = cur->l;
                continue;
            }
            x -= left_sum;
            if (cur->val >= x) {
                return idx + node_size(cur->l);
            }
            x -= cur->val;
            idx += node_size(cur->l) + 1;
            cur = cur->r;
        }
        return size();
    }

    std::vector<long long> to_vector() {
        std::vector<long long> res;
        res.reserve(size());
        collect(root_, res);
        return res;
    }
};
