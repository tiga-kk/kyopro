#pragma once

#include <bits/stdc++.h>

// Example: long long op(long long a, long long b) { return a + b; }
//          long long e() { return 0; }
//          Trie<long long, op, e> t; t.apply("apple", 1);
//          t.prefix_prod("app"); // total stored value under "app"
// Ordered string -> S map. op must be associative with identity e().
// Aggregates follow lexicographic order (a key precedes its extensions).
// count/size/kth count distinct registered keys, including values equal to e().
// Alphabet consists of consecutive bytes starting at Base. Empty keys are valid.
// O(Alphabet * length) updates/rank/kth, O(length) lookup/prefix queries.
// Erase retains nodes: memory is O(Alphabet * total distinct prefixes ever created).
template <class S, S (*op)(S, S), S (*e)(),
          int Alphabet = 26, char Base = 'a'>
struct Trie {
    static_assert(Alphabet > 0 && static_cast<unsigned char>(Base) + Alphabet <= 256);
private:
    struct Node {
        std::array<int, Alphabet> child;
        bool terminal = false;
        int count = 0;
        S value = e(), prod = e();
        Node() { child.fill(-1); }
    };
    std::vector<Node> nodes_{Node{}};

    static int index(char c) {
        int i = static_cast<unsigned char>(c) - static_cast<unsigned char>(Base);
        assert(0 <= i && i < Alphabet);
        return i;
    }
    int find(std::string_view key) const {
        int u = 0;
        for (char c : key) {
            u = nodes_[u].child[index(c)];
            if (u == -1) return -1;
        }
        return u;
    }
    void pull(int u) {
        auto& node = nodes_[u];
        node.count = node.terminal;
        node.prod = node.terminal ? node.value : e();
        for (int v : node.child) if (v != -1) {
            node.count += nodes_[v].count;
            node.prod = op(node.prod, nodes_[v].prod);
        }
    }
public:
    int size() const { return nodes_[0].count; }
    bool empty() const { return size() == 0; }
    bool contains(std::string_view key) const {
        int u = find(key);
        return u != -1 && nodes_[u].terminal;
    }
    S get(std::string_view key) const {
        int u = find(key);
        return u != -1 && nodes_[u].terminal ? nodes_[u].value : e();
    }
    void set(std::string_view key, S value) {
        std::vector<int> path{0};
        for (char c : key) {
            int u = path.back(), i = index(c);
            if (nodes_[u].child[i] == -1) {
                int v = static_cast<int>(nodes_.size());
                nodes_.emplace_back();
                nodes_[u].child[i] = v;
            }
            path.push_back(nodes_[u].child[i]);
        }
        nodes_[path.back()].terminal = true;
        nodes_[path.back()].value = value;
        for (auto it = path.rbegin(); it != path.rend(); ++it) pull(*it);
    }
    void apply(std::string_view key, S value) { set(key, op(get(key), value)); }
    bool erase(std::string_view key) {
        std::vector<int> path{0};
        for (char c : key) {
            int v = nodes_[path.back()].child[index(c)];
            if (v == -1) return false;
            path.push_back(v);
        }
        if (!nodes_[path.back()].terminal) return false;
        nodes_[path.back()].terminal = false;
        nodes_[path.back()].value = e();
        for (auto it = path.rbegin(); it != path.rend(); ++it) pull(*it);
        return true;
    }
    int prefix_count(std::string_view prefix) const {
        int u = find(prefix);
        return u == -1 ? 0 : nodes_[u].count;
    }
    S prefix_prod(std::string_view prefix) const {
        int u = find(prefix);
        return u == -1 ? e() : nodes_[u].prod;
    }
    S all_prod() const { return nodes_[0].prod; }
    // Number of registered keys strictly smaller than key.
    int rank(std::string_view key) const {
        int u = 0, result = 0;
        for (char c : key) {
            result += nodes_[u].terminal;
            int i = index(c);
            for (int j = 0; j < i; ++j) {
                int v = nodes_[u].child[j];
                if (v != -1) result += nodes_[v].count;
            }
            u = nodes_[u].child[i];
            if (u == -1) break;
        }
        return result;
    }
    // 0-indexed, optionally within the given prefix.
    std::string kth(int k, std::string_view prefix = "") const {
        int u = find(prefix);
        assert(u != -1 && 0 <= k && k < nodes_[u].count);
        std::string result(prefix);
        while (true) {
            if (nodes_[u].terminal) {
                if (k == 0) return result;
                --k;
            }
            for (int i = 0; i < Alphabet; ++i) {
                int v = nodes_[u].child[i];
                if (v == -1) continue;
                if (k >= nodes_[v].count) { k -= nodes_[v].count; continue; }
                result.push_back(static_cast<char>(static_cast<unsigned char>(Base) + i));
                u = v;
                break;
            }
        }
    }
    std::optional<std::string> lower_bound(std::string_view key) const {
        int k = rank(key);
        if (k == size()) return std::nullopt;
        return kth(k);
    }
    // Maximum common-prefix length with any registered key. Empty trie -> 0.
    int lcp(std::string_view key) const {
        int u = 0, length = 0;
        for (char c : key) {
            int v = nodes_[u].child[index(c)];
            if (v == -1 || nodes_[v].count == 0) break;
            u = v;
            ++length;
        }
        return length;
    }
    // Length of the longest registered key that is a prefix of key.
    std::optional<int> longest_prefix(std::string_view key) const {
        std::optional<int> result;
        int u = 0, length = 0;
        if (nodes_[u].terminal) result = 0;
        for (char c : key) {
            u = nodes_[u].child[index(c)];
            if (u == -1) break;
            ++length;
            if (nodes_[u].terminal) result = length;
        }
        return result;
    }
};
