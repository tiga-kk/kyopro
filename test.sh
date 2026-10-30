#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

# Change these for the problem you are testing.
GENERATOR="generator.py"
CANDIDATE=".venv/bin/python main.py"
REFERENCE=".venv/bin/python fool.py"
SEED=42
CASES=500

# For C++, build both programs first, then use for example:
# CANDIDATE=".build/main-fast"
# REFERENCE=".build/fool-fast"

exec "$SCRIPT_DIR/.venv/bin/python" "$SCRIPT_DIR/tools/stress.py" \
  --generator "$GENERATOR" \
  --candidate "$CANDIDATE" \
  --reference "$REFERENCE" \
  --seed "$SEED" \
  --cases "$CASES"
