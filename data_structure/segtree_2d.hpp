#pragma once

#include <bits/stdc++.h>

template <class S, S (*op)(S, S), S (*e)()>
struct StaticSegTree2D {
private:
    struct Node {
        std::vector<long long> ys;
        int base = 1;
        std::vector<S> seg;

        void init() {
            base = 1;
            while (base < static_cast<int>(ys.size())) base <<= 1;
            seg.assign(base << 1, e());
        }

        void set_value(int pos, S value) {
            assert(0 <= pos && pos < static_cast<int>(ys.size()));
            int k = base + pos;
            seg[k] = value;
            while (k > 1) {
                k >>= 1;
                seg[k] = op(seg[k << 1], seg[k << 1 | 1]);
            }
        }

        S get_value(int pos) const {
            assert(0 <= pos && pos < static_cast<int>(ys.size()));
            return seg[base + pos];
        }

        S prod_value(int l, int r) const {
            S sml = e();
            S smr = e();
            for (l += base, r += base; l < r; l >>= 1, r >>= 1) {
                if (l & 1) sml = op(sml, seg[l++]);
                if (r & 1) smr = op(seg[--r], smr);
            }
            return op(sml, smr);
        }
    };

    int n_ = 0;
    int size_ = 1;
    std::vector<long long> xs_;
    std::vector<std::pair<long long, long long>> points_;
    std::vector<S> point_values_;
    std::vector<Node> nodes_;

    static int lower_bound_index(const std::vector<long long>& a, long long x) {
        return static_cast<int>(std::lower_bound(a.begin(), a.end(), x) - a.begin());
    }

    int point_index(long long x, long long y) const {
        auto it = std::lower_bound(points_.begin(), points_.end(), std::make_pair(x, y));
        assert(it != points_.end() && *it == std::make_pair(x, y));
        return static_cast<int>(it - points_.begin());
    }

    int x_index(long long x) const {
        int xi = lower_bound_index(xs_, x);
        assert(xi < static_cast<int>(xs_.size()) && xs_[xi] == x);
        return xi;
    }

    int y_pos(int node, long long y) const {
        const auto& ys = nodes_[node].ys;
        auto it = std::lower_bound(ys.begin(), ys.end(), y);
        if (it == ys.end() || *it != y) return -1;
        return static_cast<int>(it - ys.begin());
    }

    S node_y_value(int node, long long y) const {
        int pos = y_pos(node, y);
        if (pos == -1) return e();
        return nodes_[node].get_value(pos);
    }

public:
    StaticSegTree2D() = default;

    explicit StaticSegTree2D(const std::vector<std::pair<long long, long long>>& points) {
        build(points);
    }

    void build(const std::vector<std::pair<long long, long long>>& points) {
        points_ = points;
        std::sort(points_.begin(), points_.end());
        points_.erase(std::unique(points_.begin(), points_.end()), points_.end());

        xs_.clear();
        xs_.reserve(points_.size());
        for (const auto& [x, y] : points_) {
            (void)y;
            xs_.push_back(x);
        }
        xs_.erase(std::unique(xs_.begin(), xs_.end()), xs_.end());

        n_ = static_cast<int>(xs_.size());
        size_ = 1;
        while (size_ < n_) size_ <<= 1;
        nodes_.assign(size_ << 1, Node{});

        std::vector<std::vector<long long>> leaf_ys(n_);
        for (const auto& [x, y] : points_) {
            int xi = x_index(x);
            leaf_ys[xi].push_back(y);
        }
        for (int i = 0; i < n_; ++i) {
            auto& ys = leaf_ys[i];
            std::sort(ys.begin(), ys.end());
            ys.erase(std::unique(ys.begin(), ys.end()), ys.end());
            nodes_[size_ + i].ys = std::move(ys);
        }
        for (int i = size_ - 1; i >= 1; --i) {
            const auto& a = nodes_[i << 1].ys;
            const auto& b = nodes_[i << 1 | 1].ys;
            nodes_[i].ys.resize(a.size() + b.size());
            auto it = std::set_union(a.begin(), a.end(), b.begin(), b.end(), nodes_[i].ys.begin());
            nodes_[i].ys.erase(it, nodes_[i].ys.end());
        }
        for (auto& node : nodes_) node.init();
        point_values_.assign(points_.size(), e());
    }

    void set(long long x, long long y, S value) {
        int pid = point_index(x, y);
        point_values_[pid] = value;

        int node = size_ + x_index(x);
        int pos = y_pos(node, y);
        assert(pos != -1);
        nodes_[node].set_value(pos, value);

        while (node > 1) {
            node >>= 1;
            int parent_pos = y_pos(node, y);
            assert(parent_pos != -1);
            S lv = node_y_value(node << 1, y);
            S rv = node_y_value(node << 1 | 1, y);
            nodes_[node].set_value(parent_pos, op(lv, rv));
        }
    }

    void add(long long x, long long y, S delta) {
        set(x, y, get(x, y) + delta);
    }

    S get(long long x, long long y) const {
        return point_values_[point_index(x, y)];
    }

    S prod(long long xl, long long xr, long long yl, long long yr) const {
        if (xl >= xr || yl >= yr || points_.empty()) return e();
        int l = lower_bound_index(xs_, xl);
        int r = lower_bound_index(xs_, xr);
        S sml = e();
        S smr = e();
        for (l += size_, r += size_; l < r; l >>= 1, r >>= 1) {
            if (l & 1) {
                const auto& ys = nodes_[l].ys;
                int yl_pos = lower_bound_index(ys, yl);
                int yr_pos = lower_bound_index(ys, yr);
                sml = op(sml, nodes_[l].prod_value(yl_pos, yr_pos));
                ++l;
            }
            if (r & 1) {
                --r;
                const auto& ys = nodes_[r].ys;
                int yl_pos = lower_bound_index(ys, yl);
                int yr_pos = lower_bound_index(ys, yr);
                smr = op(nodes_[r].prod_value(yl_pos, yr_pos), smr);
            }
        }
        return op(sml, smr);
    }
};

template <class S, S (*op)(S, S), S (*e)()>
struct DenseSegTree2D {
private:
    int h_ = 0;
    int w_ = 0;
    int size_h_ = 1;
    int size_w_ = 1;
    std::vector<std::vector<S>> seg_;

    S prod_y(int node, int l, int r) const {
        S sml = e();
        S smr = e();
        for (l += size_w_, r += size_w_; l < r; l >>= 1, r >>= 1) {
            if (l & 1) sml = op(sml, seg_[node][l++]);
            if (r & 1) smr = op(seg_[node][--r], smr);
        }
        return op(sml, smr);
    }

public:
    DenseSegTree2D() = default;

    DenseSegTree2D(int h, int w) {
        build(h, w);
    }

    explicit DenseSegTree2D(const std::vector<std::vector<S>>& grid) {
        build(grid);
    }

    void build(int h, int w) {
        assert(h >= 0 && w >= 0);
        h_ = h;
        w_ = w;
        size_h_ = 1;
        size_w_ = 1;
        while (size_h_ < h_) size_h_ <<= 1;
        while (size_w_ < w_) size_w_ <<= 1;
        seg_.assign(size_h_ << 1, std::vector<S>(size_w_ << 1, e()));
    }

    void build(const std::vector<std::vector<S>>& grid) {
        int h = static_cast<int>(grid.size());
        int w = h == 0 ? 0 : static_cast<int>(grid[0].size());
        for (const auto& row : grid) assert(static_cast<int>(row.size()) == w);

        build(h, w);
        for (int i = 0; i < h_; ++i) {
            for (int j = 0; j < w_; ++j) seg_[size_h_ + i][size_w_ + j] = grid[i][j];
            for (int j = size_w_ - 1; j >= 1; --j) {
                seg_[size_h_ + i][j] = op(seg_[size_h_ + i][j << 1], seg_[size_h_ + i][j << 1 | 1]);
            }
        }
        for (int i = size_h_ - 1; i >= 1; --i) {
            for (int j = 1; j < (size_w_ << 1); ++j) {
                seg_[i][j] = op(seg_[i << 1][j], seg_[i << 1 | 1][j]);
            }
        }
    }

    void set(int i, int j, S value) {
        assert(0 <= i && i < h_);
        assert(0 <= j && j < w_);
        int x = i + size_h_;
        int y = j + size_w_;
        seg_[x][y] = value;
        for (int col = y >> 1; col >= 1; col >>= 1) {
            seg_[x][col] = op(seg_[x][col << 1], seg_[x][col << 1 | 1]);
            if (col == 1) break;
        }
        for (x >>= 1; x >= 1; x >>= 1) {
            seg_[x][y] = op(seg_[x << 1][y], seg_[x << 1 | 1][y]);
            for (int col = y >> 1; col >= 1; col >>= 1) {
                seg_[x][col] = op(seg_[x][col << 1], seg_[x][col << 1 | 1]);
                if (col == 1) break;
            }
            if (x == 1) break;
        }
    }

    S get(int i, int j) const {
        assert(0 <= i && i < h_);
        assert(0 <= j && j < w_);
        return seg_[i + size_h_][j + size_w_];
    }

    S prod(int il, int ir, int jl, int jr) const {
        assert(0 <= il && il <= ir && ir <= h_);
        assert(0 <= jl && jl <= jr && jr <= w_);
        if (il == ir || jl == jr) return e();
        S sml = e();
        S smr = e();
        for (il += size_h_, ir += size_h_; il < ir; il >>= 1, ir >>= 1) {
            if (il & 1) sml = op(sml, prod_y(il++, jl, jr));
            if (ir & 1) smr = op(prod_y(--ir, jl, jr), smr);
        }
        return op(sml, smr);
    }
};
