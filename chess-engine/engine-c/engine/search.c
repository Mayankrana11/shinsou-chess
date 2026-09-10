#include "search.h"
#include "eval.h"
#include "ordering.h"
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
 *  (No ordering needed; counts all leaf nodes exactly.)
 * ======================================================== */

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

/* ========================================================
 *  Negamax search with Alpha-Beta pruning and move ordering
 * ======================================================== */

int search(Position* pos, int depth, int alpha, int beta, int ply) {
    /* At depth 0, drop into quiescence to resolve tactics */
    if (depth == 0) {
        return quiesce(pos, alpha, beta);
    }

    Move moves[MAX_MOVES];
    int moveCount = generateLegalMoves(pos, moves);

    /* Terminal conditions */
    if (moveCount == 0) {
        if (isInCheck(pos, pos->sideToMove)) {
            /* Checkmate: use ply for correct mate-distance preference.
             * Lower ply = closer mate = more negative = preferred by opponent,
             * so after negation the mating side prefers faster mates. */
            return -MATE_SCORE + ply;
        } else {
            return 0; /* Stalemate */
        }
    }

    /* Score and order moves: captures (MVV-LVA), promotions, killers, history */
    scoreMoves(moves, moveCount, ply, pos->sideToMove);

    int bestScore = -2000000;

    for (int i = 0; i < moveCount; i++) {
        /* Pick the best remaining move (incremental selection sort) */
        pickBestMove(moves, moveCount, i);

        makeMove(pos, &moves[i]);
        int score = -search(pos, depth - 1, -beta, -alpha, ply + 1);
        undoMove(pos, &moves[i]);

        if (score > bestScore) {
            bestScore = score;
        }

        if (bestScore >= beta) {
            /* Beta cutoff: update ordering heuristics for quiet moves */
            if (moves[i].captured == EMPTY && moves[i].type != MOVE_PROMOTION) {
                updateKillers(&moves[i], ply);
                updateHistory(&moves[i], pos->sideToMove, depth);
            }
            return beta;
        }

        if (bestScore > alpha) {
            alpha = bestScore;
        }
    }

    return bestScore;
}

/* ========================================================
 *  Quiescence search — resolve captures/promotions at depth 0
 *  Uses MVV-LVA ordering for tactical moves.
 * ======================================================== */

static int quiesce(Position* pos, int alpha, int beta) {
    /* Stand-pat: the side to move can choose not to capture */
    int standPat = evaluate(pos) * pos->sideToMove;
    if (standPat >= beta) return beta;
    if (standPat > alpha) alpha = standPat;

    Move moves[MAX_MOVES];
    int moveCount = generateTacticalMoves(pos, moves);

    if (moveCount == 0) return standPat;

    /* Order captures/promotions by MVV-LVA */
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

        if (bestScore >= beta) {
            return beta;
        }
        if (bestScore > alpha) {
            alpha = bestScore;
        }
    }

    return (bestScore > standPat) ? bestScore : standPat;
}

/* ========================================================
 *  Root search entry point
 * ======================================================== */

int findBestMove(Position* pos, int depth, Move* bestMove) {
    /* Initialize ordering data for this search */
    initOrdering();

    int alpha = -2000000;
    int beta = 2000000;
    int bestScore = -2000000;

    Move moves[MAX_MOVES];
    int moveCount = generateLegalMoves(pos, moves);

    if (moveCount == 0) return 0;

    /* Score root moves */
    scoreMoves(moves, moveCount, 0, pos->sideToMove);

    for (int i = 0; i < moveCount; i++) {
        pickBestMove(moves, moveCount, i);

        makeMove(pos, &moves[i]);
        int score = -search(pos, depth - 1, -beta, -alpha, 1);
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
