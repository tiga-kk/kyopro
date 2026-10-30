#include "../data_structure/trie.hpp"
using namespace std;
string cat(string a, string b) { return a + b; }
string trie_e() { return ""; }
using T = Trie<string, cat, trie_e, 3, 'a'>;

int main() {
    T trie;
    map<string, string> ref;
    vector<string> keys{""};
    for (int i = 0; i < 120; ++i) {
        string s;
        int x = i;
        do { s += char('a' + x % 3); x /= 3; } while (x);
        keys.push_back(s);
    }
    mt19937 rng(42);
    for (int step = 0; step < 1000; ++step) {
        string key = keys[rng() % keys.size()];
        string value = rng() % 5 == 0 ? "" : to_string(step) + ",";
        switch (rng() % 3) {
            case 0: trie.set(key, value); ref[key] = value; break;
            case 1: trie.apply(key, value); ref[key] += value; break;
            case 2: assert(trie.erase(key) == bool(ref.erase(key))); break;
        }
        assert(trie.size() == int(ref.size()));
        assert(trie.empty() == ref.empty());
        string all;
        int k = 0;
        for (const auto& [s, v] : ref) {
            all += v;
            assert(trie.kth(k++) == s);
        }
        assert(trie.all_prod() == all);
        for (int query = 0; query < 8; ++query) {
            string s = keys[rng() % keys.size()];
            assert(trie.contains(s) == ref.contains(s));
            assert(trie.get(s) == (ref.contains(s) ? ref.at(s) : ""));
            int count = 0, rank = 0, lcp = 0;
            optional<int> longest;
            string prod;
            vector<string> matching;
            for (const auto& [t, v] : ref) {
                rank += t < s;
                int common = 0;
                while (common < int(min(s.size(), t.size())) && s[common] == t[common]) ++common;
                lcp = max(lcp, common);
                if (s.starts_with(t) && (!longest || *longest < int(t.size()))) longest = int(t.size());
                if (t.starts_with(s)) { ++count; prod += v; matching.push_back(t); }
            }
            assert(trie.prefix_count(s) == count);
            assert(trie.prefix_prod(s) == prod);
            assert(trie.rank(s) == rank);
            assert(trie.lcp(s) == lcp);
            assert(trie.longest_prefix(s) == longest);
            auto it = ref.lower_bound(s);
            assert(trie.lower_bound(s) == (it == ref.end() ? optional<string>{} : optional<string>{it->first}));
            for (int i = 0; i < count; ++i) assert(trie.kth(i, s) == matching[i]);
        }
    }
    Trie<string, cat, trie_e, 10, '0'> digits;
    digits.set("123", "x");
    assert(digits.kth(0) == "123");
    // Iterative operations must handle long keys without recursive destruction.
    T deep;
    string long_key(100000, 'a');
    deep.set(long_key, "x");
    assert(deep.get(long_key) == "x");
    assert(deep.erase(long_key));
    assert(deep.lcp(long_key) == 0);
    cout << "Trie tests passed\n";
}
