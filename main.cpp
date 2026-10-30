#include <algorithm>
#include <array>
#include <atcoder/all>
#include <bitset>
#include <boost/multiprecision/cpp_int.hpp>
#include <cassert>
#include <chrono>
#include <cmath>
#include <complex>
#include <cstring>
#include <ctime>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <map>
#include <queue>
#include <random>
#include <set>
#include <stack>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>
using namespace std;
using namespace atcoder;
using namespace boost::multiprecision;
using ll = long long;
using ld = long double;
using P = pair<ll, ll>;
using mint = modint998244353;
#define int long long
#define rep(i, s, n) for (ll i = (s); i < (ll)(n); i++)
#define all(a) (a).begin(), (a).end()
#define rall(a) (a).rbegin(), (a).rend()
const ll INF = 1e18;
const int dx[8] = {0, 1, 0, -1, -1, -1, 1, 1};
const int dy[8] = {1, 0, -1, 0, -1, 1, -1, 1};
template <typename T> inline bool chmax(T &a, T b) {
  return ((a < b) ? (a = b, true) : (false));
}
template <typename T> inline bool chmin(T &a, T b) {
  return ((a > b) ? (a = b, true) : (false));
}
void yn(bool t) { cout << (t ? "Yes" : "No") << endl; }
uint64_t seed = chrono::duration_cast<chrono::nanoseconds>(
                    chrono::high_resolution_clock::now().time_since_epoch())
                    .count();
mt19937_64 rng(seed);
int randint(int l, int r) { return uniform_int_distribution<int>(l, r)(rng); }

bool f(const P &a, const P &b) {
  return (a.first * b.second - a.second * b.first) > 0;
}

int solve() {
  int n;
  cin >> n;
  set<P> s;
  vector<P> v(n);
  int ans = 0;
  int now = 0;
  rep(i, 0, n) {
    int x, y;
    cin >> x >> y;
    bool tf = false;
    if (y < 0 || (y == 0 && x < 0)) {
      tf = true;
      x = -x;
      y = -y;
    }
    v[i].first = x;
    v[i].second = y;
    if (!tf) {
      continue;
    }
    s.insert({x, y});
    now++;
  }
  now = n - now;
  ans = max(now, n - now);
  sort(v.begin(), v.end(), f);
  rep(i, 0, n) {
    if (s.count(v[i])) {
      now++;
    } else {
      now--;
    }
    ans = max({ans, now, n - now});
  }
  cout << ans << endl;
  return 0;
}

signed main() {
#ifdef INPUT_FILE_PATH
  freopen(INPUT_FILE_PATH, "r", stdin);
#endif
  ios::sync_with_stdio(false);
  cin.tie(nullptr);
  cout << fixed << setprecision(15);
  int t = 1;
  cin >> t;
  while (t--) {
    solve();
  }
  return 0;
}
