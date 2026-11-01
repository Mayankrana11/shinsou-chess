CC = gcc
CFLAGS = -Wall -Wextra -Wpedantic -std=c11 -I.
TARGET = shinsou.exe

# Core source files
CORE_SRC = board/board.c \
           movegen/movegen.c \
           movegen/attacks.c \
           fen/fen.c \
           engine/eval.c \
           engine/ordering.c \
           engine/search.c \
           engine/tt.c

# Main engine binary
SRC = $(CORE_SRC) main.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

# Test binary for evaluation
test_eval: $(CORE_SRC) tests/test_eval.c
	$(CC) $(CFLAGS) -o tests/test_eval.exe $(CORE_SRC) tests/test_eval.c

# Run evaluation tests
run_eval: test_eval
	./tests/test_eval.exe

# Test binary for search
test_search: $(CORE_SRC) tests/test_search.c
	$(CC) $(CFLAGS) -o tests/test_search.exe $(CORE_SRC) tests/test_search.c

# Run search tests
run_search: test_search
	./tests/test_search.exe

# Test binary for move ordering
test_ordering: $(CORE_SRC) tests/test_ordering.c
	$(CC) $(CFLAGS) -o tests/test_ordering.exe $(CORE_SRC) tests/test_ordering.c

# Run ordering tests
run_ordering: test_ordering
	./tests/test_ordering.exe

clean:
	del /f /q $(TARGET) tests\test_eval.exe tests\test_search.exe tests\test_ordering.exe 2>nul || true

# WASM Build
EMCC = emcc
WASM_CFLAGS = -O3 -std=c11 -I. -Iboard -Imovegen -Ifen -Iengine -Iutils
WASM_TARGET = ../../frontend/public/engine/engine.js
WASM_FLAGS = -s EXPORTED_FUNCTIONS="['_get_best_move']" \
             -s EXPORTED_RUNTIME_METHODS="['ccall', 'cwrap']" \
             -s MODULARIZE=1 -s EXPORT_NAME="createEngineModule" \
             -s STACK_SIZE=5242880 -s ALLOW_MEMORY_GROWTH=1

wasm: $(CORE_SRC) wasm_bridge.c
	$(EMCC) $(WASM_CFLAGS) $(CORE_SRC) wasm_bridge.c $(WASM_FLAGS) -o $(WASM_TARGET)

wasm_clean:
	del /f /q ../../frontend/public/engine/engine.js ../../frontend/public/engine/engine.wasm 2>nul || true

.PHONY: all clean test_eval run_eval test_search run_search test_ordering run_ordering wasm wasm_clean
