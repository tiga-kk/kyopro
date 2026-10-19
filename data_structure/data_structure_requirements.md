# 競プロ用データ構造ライブラリ 実装要件まとめ

## 基本方針

1つの万能データ構造に全部を詰め込まず、用途ごとに分ける。

理由は、以下の機能群が互いに自然には両立しないため。

- `split / merge / insert / erase / reverse` は implicit treap が得意。
- `sum / min / max / add / assign` は lazy propagation が得意。
- `chmin / chmax` は Segment Tree Beats が必要。
- `kth / frequency / distinct / mex` は Wavelet Matrix 系が得意。
- 2次元クエリは 2D Segment Tree や Fenwick Tree of Segment Tree 系が得意。

特に、以下は同じライブラリに混ぜない方がよい。

- `chmin/chmax` と汎用 `prod`
- `chmin/chmax` と `mul`
- `kth/distinct/mex` と `add/assign/reverse`
- Wavelet Matrix 的機能と動的列操作全般

---

# 実装予定一覧

## 1. NumericImplicitTreap

### 目的

動的な数列を扱うための、数値専用 implicit treap。

配列の途中への挿入・削除・区間反転・区間加算・区間代入・区間集約を高速に行う。

### 位置づけ

最も実用的な普段使い用。

`GenericImplicitTreap` より汎用性は低いが、定数倍が軽く、競プロ本番で使いやすい。

### 対応機能

- `size()`
- `empty()`
- `insert(pos, x)`
- `erase(pos)`
- `get(pos)`
- `set_value(pos, x)`
- `push_front(x)`
- `push_back(x)`
- `pop_front()`
- `pop_back()`
- `split_prefix(k)`
- `cut(l, r)`
- `insert_treap(pos, other)`
- `append(other)`
- `move(l, r, pos_after_erase)`
- `reverse(l, r)`
- `range_add(l, r, x)`
- `range_assign(l, r, x)`
- `range_sum(l, r)`
- `range_min(l, r)`
- `range_max(l, r)`
- `range_info(l, r)`
- `max_right(l, pred)`
- `min_left(r, pred)`
- `lower_bound_prefix_sum(x)`
- `to_vector()`

### 区間仕様

すべて 0-indexed。

区間は半開区間 `[l, r)`。

### 内部ノード情報

```cpp
struct Node {
    Node *l, *r;
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
};
```

### 遅延伝播

対応する lazy:

- reverse
- assign
- add

伝播順序:

1. reverse
2. assign
3. add

`assign` は `add` より強い。

- `assign(x)` の後に `add(y)` → `assign(x + y)`
- `add(y)` の後に `assign(x)` → `assign(x)`

### 計算量

期待計算量:

- `insert`: `O(log N)`
- `erase`: `O(log N)`
- `reverse`: `O(log N)`
- `range_add`: `O(log N)`
- `range_assign`: `O(log N)`
- `range_sum/min/max`: `O(log N)`
- `cut`: `O(log N)`
- `move`: `O(log N)`
- `to_vector`: `O(N)`

### 注意点

`lower_bound_prefix_sum` は全要素が非負である場合にだけ正しい。

`max_right/min_left` は predicate が単調である場合にだけ正しい。

`range_min/range_max` は空区間を原則許さない。

### 今後の改善候補

- O(N) build
- メモリプール
- `rotate(l, r, k)` は今回は不要なので実装しない
- デバッグ用 `dump()`

---

## 2. GenericImplicitTreap

### 目的

モノイドと遅延写像を外から渡せる、汎用 implicit treap。

AtCoder Library の `lazy_segtree` に `split/merge/insert/erase/reverse` を加えたような位置づけ。

### 位置づけ

広い問題に流用するための汎用版。

ただし、数値専用処理では `NumericImplicitTreap` より定数倍が重い。

### テンプレート案

```cpp
template <
    class S,
    S (*op)(S, S),
    S (*e)(),
    class F,
    S (*mapping)(F, S),
    F (*composition)(F, F),
    F (*id)()
>
struct GenericImplicitTreap;
```

または関数オブジェクト版にして、インライン化されやすくする。

### 対応機能

- `size()`
- `empty()`
- `insert(pos, x)`
- `erase(pos)`
- `get(pos)`
- `set(pos, x)`
- `split_prefix(k)`
- `cut(l, r)`
- `insert_treap(pos, other)`
- `append(other)`
- `move(l, r, pos_after_erase)`
- `reverse(l, r)`
- `range_apply(l, r, f)`
- `range_prod(l, r)`
- `max_right(l, pred)`
- `min_left(r, pred)`
- `to_vector()`

### reverse と非可換 prod

`op` が非可換な場合、反転時に順方向集約だけでは壊れる。

そのため、各ノードに以下を持つ。

```cpp
S prod;
S rprod;
```

更新式:

```cpp
prod  = op(prod(left), op(val, prod(right)));
rprod = op(rprod(right), op(val, rprod(left)));
```

`reverse` 時:

```cpp
swap(left, right);
swap(prod, rprod);
rev ^= 1;
```

### 得意な用途

- 区間和
- 区間 affine
- modint 上の区間 affine/sum
- 文字列連結
- ローリングハッシュ
- 行列積
- 順序が重要な区間 prod

### 苦手・非対応

- `chmin/chmax`
- 一般値域の `kth`
- 一般値域の `distinct`
- 一般値域の `mex`

### 注意点

`range_add + sum/min/max` は表現可能だが、数値専用なら `NumericImplicitTreap` の方が軽い。

---

## 3. ImplicitTreapBeats

### 目的

動的列に Segment Tree Beats を載せ、`chmin/chmax/add/sum/min/max` を扱う。

### 位置づけ

特殊用途。

強力だが実装が重く、定数倍も重い。

本当に `split/merge/insert/erase` と `chmin/chmax` が同時に必要な場合だけ使う。

### 対応候補

- `split`
- `merge`
- `insert`
- `erase`
- `range_add`
- `range_chmin`
- `range_chmax`
- `range_sum`
- `range_min`
- `range_max`
- `get`
- `set`
- `to_vector`

### reverse について

`sum/min/max` は順序非依存なので理論上 `reverse` と相性は悪くない。

ただし `push` が複雑になり、バグの原因になる。

初期版では `reverse` は入れないか、後から慎重に追加する。

### mul について

初期版では入れない。

理由:

- `mul < 0` で大小関係が反転する。
- `chmin/chmax` と `mul` の合成が壊れやすい。
- 定数倍と実装複雑度が上がる。

どうしても入れるなら `mul >= 0` 限定にする。

### 内部ノード情報

```cpp
struct Node {
    Node *l, *r;
    uint32_t pri;
    int sz;

    long long val;
    long long sum;

    long long max1, max2;
    int maxc;

    long long min1, min2;
    int minc;

    long long lazy_add;
};
```

### Beats の基本条件

`chmin(x)`:

- `max1 <= x` なら何もしない。
- `max2 < x < max1` なら最大値だけを `x` に下げる。
- `x <= max2` なら子に降りる。

`chmax(x)` はその逆。

### 注意点

`GenericImplicitTreap` とは別物にする。

汎用 prod とは混ぜない。

本番用ライブラリとしては、必要になるまで実装しなくてよい。

---

## 4. WaveletMatrix

### 目的

静的配列に対して、値の順位・頻度・mex 系クエリを高速に答える。

implicit treap の代わりではなく、静的配列用の別系統データ構造。

### 位置づけ

区間 kth / frequency / mex 用。

配列が変化しない、または変更がない問題で使う。

### 対応機能

- `access(i)`
- `rank(x, r)`
- `rank(x, l, r)`
- `select(x, k)`
- `kth_smallest(l, r, k)`
- `kth_largest(l, r, k)`
- `range_freq(l, r, upper)`
- `range_freq(l, r, lower, upper)`
- `prev_value(l, r, upper)`
- `next_value(l, r, lower)`
- `range_min(l, r)`
- `range_max(l, r)`
- `range_mex(l, r)` 可能なら実装
- `topk(l, r, k)` は必要なら追加

### 値の扱い

`long long` または `uint64_t` を扱う。

実装方針は2択。

1. 非圧縮版
   - 64bit 値をそのまま扱う。
   - 実装はやや単純。
   - 計算量は `O(64)`。

2. 座標圧縮版
   - 値集合を圧縮して扱う。
   - `range_freq` や `kth` は速い。
   - 元値への復元配列が必要。

競プロ用には、まず座標圧縮版を実装するのが実用的。

### 内部構造

各 bit 段ごとに bit vector を持つ。

必要な bit vector 操作:

- `rank0(pos)`
- `rank1(pos)`
- `access(pos)` 用の bit 取得

簡易実装では `vector<int> prefix_ones` を持つ。

高速版では succinct bit vector を実装してもよい。

### 計算量

`σ` を値の種類数とする。

座標圧縮版:

- 構築: `O(N log σ)`
- `access`: `O(log σ)`
- `rank`: `O(log σ)`
- `kth`: `O(log σ)`
- `range_freq`: `O(log σ)`
- `prev/next`: `O(log σ)`

### 得意な用途

- 区間 k 番目
- 区間内で `x` 未満の個数
- 区間内で `[L, R)` に入る個数
- 区間内の最小値・最大値
- 静的 mex

### 苦手・非対応

- `insert`
- `erase`
- `reverse`
- `range_add`
- `range_assign`

### 注意点

implicit treap に Wavelet Matrix 的機能を混ぜるのは基本的に避ける。

動的列で小さい値域なら、別途 `SmallAlphabetImplicitTreap` を考える余地はあるが、今回は実装しない。

---

## 5. 2D Segment Tree

### 目的

2次元平面上の点またはグリッドに対して、矩形クエリを処理する。

### 位置づけ

2次元範囲クエリ用。

Wavelet Matrix と同じく、implicit treap とは別系統。

## 5-A. Static 2D Segment Tree

### 目的

点集合が静的で、点更新または矩形集約を行う。

主に座標圧縮された点集合向け。

### 対応機能

- `build(points)`
- `set(x, y, value)`
- `add(x, y, delta)`
- `prod(xl, xr, yl, yr)`

### 集約

モノイドで汎用化する。

```cpp
template <class S, S (*op)(S, S), S (*e)()>
struct SegTree2D;
```

対応例:

- sum
- min
- max
- gcd
- xor

### 座標仕様

半開区間:

```text
[xl, xr) × [yl, yr)
```

点集合は事前に全列挙しておく。

### 内部構造

外側 x 方向にセグ木を持つ。

各 x ノードごとに、その範囲に含まれる y 座標を圧縮した内側セグ木を持つ。

### 計算量

`N` を点数とする。

- 構築: `O(N log N)`
- 点更新: `O(log^2 N)`
- 矩形クエリ: `O(log^2 N)`
- 空間: `O(N log N)`

### 注意点

事前に現れる点を全て知っている必要がある。

完全動的な新規座標追加には向かない。

## 5-B. Dense Grid 2D Segment Tree

### 目的

`H × W` のグリッド全体を扱う。

### 対応機能

- `set(i, j, value)`
- `prod(il, ir, jl, jr)`

### 計算量

- 構築: `O(HW)`
- 点更新: `O(log H log W)`
- 矩形クエリ: `O(log H log W)`
- 空間: `O(HW)`

### 注意点

`H*W` が大きい場合は使えない。

スパースな点集合なら Static 2D Segment Tree を使う。

---

# kth / distinct / mex について

## implicit treap に自然に混ぜられるか

一般値域では混ぜない方がよい。

理由:

- kth は部分木内の値分布が必要。
- distinct は集合情報が必要。
- mex も集合情報が必要。
- これらは `pull` が `O(1)` では済まなくなる。

その結果、implicit treap の長所である軽さが消える。

## 値域が小さい場合

値域が小さいなら別ライブラリとして `SmallAlphabetImplicitTreap` が可能。

ただし今回は実装しない。

可能な設計:

- `uint64_t mask`
- `bitset<V>`
- `array<int, V> freq`

対応できるもの:

- distinct
- mex
- frequency
- 種類としての kth
- 値域が小さい場合の重複込み kth

ただし `range_add` とは相性が悪い。

---

# 実装優先順位

## 優先度 1

`NumericImplicitTreap`

理由:

- 本番で使いやすい。
- 定数倍が比較的軽い。
- 動的列問題に広く対応できる。

現在、基本版は実装済み。

追加済み:

- `range_assign`
- `max_right`
- `min_left`
- `lower_bound_prefix_sum`
- `move`

未追加だが候補:

- O(N) build
- メモリプール

## 優先度 2

`WaveletMatrix`

理由:

- kth/frequency/mex 系を担当する。
- implicit treap と役割が明確に違う。
- 静的配列問題で非常に強い。

## 優先度 3

`2D Segment Tree`

理由:

- 2次元矩形クエリ用。
- 点集合が静的な場合に有用。
- 実装は Wavelet Matrix より独立している。

## 優先度 4

`GenericImplicitTreap`

理由:

- 汎用 prod が必要な問題に使える。
- ただし NumericImplicitTreap より定数倍が重い。
- 実装難易度は中程度。

## 優先度 5

`ImplicitTreapBeats`

理由:

- 強いが重い。
- 実装バグの危険が高い。
- 必要になる頻度は高くない。
- `chmin/chmax` が必要な問題が出てからでもよい。

---

# 使い分け基準

## 動的列操作がある

使う候補:

- `NumericImplicitTreap`
- `GenericImplicitTreap`

例:

- 挿入
- 削除
- reverse
- cut and paste
- merge/split

## 区間 add/assign/sum/min/max がある

使う候補:

- `NumericImplicitTreap`

## 区間 affine や文字列・行列などの汎用 prod がある

使う候補:

- `GenericImplicitTreap`

## 区間 chmin/chmax がある

使う候補:

- 通常の Segment Tree Beats
- 動的列が必要なら `ImplicitTreapBeats`

## 静的配列で kth/frequency/mex がある

使う候補:

- `WaveletMatrix`

## 2次元矩形クエリがある

使う候補:

- `2D Segment Tree`
- 場合によっては 2D Fenwick Tree

---

# 実装しない・混ぜないもの

## NumericImplicitTreap に入れない

- `chmin`
- `chmax`
- `mul`
- `kth`
- `distinct`
- `mex`

## GenericImplicitTreap に入れない

- `chmin`
- `chmax`
- 一般値域の `kth`
- 一般値域の `distinct`
- 一般値域の `mex`

## ImplicitTreapBeats に入れない

- 汎用 prod
- `mul`
- Wavelet Matrix 的機能

## WaveletMatrix に入れない

- `insert`
- `erase`
- `reverse`
- `range_add`
- `range_assign`

---

# 推奨ファイル構成

```text
data_structure/
  implicit_treap_numeric.hpp
  implicit_treap_generic.hpp
  implicit_treap_beats.hpp
  wavelet_matrix.hpp
  segtree_2d.hpp
```

最初に完成させる順番:

```text
1. implicit_treap_numeric.hpp
2. wavelet_matrix.hpp
3. segtree_2d.hpp
4. implicit_treap_generic.hpp
5. implicit_treap_beats.hpp
```

---

# 現時点の結論

まず本番で使える核として `NumericImplicitTreap` を固める。

次に、kth/frequency/mex 用に `WaveletMatrix` を実装する。

その後、2次元矩形クエリ用に `2D Segment Tree` を実装する。

`GenericImplicitTreap` と `ImplicitTreapBeats` は、必要性が高まった段階で追加する。

特に `ImplicitTreapBeats` は、万能化を狙わず、`chmin/chmax/add/sum/min/max` 専用として作る。
