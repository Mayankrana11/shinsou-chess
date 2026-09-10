CC = gcc
CFLAGS = -Wall -Wextra -Wpedantic -std=c11 -I.
TARGET = shinsou.exe

# Core source files
CORE_SRC = board/board.c \
           movegen/movegen.c \
           fen/fen.c \
           engine/eval.c \
           engine/ordering.c \
           engine/search.c

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
	del $(TARGET)
	del tests/test_eval.exe
	del tests/test_search.exe
	del tests/test_ordering.exe

.PHONY: all clean test_eval run_eval test_search run_search test_ordering run_ordering
