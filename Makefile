CXX := g++-15
CPP_SRC ?= main.cpp
PY_SRC ?= main.py

ACL_DIR ?= ac-library
BUILD_DIR ?= .build
PCH_DIR ?= $(BUILD_DIR)/pch

CPP_FAST_BIN ?= $(BUILD_DIR)/main-fast
CPP_DEBUG_BIN ?= $(BUILD_DIR)/main-debug
CPP_OPT_BIN ?= $(BUILD_DIR)/main-opt

CPP_COMMON_FLAGS := -std=gnu++23 -Wall -Wextra -Wshadow -Winvalid-pch -I$(ACL_DIR) -DINPUT_FILE_PATH=\"input.txt\"
CPP_FAST_FLAGS := $(CPP_COMMON_FLAGS) -O0 -DLOCAL
CPP_DEBUG_FLAGS := $(CPP_COMMON_FLAGS) -g -O0 -DLOCAL -D_GLIBCXX_DEBUG -fsanitize=address,undefined -fno-sanitize-recover=undefined -fno-omit-frame-pointer -pipe
CPP_ATCODER_FLAGS := $(CPP_COMMON_FLAGS) -O2 -DATCODER -DONLINE_JUDGE

PCH_SRC := .config/pch.h
PCH_FAST_HEADER := $(PCH_DIR)/pch_fast.h
PCH_DEBUG_HEADER := $(PCH_DIR)/pch_debug.h
PCH_RELEASE_HEADER := $(PCH_DIR)/pch_release.h
PCH_FAST_OUT := $(PCH_FAST_HEADER).gch
PCH_DEBUG_OUT := $(PCH_DEBUG_HEADER).gch
PCH_RELEASE_OUT := $(PCH_RELEASE_HEADER).gch

.PHONY: run run-cpp run-opt-cpp debug-cpp run-py fmt-py check-py clean versions

# デフォルトのショートカット (make run で実行可能に)
run: run-cpp

# 通常の実行（高速ビルド: -O0 + PCH、重いデバッグ検査なし）
run-cpp: build-fast-cpp
	@./$(CPP_FAST_BIN)

# デバッグ実行（デバッグ情報 + _GLIBCXX_DEBUG）
debug-cpp: build-debug-cpp
	@./$(CPP_DEBUG_BIN)

# 最適化ビルドでの実行（-O2 + PCH）
run-opt-cpp: build-opt-cpp
	@./$(CPP_OPT_BIN)

build-fast-cpp: $(CPP_FAST_BIN)

build-debug-cpp: $(CPP_DEBUG_BIN)

build-opt-cpp: $(CPP_OPT_BIN)

$(CPP_FAST_BIN): $(CPP_SRC) $(PCH_FAST_OUT) | $(BUILD_DIR)
	@$(CXX) $(CPP_FAST_FLAGS) -include $(PCH_FAST_HEADER) -o $@ $(CPP_SRC)

$(CPP_DEBUG_BIN): $(CPP_SRC) $(PCH_DEBUG_OUT) | $(BUILD_DIR)
	@$(CXX) $(CPP_DEBUG_FLAGS) -include $(PCH_DEBUG_HEADER) -o $@ $(CPP_SRC)

$(CPP_OPT_BIN): $(CPP_SRC) $(PCH_RELEASE_OUT) | $(BUILD_DIR)
	@$(CXX) $(CPP_ATCODER_FLAGS) -include $(PCH_RELEASE_HEADER) -o $@ $(CPP_SRC)

$(BUILD_DIR):
	@mkdir -p $@

$(PCH_DIR): $(BUILD_DIR)
	@mkdir -p $@

$(PCH_FAST_HEADER): $(PCH_SRC) | $(PCH_DIR)
	@ln -sf ../../$(PCH_SRC) $@

$(PCH_DEBUG_HEADER): $(PCH_SRC) | $(PCH_DIR)
	@ln -sf ../../$(PCH_SRC) $@

$(PCH_RELEASE_HEADER): $(PCH_SRC) | $(PCH_DIR)
	@ln -sf ../../$(PCH_SRC) $@

$(PCH_FAST_OUT): $(PCH_FAST_HEADER)
	@echo "Precompiling headers for Fast..."
	@$(CXX) $(CPP_FAST_FLAGS) -x c++-header -c $< -o $@

$(PCH_DEBUG_OUT): $(PCH_DEBUG_HEADER)
	@echo "Precompiling headers for Debug..."
	@$(CXX) $(CPP_DEBUG_FLAGS) -x c++-header -c $< -o $@

$(PCH_RELEASE_OUT): $(PCH_RELEASE_HEADER)
	@echo "Precompiling headers for Release..."
	@$(CXX) $(CPP_ATCODER_FLAGS) -x c++-header -c $< -o $@

run-py:
	uv run python $(PY_SRC)

fmt-py:
	uv run ruff format $(PY_SRC)

check-py:
	uv run ruff check $(PY_SRC)

versions:
	@echo "CXX=$$(command -v $(CXX) || true)"
	@$(CXX) --version | head -n 1 || true
	@uv --version || true
	@uv run python --version || true

clean:
	rm -rf $(BUILD_DIR)
