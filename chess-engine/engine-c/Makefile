CC = gcc
CFLAGS = -Wall -Wextra -Wpedantic -std=c11 -I.
TARGET = shinsou.exe

# Core source files
CORE_SRC = board/board.c \
           movegen/movegen.c \
           fen/fen.c \
           engine/eval.c \
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

clean:
	del $(TARGET)
	del tests/test_eval.exe

.PHONY: all clean test_eval run_eval
