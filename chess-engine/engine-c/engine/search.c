#include "search.h"
#include "../movegen/movegen.h"
#include "../board/board.h"
#include <stdio.h>
#include <stdint.h>

uint64_t perft(Position* pos, int depth) {
    if (depth == 0) {
        return 1;
    }

    Move moves[MAX_MOVES];
    int moveCount = generateLegalMoves(pos, moves);
    uint64_t nodes = 0;

    for (int i = 0; i < moveCount; i++) {
        makeMove(pos, &moves[i]);
        nodes += perft(pos, depth - 1);
        undoMove(pos, &moves[i]);
    }

    return nodes;
}

void perftDivide(Position* pos, int depth) {
    if (depth == 0) {
        return;
    }

    Move moves[MAX_MOVES];
    int moveCount = generateLegalMoves(pos, moves);
    uint64_t totalNodes = 0;

    for (int i = 0; i < moveCount; i++) {
        makeMove(pos, &moves[i]);
        uint64_t nodes = perft(pos, depth - 1);
        undoMove(pos, &moves[i]);

        char fromFile = 'a' + moves[i].fromCol;
        char fromRank = '8' - moves[i].fromRow;
        char toFile = 'a' + moves[i].toCol;
        char toRank = '8' - moves[i].toRow;

        printf("%c%c%c%c: %llu\n", fromFile, fromRank, toFile, toRank, (unsigned long long)nodes);
        totalNodes += nodes;
    }

    printf("\nTotal nodes: %llu\n", (unsigned long long)totalNodes);
}