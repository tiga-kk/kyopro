# AtCoder workspace for C++ and Python

AtCoderにできるだけ近いが、完全再現までは狙わない競プロ用ワークスペースです。

## 初回セットアップ

```bash
cd ~/projects
unzip atcoder-workspace-uv-input.zip
cd atcoder-workspace-uv-input
./scripts/setup-server.sh
```

## 実行

```bash
make run-cpp      # 高速ローカルビルド（O0、重いデバッグ検査なし）
make debug-cpp    # _GLIBCXX_DEBUG とデバッグ情報付き
make run-opt-cpp  # AtCoderに近い O2 ビルド
make run-py       # uv run python main.py
make run-pypy     # pypy3 main.py
```

`main.cpp` と `main.py` は、同じディレクトリに `input.txt` があれば自動で読みます。
AtCoder提出時は通常 `input.txt` が存在しないため、標準入力を読みます。

## 方針

- Python環境は `uv` で管理します。
- C++は `g++-15`、`-std=gnu++23`、`-O2`、`-DATCODER`、`-DONLINE_JUDGE` に寄せます。
- C++のlanguage serverは `clangd` 前提です。
- Pythonのローカル偽 `atcoder/` パッケージは置きません。`ac-library-python` を使います。
