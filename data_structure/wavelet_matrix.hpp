#pragma once

#include <bits/stdc++.h>

struct WaveletMatrix {
private:
    struct BitVector {
        std::vector<int> prefix_ones;
        std::vector<unsigned char> bits;

        BitVector() = default;

        explicit BitVector(const std::vector<int>& src) : prefix_ones(src.size() + 1, 0), bits(src.size(), 0) {
            for (int i = 0; i < static_cast<int>(src.size()); ++i) {
                bits[i] = static_cast<unsigned char>(src[i]);
                prefix_ones[i + 1] = prefix_ones[i] + src[i];
            }
        }

        int rank1(int pos) const {
            return prefix_ones[pos];
        }

        int rank0(int pos) const {
            return pos - rank1(pos);
        }

        int access(int pos) const {
            return bits[pos];
        }
    };

    int n_ = 0;
    int levels_ = 1;
    std::vector<long long> values_;
    std::vector<BitVector> bit_vectors_;
    std::vector<int> mids_;
    std::vector<std::vector<int>> positions_;

    int lower_id(long long x) const {
        return static_cast<int>(std::lower_bound(values_.begin(), values_.end(), x) - values_.begin());
    }

    int exact_id(long long x) const {
        int id = lower_id(x);
        if (id == static_cast<int>(values_.size()) || values_[id] != x) return -1;
        return id;
    }

    int count_id(int id, int l, int r) const {
        if (id < 0 || id >= static_cast<int>(values_.size())) return 0;
        for (int level = 0; level < levels_; ++level) {
            int bit = (id >> (levels_ - 1 - level)) & 1;
            if (bit == 0) {
                l = bit_vectors_[level].rank0(l);
                r = bit_vectors_[level].rank0(r);
            } else {
                l = mids_[level] + bit_vectors_[level].rank1(l);
                r = mids_[level] + bit_vectors_[level].rank1(r);
            }
        }
        return r - l;
    }

    int count_less_id(int l, int r, int upper_id) const {
        if (l >= r || upper_id <= 0) return 0;
        if (upper_id >= static_cast<int>(values_.size())) return r - l;
        int res = 0;
        for (int level = 0; level < levels_; ++level) {
            int bit = (upper_id >> (levels_ - 1 - level)) & 1;
            int l0 = bit_vectors_[level].rank0(l);
            int r0 = bit_vectors_[level].rank0(r);
            if (bit == 0) {
                l = l0;
                r = r0;
            } else {
                res += r0 - l0;
                l = mids_[level] + bit_vectors_[level].rank1(l);
                r = mids_[level] + bit_vectors_[level].rank1(r);
            }
        }
        return res;
    }

public:
    WaveletMatrix() = default;

    explicit WaveletMatrix(const std::vector<long long>& a) : n_(static_cast<int>(a.size())), values_(a) {
        std::sort(values_.begin(), values_.end());
        values_.erase(std::unique(values_.begin(), values_.end()), values_.end());

        int sigma = std::max(1, static_cast<int>(values_.size()));
        while ((1ULL << levels_) < static_cast<unsigned long long>(sigma)) ++levels_;

        std::vector<int> cur(n_);
        positions_.assign(values_.size(), {});
        for (int i = 0; i < n_; ++i) {
            cur[i] = lower_id(a[i]);
            positions_[cur[i]].push_back(i);
        }

        bit_vectors_.reserve(levels_);
        mids_.reserve(levels_);
        for (int level = 0; level < levels_; ++level) {
            int shift = levels_ - 1 - level;
            std::vector<int> bits(n_, 0);
            std::vector<int> zero;
            std::vector<int> one;
            zero.reserve(n_);
            one.reserve(n_);
            for (int i = 0; i < n_; ++i) {
                int bit = (cur[i] >> shift) & 1;
                bits[i] = bit;
                if (bit == 0) {
                    zero.push_back(cur[i]);
                } else {
                    one.push_back(cur[i]);
                }
            }
            bit_vectors_.emplace_back(bits);
            mids_.push_back(static_cast<int>(zero.size()));
            cur.swap(zero);
            cur.insert(cur.end(), one.begin(), one.end());
        }
    }

    int size() const {
        return n_;
    }

    long long access(int i) const {
        assert(0 <= i && i < n_);
        int id = 0;
        int pos = i;
        for (int level = 0; level < levels_; ++level) {
            int bit = bit_vectors_[level].access(pos);
            id = (id << 1) | bit;
            if (bit == 0) {
                pos = bit_vectors_[level].rank0(pos);
            } else {
                pos = mids_[level] + bit_vectors_[level].rank1(pos);
            }
        }
        return values_[id];
    }

    int rank(long long x, int r) const {
        assert(0 <= r && r <= n_);
        return rank(x, 0, r);
    }

    int rank(long long x, int l, int r) const {
        assert(0 <= l && l <= r && r <= n_);
        return count_id(exact_id(x), l, r);
    }

    int select(long long x, int k) const {
        if (k < 0) return -1;
        int id = exact_id(x);
        if (id < 0 || k >= static_cast<int>(positions_[id].size())) return -1;
        return positions_[id][k];
    }

    long long kth_smallest(int l, int r, int k) const {
        assert(0 <= l && l < r && r <= n_);
        assert(0 <= k && k < r - l);
        int id = 0;
        for (int level = 0; level < levels_; ++level) {
            int l0 = bit_vectors_[level].rank0(l);
            int r0 = bit_vectors_[level].rank0(r);
            int zero_count = r0 - l0;
            id <<= 1;
            if (k < zero_count) {
                l = l0;
                r = r0;
            } else {
                id |= 1;
                k -= zero_count;
                l = mids_[level] + bit_vectors_[level].rank1(l);
                r = mids_[level] + bit_vectors_[level].rank1(r);
            }
        }
        return values_[id];
    }

    long long kth_largest(int l, int r, int k) const {
        assert(0 <= l && l < r && r <= n_);
        assert(0 <= k && k < r - l);
        return kth_smallest(l, r, r - l - 1 - k);
    }

    int range_freq(int l, int r, long long upper) const {
        assert(0 <= l && l <= r && r <= n_);
        return count_less_id(l, r, lower_id(upper));
    }

    int range_freq(int l, int r, long long lower, long long upper) const {
        assert(0 <= l && l <= r && r <= n_);
        if (lower >= upper) return 0;
        return range_freq(l, r, upper) - range_freq(l, r, lower);
    }

    std::optional<long long> prev_value(int l, int r, long long upper) const {
        assert(0 <= l && l <= r && r <= n_);
        int cnt = range_freq(l, r, upper);
        if (cnt == 0) return std::nullopt;
        return kth_smallest(l, r, cnt - 1);
    }

    std::optional<long long> next_value(int l, int r, long long lower) const {
        assert(0 <= l && l <= r && r <= n_);
        int cnt = range_freq(l, r, lower);
        if (cnt == r - l) return std::nullopt;
        return kth_smallest(l, r, cnt);
    }

    long long range_min(int l, int r) const {
        assert(0 <= l && l < r && r <= n_);
        return kth_smallest(l, r, 0);
    }

    long long range_max(int l, int r) const {
        assert(0 <= l && l < r && r <= n_);
        return kth_largest(l, r, 0);
    }

    // range_mex は要件上「可能なら」なので今回は未実装。
};
