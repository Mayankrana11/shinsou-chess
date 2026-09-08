#include "search.h"
#include "eval.h"
#include "../movegen/movegen.h"
#include "../board/board.h"
#include <stdio.h>
#include <stdint.h>

#define MATE_SCORE 100000

// Forward declaration of quiescence search
static int quiesce(Position* pos, int alpha, int beta);

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

int search(Position* pos, int depth, int alpha, int beta) {
    // 1. Terminal Conditions
    if (depth == 0) {
        // Use quiescence search to avoid the horizon effect
        return quiesce(pos, alpha, beta);
    }

    Move moves[MAX_MOVES];
    int moveCount = generateLegalMoves(pos, moves);

    if (moveCount == 0) {
        if (isInCheck(pos, pos->sideToMove)) {
            return -MATE_SCORE + depth;
        } else {
            return 0;
        }
    }

    int bestScore = -2000000;

    for (int i = 0; i < moveCount; i++) {
        makeMove(pos, &moves[i]);
        int score = -search(pos, depth - 1, -beta, -alpha);
        undoMove(pos, &moves[i]);

        if (score > bestScore) {
            bestScore = score;
        }

        if (bestScore >= beta) {
            return beta;
        }
        if (bestScore > alpha) {
            alpha = bestScore;
        }
    }

    return bestScore;
}

int quiesce(Position* pos, int alpha, int beta) {
    // Stand-pat evaluation: current player can choose to stop here
    int standPat = evaluate(pos) * pos->sideToMove;
    if (standPat >= beta) return beta;
    if (standPat > alpha) alpha = standPat;

    Move moves[MAX_MOVES];
    int moveCount = generateTacticalMoves(pos, moves);

    if (moveCount == 0) return standPat;

    int bestScore = -2000000;

    for (int i = 0; i < moveCount; i++) {
        makeMove(pos, &moves[i]);
        // Quiescence search recursively looks at other captures
        int score = -quiesce(pos, -beta, -alpha);
        undoMove(pos, &moves[i]);

        if (score > bestScore) {
            bestScore = score;
        }

        if (bestScore >= beta) {
            return beta;
        }
        if (bestScore > alpha) {
            alpha = bestScore;
        }
    }

    return (bestScore > standPat) ? bestScore : standPat;
}

int findBestMove(Position* pos, int depth, Move* bestMove) {
    int alpha = -2000000;
    int beta = 2000000;
    int bestScore = -2000000;

    Move moves[MAX_MOVES];
    int moveCount = generateLegalMoves(pos, moves);

    if (moveCount == 0) return 0;

    for (int i = 0; i < moveCount; i++) {
        makeMove(pos, &moves[i]);
        int score = -search(pos, depth - 1, -beta, -alpha);
        undoMove(pos, &moves[i]);

        if (score > bestScore) {
            bestScore = score;
            if (bestMove) {
                *bestMove = moves[i];
            }
        }

        if (bestScore > alpha) {
            alpha = bestScore;
        }
    }

    return bestScore;
}
