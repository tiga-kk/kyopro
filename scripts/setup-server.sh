#!/usr/bin/env bash
set -euo pipefail

sudo apt update
sudo apt install -y \
  build-essential make git curl ca-certificates \
  gcc-15 g++-15 gdb clangd \
  pypy3

# uvは既に入っている前提。なければ公式手順で入れてください。
uv --version

# AtCoderのCPythonに寄せる。既に入っていれば何もしない。
uv python install 3.13
uv sync

make versions
