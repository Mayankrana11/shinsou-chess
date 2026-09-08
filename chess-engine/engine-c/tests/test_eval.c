#include <stdio.h>
#include <assert.h>
#include "../board/board.h"
#include "../engine/eval.h"
#include "../fen/fen.h"
#include "../utils/constants.h"

void test_starting_position() {
    Position pos;
    initBoard(&pos);
    int score = evaluate(&pos);
    printf("Starting position score: %d\n", score);
    // Starting position should be roughly 0
    assert(score > -100 && score < 100);
}

void test_material_advantage() {
    Position pos;
    // FEN for White with an extra queen
    // "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1" is start
    // Let's use a simple FEN: a queen for white, nothing else.
    parseFEN(&pos, "4q3/8/8/8/8/8/8/4K3 w - - 0 1");
    int score = evaluate(&pos);
    printf("Black extra queen score: %d\n", score);
    assert(score < -800);

    parseFEN(&pos, "4k3/8/8/8/8/8/8/4Q3 w - - 0 1");
    score = evaluate(&pos);
    printf("White extra queen score: %d\n", score);
    assert(score > 800);
}

void test_pst_knights() {
    Position pos;
    // Knight in center
    parseFEN(&pos, "8/8/8/8/4N3/8/8/4K3 w - - 0 1");
    evaluate(&pos);

    // Knight on edge
    parseFEN(&pos, "8/8/8/8/8/8/8/K1N3K1 w - - 0 1"); // This is not a legal FEN but for eval it's fine
    // Wait, let's use valid FENs.
    parseFEN(&pos, "8/8/8/8/8/8/8/K1N3K1 w - - 0 1"); // Invalid

    // Let's just manually place pieces in a Position for PST test.
    clearBoard(&pos);
    pos.board[7][4] = WKING;
    pos.board[3][3] = WKNIGHT;
    int s1 = evaluate(&pos);

    clearBoard(&pos);
    pos.board[7][4] = WKING;
    pos.board[7][0] = WKNIGHT;
    int s2 = evaluate(&pos);

    printf("Knight center: %d, Knight edge: %d\n", s1, s2);
    assert(s1 > s2);
}

int main() {
    printf("Running Evaluation Tests...\n");
    test_starting_position();
    test_material_advantage();
    test_pst_knights();
    printf("All Evaluation Tests Passed!\n");
    return 0;
}
