#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "../board/board.h"
#include "../engine/ordering.h"
#include "../engine/search.h"
#include "../movegen/movegen.h"
#include "../fen/fen.h"
#include "../utils/constants.h"

/* ============================================================
 *  Test 1: MVV-LVA capture ordering
 *  Verify PxQ scores higher than QxP.
 * ============================================================ */
void test_mvvlva_ordering() {
    printf("Test 1: MVV-LVA Ordering...\n");
    Position pos;
    clearBoard(&pos);
    pos.sideToMove = WHITE;
    pos.whiteKingRow = 7; pos.whiteKingCol = 4;
    pos.blackKingRow = 0; pos.blackKingCol = 4;
    pos.board[7][4] = WKING;
    pos.board[0][4] = BKING;

    /* White pawn on e5 (row 3, col 4) can capture black queen on d4 (row 2, col 3) */
    pos.board[3][4] = WPAWN;
    pos.board[2][3] = BQUEEN;

    /* White queen on a1 (row 7, col 0) can capture black pawn on b2 (row 6, col 1) */
    pos.board[7][0] = WQUEEN;
    pos.board[6][1] = BPAWN;

    Move moves[MAX_MOVES];
    int count = generateLegalMoves(&pos, moves);

    initOrdering();
    scoreMoves(moves, count, 0, WHITE);

    /* Find PxQ and QxP captures */
    int pxqScore = -1, qxpScore = -1;
    for (int i = 0; i < count; i++) {
        if (moves[i].captured != EMPTY) {
            printf("  Capture: (%d,%d)->(%d,%d) piece=%d cap=%d score=%d\n",
                   moves[i].fromRow, moves[i].fromCol,
                   moves[i].toRow, moves[i].toCol,
                   moves[i].piece, moves[i].captured, moves[i].score);
        }
        /* PxQ: pawn (3,4) captures queen at (2,3) */
        if (moves[i].fromRow == 3 && moves[i].fromCol == 4 &&
            moves[i].toRow == 2 && moves[i].toCol == 3) {
            pxqScore = moves[i].score;
        }
        /* QxP: queen (7,0) captures pawn at (6,1) */
        if (moves[i].fromRow == 7 && moves[i].fromCol == 0 &&
            moves[i].toRow == 6 && moves[i].toCol == 1) {
            qxpScore = moves[i].score;
        }
    }

    printf("  PxQ score: %d\n", pxqScore);
    printf("  QxP score: %d\n", qxpScore);

    assert(pxqScore > 0 && "PxQ capture should have positive score");
    assert(qxpScore > 0 && "QxP capture should have positive score");
    assert(pxqScore > qxpScore && "PxQ (MVV-LVA) must be ordered before QxP");
    printf("  PASSED\n\n");
}

/* ============================================================
 *  Test 2: Captures always ordered before quiet moves
 * ============================================================ */
void test_captures_before_quiet() {
    printf("Test 2: Captures Before Quiet Moves...\n");
    Position pos;
    clearBoard(&pos);
    pos.sideToMove = WHITE;
    pos.whiteKingRow = 7; pos.whiteKingCol = 4;
    pos.blackKingRow = 0; pos.blackKingCol = 4;
    pos.board[7][4] = WKING;
    pos.board[0][4] = BKING;
    pos.board[4][3] = WPAWN;
    pos.board[3][4] = BPAWN;  /* Pawn can capture diagonally */

    Move moves[MAX_MOVES];
    int count = generateLegalMoves(&pos, moves);

    initOrdering();
    scoreMoves(moves, count, 0, WHITE);

    int minCaptureScore = 2000000;
    int maxQuietScore = -1;
    for (int i = 0; i < count; i++) {
        if (moves[i].captured != EMPTY) {
            if (moves[i].score < minCaptureScore) minCaptureScore = moves[i].score;
        } else if (moves[i].type != MOVE_PROMOTION) {
            if (moves[i].score > maxQuietScore) maxQuietScore = moves[i].score;
        }
    }

    printf("  Min capture score: %d\n", minCaptureScore);
    printf("  Max quiet score: %d\n", maxQuietScore);
    assert(minCaptureScore > maxQuietScore && "All captures must score above quiet moves");
    printf("  PASSED\n\n");
}

/* ============================================================
 *  Test 3: Killer move ordering
 *  A registered killer should score above other quiet moves.
 * ============================================================ */
void test_killer_moves() {
    printf("Test 3: Killer Move Ordering...\n");
    Position pos;
    initBoard(&pos);

    initOrdering();

    /* Register e2-e4 as killer at ply 0 */
    Move killer;
    killer.fromRow = 6; killer.fromCol = 4;
    killer.toRow = 4;   killer.toCol = 4;
    killer.captured = EMPTY;
    killer.type = MOVE_NORMAL;
    killer.promotion = 0;
    updateKillers(&killer, 0);

    Move moves[MAX_MOVES];
    int count = generateLegalMoves(&pos, moves);
    scoreMoves(moves, count, 0, WHITE);

    /* Find the killer and another quiet move */
    int killerScore = -1, quietScore = -1;
    for (int i = 0; i < count; i++) {
        if (moves[i].fromRow == 6 && moves[i].fromCol == 4 &&
            moves[i].toRow == 4 && moves[i].toCol == 4) {
            killerScore = moves[i].score;
            printf("  Killer (e2e4) score: %d\n", killerScore);
        }
        /* a2-a3 as reference quiet move */
        if (moves[i].fromRow == 6 && moves[i].fromCol == 0 &&
            moves[i].toRow == 5 && moves[i].toCol == 0) {
            quietScore = moves[i].score;
            printf("  Quiet (a2a3) score: %d\n", quietScore);
        }
    }

    assert(killerScore == KILLER_SCORE_1 && "Killer #1 should have KILLER_SCORE_1");
    assert(killerScore > quietScore && "Killer must score above other quiet moves");
    printf("  PASSED\n\n");
}

/* ============================================================
 *  Test 4: Two-killer FIFO replacement
 * ============================================================ */
void test_killer_replacement() {
    printf("Test 4: Killer FIFO Replacement...\n");
    Position pos;
    initBoard(&pos);

    initOrdering();

    /* Register first killer: e2-e4 */
    Move k1;
    k1.fromRow = 6; k1.fromCol = 4;
    k1.toRow = 4;   k1.toCol = 4;
    updateKillers(&k1, 0);

    /* Register second killer: d2-d4 (pushes e2-e4 to slot 2) */
    Move k2;
    k2.fromRow = 6; k2.fromCol = 3;
    k2.toRow = 4;   k2.toCol = 3;
    updateKillers(&k2, 0);

    Move moves[MAX_MOVES];
    int count = generateLegalMoves(&pos, moves);
    scoreMoves(moves, count, 0, WHITE);

    int k1Score = -1, k2Score = -1;
    for (int i = 0; i < count; i++) {
        if (moves[i].fromRow == 6 && moves[i].fromCol == 4 &&
            moves[i].toRow == 4 && moves[i].toCol == 4) {
            k1Score = moves[i].score;
        }
        if (moves[i].fromRow == 6 && moves[i].fromCol == 3 &&
            moves[i].toRow == 4 && moves[i].toCol == 3) {
            k2Score = moves[i].score;
        }
    }

    printf("  First killer (now slot 2) score: %d (expected %d)\n", k1Score, KILLER_SCORE_2);
    printf("  Second killer (slot 1) score: %d (expected %d)\n", k2Score, KILLER_SCORE_1);
    assert(k2Score == KILLER_SCORE_1 && "Newest killer should be in slot 1");
    assert(k1Score == KILLER_SCORE_2 && "Previous killer should be in slot 2");
    assert(k2Score > k1Score && "Killer #1 must score above killer #2");
    printf("  PASSED\n\n");
}

/* ============================================================
 *  Test 5: History heuristic ordering
 * ============================================================ */
void test_history_heuristic() {
    printf("Test 5: History Heuristic...\n");
    Position pos;
    initBoard(&pos);

    initOrdering();

    /* Simulate: e2-e4 caused beta cutoffs 10 times at depth 5 */
    Move histMove;
    histMove.fromRow = 6; histMove.fromCol = 4;
    histMove.toRow = 4;   histMove.toCol = 4;

    for (int i = 0; i < 10; i++) {
        updateHistory(&histMove, WHITE, 5);
    }

    Move moves[MAX_MOVES];
    int count = generateLegalMoves(&pos, moves);
    scoreMoves(moves, count, 0, WHITE);

    int histScore = -1, otherScore = -1;
    for (int i = 0; i < count; i++) {
        if (moves[i].fromRow == 6 && moves[i].fromCol == 4 &&
            moves[i].toRow == 4 && moves[i].toCol == 4) {
            histScore = moves[i].score;
            printf("  e2e4 history score: %d (expected: %d)\n", histScore, 10 * 25);
        }
        if (moves[i].fromRow == 6 && moves[i].fromCol == 0 &&
            moves[i].toRow == 5 && moves[i].toCol == 0) {
            otherScore = moves[i].score;
            printf("  a2a3 score: %d (expected: 0)\n", otherScore);
        }
    }

    assert(histScore == 10 * 25 && "History should be 10 * depth^2 = 250");
    assert(histScore > otherScore && "History-boosted move must score above unboosted");
    printf("  PASSED\n\n");
}

/* ============================================================
 *  Test 6: pickBestMove incremental selection sort
 * ============================================================ */
void test_pick_best() {
    printf("Test 6: pickBestMove Selection Sort...\n");
    Move moves[5];
    memset(moves, 0, sizeof(moves));
    moves[0].score = 100;  moves[0].fromRow = 0;
    moves[1].score = 500;  moves[1].fromRow = 1;
    moves[2].score = 300;  moves[2].fromRow = 2;
    moves[3].score = 900;  moves[3].fromRow = 3;
    moves[4].score = 200;  moves[4].fromRow = 4;

    pickBestMove(moves, 5, 0);
    assert(moves[0].fromRow == 3 && moves[0].score == 900);

    pickBestMove(moves, 5, 1);
    assert(moves[1].score == 500);

    pickBestMove(moves, 5, 2);
    assert(moves[2].score == 300);

    pickBestMove(moves, 5, 3);
    assert(moves[3].score == 200);

    printf("  Sorted order: %d %d %d %d %d\n",
           moves[0].score, moves[1].score, moves[2].score,
           moves[3].score, moves[4].score);
    printf("  PASSED\n\n");
}

/* ============================================================
 *  Test 7: Perft correctness — ordering must not change results
 * ============================================================ */
void test_perft_correctness() {
    printf("Test 7: Perft Correctness (ordering must not break move gen)...\n");
    Position pos;
    initBoard(&pos);

    uint64_t p1 = perft(&pos, 1);
    uint64_t p2 = perft(&pos, 2);
    uint64_t p3 = perft(&pos, 3);
    uint64_t p4 = perft(&pos, 4);

    printf("  Depth 1: %llu (expected 20)\n", (unsigned long long)p1);
    printf("  Depth 2: %llu (expected 400)\n", (unsigned long long)p2);
    printf("  Depth 3: %llu (expected 8902)\n", (unsigned long long)p3);
    printf("  Depth 4: %llu (expected 197281)\n", (unsigned long long)p4);

    assert(p1 == 20);
    assert(p2 == 400);
    assert(p3 == 8902);
    assert(p4 == 197281);
    printf("  PASSED\n\n");
}

/* ============================================================
 *  Test 8: Search still finds correct best moves
 * ============================================================ */
void test_search_correctness() {
    printf("Test 8: Search Correctness With Ordering...\n");

    /* Test A: Engine should capture hanging queen */
    Position pos;
    clearBoard(&pos);
    pos.sideToMove = WHITE;
    pos.board[7][4] = WKING;
    pos.board[0][4] = BKING;
    pos.board[3][3] = BQUEEN;   /* Hanging queen */
    pos.board[7][3] = WROOK;    /* Rook can capture it */
    pos.whiteKingRow = 7; pos.whiteKingCol = 4;
    pos.blackKingRow = 0; pos.blackKingCol = 4;

    Move bestMove;
    findBestMove(&pos, 3, &bestMove);
    printf("  Capture queen: (%d,%d)->(%d,%d)\n",
           bestMove.fromRow, bestMove.fromCol, bestMove.toRow, bestMove.toCol);
    assert(bestMove.toRow == 3 && bestMove.toCol == 3 && "Should capture the queen");

    /* Test B: Mate in one — queen delivers checkmate */
    clearBoard(&pos);
    pos.sideToMove = WHITE;
    pos.board[7][4] = WKING;
    pos.board[0][7] = BKING;    /* Black king in corner */
    pos.board[1][6] = BPAWN;    /* Blocks escape */
    pos.board[1][7] = BPAWN;
    pos.board[6][0] = WQUEEN;   /* Queen can deliver back rank mate */
    pos.whiteKingRow = 7; pos.whiteKingCol = 4;
    pos.blackKingRow = 0; pos.blackKingCol = 7;

    findBestMove(&pos, 3, &bestMove);
    printf("  Mate: (%d,%d)->(%d,%d)\n",
           bestMove.fromRow, bestMove.fromCol, bestMove.toRow, bestMove.toCol);
    /* Queen should go to rank 8 (row 0) for back rank mate */
    assert(bestMove.toRow == 0 && "Should deliver back rank mate");

    printf("  PASSED\n\n");
}

/* ============================================================
 *  Test 9: Quiescence with MVV-LVA ordering
 * ============================================================ */
void test_quiescence_ordering() {
    printf("Test 9: Quiescence MVV-LVA Ordering...\n");
    Position pos;
    clearBoard(&pos);
    pos.sideToMove = WHITE;
    pos.board[7][4] = WKING;
    pos.board[0][4] = BKING;
    pos.board[7][3] = WQUEEN;
    pos.board[6][3] = BQUEEN;
    pos.board[7][2] = BROOK;
    pos.whiteKingRow = 7; pos.whiteKingCol = 4;
    pos.blackKingRow = 0; pos.blackKingCol = 4;

    Move bestMove;
    int score = findBestMove(&pos, 1, &bestMove);
    printf("  Depth 1 score: %d\n", score);
    /* Quiescence should see the recapture and not overvalue */
    assert(score < 500 && "Quiescence should see recapture risk");
    printf("  PASSED\n\n");
}

int main() {
    printf("=== Move Ordering Tests (Phase 14) ===\n\n");
    test_mvvlva_ordering();
    test_captures_before_quiet();
    test_killer_moves();
    test_killer_replacement();
    test_history_heuristic();
    test_pick_best();
    test_perft_correctness();
    test_search_correctness();
    test_quiescence_ordering();
    printf("=== All Move Ordering Tests Passed! ===\n");
    return 0;
}
