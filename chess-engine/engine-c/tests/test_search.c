#include <stdio.h>
#include <assert.h>
#include "../board/board.h"
#include "../engine/search.h"
#include "../fen/fen.h"
#include "../utils/constants.h"

void test_capture_search() {
    printf("Testing Capture Search...\n");
    Position pos;
    // White to move, only legal move is to capture the black queen
    // 8/8/8/8/8/8/8/4K3 w - - 0 1
    // Put a black queen on d4 and white king on e1
    clearBoard(&pos);
    pos.sideToMove = WHITE;
    pos.board[7][4] = WKING;
    pos.board[3][3] = BQUEEN;
    pos.whiteKingRow = 7; pos.whiteKingCol = 4;
    pos.blackKingRow = 0; pos.blackKingCol = 0; // dummy

    // Need a white piece to capture it. Put a white rook on d1
    pos.board[7][3] = WROOK;

    Move bestMove;
    findBestMove(&pos, 3, &bestMove);

    printf("Best move: (%d,%d) -> (%d,%d)\n", bestMove.fromRow, bestMove.fromCol, bestMove.toRow, bestMove.toCol);
    assert(bestMove.toRow == 3 && bestMove.toCol == 3);
    printf("Capture Search PASSED\n");
}

void test_mate_in_one() {
    printf("\nTesting Mate in One...\n");
    Position pos;
    // Fool's mate variant or simple mate
    // 4k3/8/8/8/8/8/8/4K3 w - - 0 1
    // White Queen on a7, Black King on e8.
    clearBoard(&pos);
    pos.sideToMove = WHITE;
    pos.board[0][4] = BKING;
    pos.board[1][0] = WQUEEN;
    pos.whiteKingRow = 7; pos.whiteKingCol = 4;
    pos.blackKingRow = 0; pos.blackKingCol = 4;

    Move bestMove;
    findBestMove(&pos, 3, &bestMove);

    printf("Best move: (%d,%d) -> (%d,%d)\n", bestMove.fromRow, bestMove.fromCol, bestMove.toRow, bestMove.toCol);
    // Should move Queen to a8 (0,0) for mate
    assert(bestMove.toRow == 0 && bestMove.toCol == 0);
    printf("Mate in One PASSED\n");
}

int main() {
    printf("Running Search Tests...\n");
    test_capture_search();
    test_mate_in_one();
    printf("\nAll Search Tests Passed!\n");
    return 0;
}
