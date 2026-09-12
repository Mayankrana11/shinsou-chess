#include "search.h"
#include "eval.h"
#include "ordering.h"
#include "tt.h"
#include "../movegen/movegen.h"
#include "../board/board.h"
#include "../utils/constants.h"
#include <stdio.h>
#include <stdint.h>

#define MATE_SCORE 100000

/* Forward declaration of quiescence search */
static int quiesce(Position* pos, int alpha, int beta);

/* ========================================================
 *  Perft — move generation correctness testing
 * ======================================================== */

uint64_t perft(Position* pos, int depth) {
    if (depth == 0) return 1;
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
    if (depth == 0) return;
    Move moves[MAX_MOVES];
    int moveCount = generateLegalMoves(pos, moves);
    uint64_t totalNodes = 0;
    for (int i = 0; i < moveCount; i++) {
        makeMove(pos, &moves[i]);
        uint64_t nodes = perft(pos, depth - 1);
        undoMove(pos, &moves[i]);
        printf("%c%c%c%c: %llu\n",
               'a' + moves[i].fromCol, '8' - moves[i].fromRow,
               'a' + moves[i].toCol, '8' - moves[i].toRow,
               (unsigned long long)nodes);
        totalNodes += nodes;
    }
    printf("\nTotal nodes: %llu\n", (unsigned long long)totalNodes);
}

/* ========================================================
 *  Negamax search with Alpha-Beta pruning and TT
 * ======================================================== */

int search(Position* pos, int depth, int alpha, int beta, int ply) {
    /* 1. TT Probe */
    TTEntry *entry = ttProbe(pos->hash);
    if (entry && entry->depth >= depth) {
        if (entry->flag == EXACT) return entry->score;
        if (entry->flag == LOWERBOUND && entry->score >= beta) return entry->score;
        if (entry->flag == UPPERBOUND && entry->score <= alpha) return entry->score;
    }

    if (depth == 0) {
        return quiesce(pos, alpha, beta);
    }

    Move moves[MAX_MOVES];
    int moveCount = generateLegalMoves(pos, moves);

    if (moveCount == 0) {
        if (isInCheck(pos, pos->sideToMove)) {
            return -MATE_SCORE + ply;
        } else {
            return 0;
        }
    }

    /* Root move ordering from TT if available */
    if (entry) {
        // Move TT move to front (manual swap or just let scoreMoves handle it if we’d integrated it there)
        // For now, we'll let scoreMoves and pickBestMove handle the heuristics.
    }

    scoreMoves(moves, moveCount, ply, pos->sideToMove);

    int bestScore = -2000000;
    Move bestMove = moves[0];
    int alphaOrig = alpha;

    for (int i = 0; i < moveCount; i++) {
        pickBestMove(moves, moveCount, i);

        makeMove(pos, &moves[i]);
        int score = -search(pos, depth - 1, -beta, -alpha, ply + 1);
        undoMove(pos, &moves[i]);

        if (score > bestScore) {
            bestScore = score;
            bestMove = moves[i];
        }

        if (bestScore >= beta) {
            if (moves[i].captured == EMPTY && moves[i].type != MOVE_PROMOTION) {
                updateKillers(&moves[i], ply);
                updateHistory(&moves[i], pos->sideToMove, depth);
            }
            break;
        }

        if (bestScore > alpha) {
            alpha = bestScore;
        }
    }

    /* 2. TT Store */
    TTFlag flag = EXACT;
    if (bestScore <= alphaOrig) flag = UPPERBOUND;
    else if (bestScore >= beta) flag = LOWERBOUND;

    ttStore(pos->hash, depth, bestScore, flag, bestMove);

    return bestScore;
}

/* ========================================================
 *  Quiescence search
 * ======================================================== */

static int quiesce(Position* pos, int alpha, int beta) {
    int standPat = evaluate(pos) * pos->sideToMove;
    if (standPat >= beta) return beta;
    if (standPat > alpha) alpha = standPat;

    Move moves[MAX_MOVES];
    int moveCount = generateTacticalMoves(pos, moves);
    if (moveCount == 0) return standPat;

    scoreCaptures(moves, moveCount);

    int bestScore = -2000000;
    for (int i = 0; i < moveCount; i++) {
        pickBestMove(moves, moveCount, i);
        makeMove(pos, &moves[i]);
        int score = -quiesce(pos, -beta, -alpha);
        undoMove(pos, &moves[i]);

        if (score > bestScore) {
            bestScore = score;
        }
        if (bestScore >= beta) return beta;
        if (bestScore > alpha) alpha = bestScore;
    }

    return (bestScore > standPat) ? bestScore : standPat;
}

/* ========================================================
 *  Iterative Deepening Entry Point
 * ======================================================== */

int findBestMove(Position* pos, int depth, Move* bestMove) {
    int alpha = -2000000;
    int beta = 2000000;
    int bestScore = -2000000;

    /* Iterative Deepening */
    for (int d = 1; d <= depth; d++) {
        Move moves[MAX_MOVES];
        int moveCount = generateLegalMoves(pos, moves);
        if (moveCount == 0) return 0;

        scoreMoves(moves, moveCount, 0, pos->sideToMove);

        int currentBestScore = -2000000;
        Move currentBestMove = moves[0];

        for (int i = 0; i < moveCount; i++) {
            pickBestMove(moves, moveCount, i);
            makeMove(pos, &moves[i]);
            int score = -search(pos, d - 1, -beta, -alpha, 1);
            undoMove(pos, &moves[i]);

            if (score > currentBestScore) {
                currentBestScore = score;
                currentBestMove = moves[i];
            }

            if (currentBestScore > alpha) {
                alpha = currentBestScore;
            }
        }

        bestScore = currentBestScore;
        if (bestMove) *bestMove = currentBestMove;
    }

    return bestScore;
}
