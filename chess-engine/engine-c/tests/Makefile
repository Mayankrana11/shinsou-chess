CC = gcc
CFLAGS = -Wall -Wextra -Wpedantic -std=c11 -I. -I../../
TARGET = test_fen.exe

SRC = ../board/board.c ../fen/fen.c ../movegen/movegen.c test_fen.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

clean:
	del $(TARGET)

test: all
	./$(TARGET)

.PHONY: all clean test