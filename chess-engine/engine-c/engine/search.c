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
#define NULL_MOVE_REDUCTION 100
#define FUTILITY_THRESHOLD 150

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
 *  Negamax search with PVS, NMP, LMR, and Futility Pruning
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

    /* 2. Null Move Pruning (NMP) */
    /* Apply NMP at depths > 3 and not in late game.
       If the side to move can pass and still have a score >= beta, prune. */
    if (depth >= 3 && !isInCheck(pos, pos->sideToMove)) {
        Position nullPos = *pos;
        nullPos.sideToMove = -pos->sideToMove;
        nullPos.halfmoveClock++;
        // A simple hash update for null move (side change)
        nullPos.hash ^= side_key;

        int nullScore = -search(&nullPos, depth - 1 - NULL_MOVE_REDUCTION, -beta, -alpha, ply + 1);
        if (nullScore >= beta) {
            return beta;
        }
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

    scoreMoves(moves, moveCount, ply, pos->sideToMove);

    int bestScore = -2000000;
    Move bestMove = moves[0];
    int alphaOrig = alpha;
    int firstMove = 1;

    for (int i = 0; i < moveCount; i++) {
        pickBestMove(moves, moveCount, i);

        /* 3. Futility Pruning */
        /* If the move is too bad to possibly reach alpha, prune it. */
        if (depth >= 3 && moves[i].captured == EMPTY && moves[i].type != MOVE_PROMOTION) {
            int staticEval = evaluate(pos); // Simplified static eval for futility
            if (staticEval - FUTILITY_THRESHOLD <= alpha) {
                continue;
            }
        }

        /* 4. Principal Variation Search (PVS) & Late Move Reductions (LMR) */
        int currentAlpha = alpha;
        int currentBeta = beta;
        int searchDepth = depth - 1;

        if (!firstMove) {
            /* PVS: Search with null window first */
            currentBeta = alpha + 1;

            /* LMR: Reduce depth for moves searched late */
            if (i > 3 && depth >= 3 && moves[i].captured == EMPTY && moves[i].type != MOVE_PROMOTION) {
                searchDepth -= 2; // Simple reduction
            }
        }

        makeMove(pos, &moves[i]);
        int score = -search(pos, searchDepth, -currentBeta, -currentAlpha, ply + 1);
        undoMove(pos, &moves[i]);

        /* If search with null window failed high, re-search with full window */
        if (!firstMove && score > alpha && currentBeta == alpha + 1) {
            makeMove(pos, &moves[i]);
            score = -search(pos, depth - 1, -beta, -alpha, ply + 1);
            undoMove(pos, &moves[i]);
        }

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

        firstMove = 0;
    }

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
 *  Search Entry Point with Aspiration Windows
 * ======================================================== */

int findBestMove(Position* pos, int depth, Move* bestMove) {
    int alpha = -2000000;
    int beta = 2000000;
    int bestScore = -2000000;

    /* Aspiration Windows: Start with a narrow window around previous best score */
    int aspirationWidth = 50;
    int currentAlpha = -2000000;
    int currentBeta = 2000000;
    int iteration = 0;

    while (iteration < 3) {
        if (iteration > 0) {
            currentAlpha = bestScore - aspirationWidth;
            currentBeta = bestScore + aspirationWidth;
        }

        Move moves[MAX_MOVES];
        int moveCount = generateLegalMoves(pos, moves);
        if (moveCount == 0) return 0;

        scoreMoves(moves, moveCount, 0, pos->sideToMove);

        int currentBestScore = -2000000;
        Move currentBestMove = moves[0];

        for (int i = 0; i < moveCount; i++) {
            pickBestMove(moves, moveCount, i);
            makeMove(pos, &moves[i]);
            int score = -search(pos, depth - 1, -currentBeta, -currentAlpha, 1);
            undoMove(pos, &moves[i]);

            if (score > currentBestScore) {
                currentBestScore = score;
                currentBestMove = moves[i];
            }

            if (currentBestScore > currentAlpha) {
                currentAlpha = currentBestScore;
            }
        }

        /* Check if the score is within the aspiration window */
        if (currentBestScore <= currentBeta && currentBestScore >= currentAlpha) {
            bestScore = currentBestScore;
            if (bestMove) *bestMove = currentBestMove;
            break;
        }

        /* If it failed high or low, expand the window and retry */
        bestScore = currentBestScore;
        if (bestMove) *bestMove = currentBestMove;
        aspirationWidth *= 2;
        iteration++;
    }

    return bestScore;
}
