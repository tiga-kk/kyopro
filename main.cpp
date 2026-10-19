#include <algorithm>
#include <array>
#include <atcoder/all>
#include <bitset>
#include <boost/multiprecision/cpp_int.hpp>
#include <cassert>
#include <chrono>
#include <cmath>
#include <complex>
#include <cstdio>
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

template <class T> T ceil(T a, T b) {
  assert(b != 0);
  T q = a / b;
  T r = a % b;
  if (r != 0 && ((r > 0) == (b > 0))) {
    q++;
  }
  return q;
}

int solve() {
  int n, q;
  cin >> n >> q;
  vector<int> a(n + 1, 0);
  rep(i, 1, n + 1) cin >> a[i];
  vector<int> sum1(n + 1, 0);
  vector<int> sum2(n + 1, 0);
  vector<int> idx(n + 1, 0);
  rep(i, 1, n + 1) {
    sum1[i] = sum1[i - 1] + a[i];
    sum2[i] = sum2[i - 1] + sum1[i];
    idx[i] = idx[i - 1] + i;
  }

  while (q--) {
    int l, r;
    cin >> l >> r;
    auto itrl = lower_bound(all(idx), l);
    auto itrr = lower_bound(all(idx), r);
    int idxl = itrl - idx.begin();
    int idxr = itrr - idx.begin();
    int ans = 0;
    ans += sum2[idxr] - sum2[idxl - 1];
    ans -= sum1[l - 1 - idxl * (idxl - 1) / 2];
    ans -= sum1[idxr] - sum1[r - idxr * (idxr - 1) / 2];
    cout << ans << endl;
  }
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
  // cin>>t;
  while (t--) {
    solve();
  }
  return 0;
}