#include <stdio.h>
#include <assert.h>
#include "../board/board.h"
#include "../engine/search.h"
#include "../fen/fen.h"
#include "../utils/constants.h"

void test_capture_search() {
    printf("Testing Capture Search...\n");
    Position pos;
    clearBoard(&pos);
    pos.sideToMove = WHITE;
    pos.board[7][4] = WKING;
    pos.board[0][4] = BKING;
    pos.board[3][3] = BQUEEN;
    pos.whiteKingRow = 7; pos.whiteKingCol = 4;
    pos.blackKingRow = 0; pos.blackKingCol = 4;
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
    clearBoard(&pos);
    /* Back rank mate: black king trapped, white rook delivers mate */
    pos.sideToMove = WHITE;
    pos.board[0][7] = BKING;
    pos.board[1][6] = BPAWN;    /* Blocks g7 */
    pos.board[1][7] = BPAWN;    /* Blocks h7 */
    pos.board[7][4] = WKING;
    pos.board[6][0] = WROOK;    /* Rook can go to a8 for mate */
    pos.whiteKingRow = 7; pos.whiteKingCol = 4;
    pos.blackKingRow = 0; pos.blackKingCol = 7;

    Move bestMove;
    findBestMove(&pos, 3, &bestMove);

    printf("Best move: (%d,%d) -> (%d,%d)\n", bestMove.fromRow, bestMove.fromCol, bestMove.toRow, bestMove.toCol);
    /* Rook to back rank (row 0) delivers mate */
    assert(bestMove.toRow == 0);
    printf("Mate in One PASSED\n");
}

void test_quiescence_horizon() {
    printf("\nTesting Quiescence Search (Horizon Effect)...\n");
    Position pos;
    clearBoard(&pos);
    pos.sideToMove = WHITE;
    pos.board[7][4] = WKING;
    pos.board[0][4] = BKING;

    // White Queen on d1, Black Queen on d2
    pos.board[7][3] = WQUEEN;
    pos.board[6][3] = BQUEEN;

    pos.whiteKingRow = 7; pos.whiteKingCol = 4;
    pos.blackKingRow = 0; pos.blackKingCol = 4;

    // Add another black piece to capture white queen
    pos.board[7][2] = BROOK; // Rook on c1

    Move bestMove;
    int score = findBestMove(&pos, 1, &bestMove);

    printf("Depth 1 score: %d\n", score);
    printf("Best move: (%d,%d) -> (%d,%d)\n", bestMove.fromRow, bestMove.fromCol, bestMove.toRow, bestMove.toCol);

    // The score should not be a massive +900 because quiescence sees the recapture.
    assert(score < 500);
    printf("Quiescence Horizon Test PASSED\n");
}

int main() {
    printf("Running Search Tests...\n");
    test_capture_search();
    test_mate_in_one();
    test_quiescence_horizon();
    printf("\nAll Search Tests Passed!\n");
    return 0;
}
