#include <stdio.h>

#include "board/board.h"
#include "movegen/movegen.h"

int main() {

    Position pos;

    initBoard(&pos);

    printBoard(&pos);

    Move moves[MAX_MOVES];

    int count = generateMoves(&pos, moves);

    printf("Generated Moves: %d\n\n", count);

    for(int i=0;i<count;i++) {

        printf(
            "(%d,%d) -> (%d,%d)\n",

            moves[i].fromRow,
            moves[i].fromCol,

            moves[i].toRow,
            moves[i].toCol
        );
    }

    return 0;
}