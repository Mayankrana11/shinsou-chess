CC = gcc
CFLAGS = -Wall -Wextra -Wpedantic -std=c11 -I.
TARGET = shinsou.exe
SRC = board/board.c movegen/movegen.c main.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

clean:
	del $(TARGET)

test: $(TARGET)
	./$(TARGET)

.PHONY: all clean test