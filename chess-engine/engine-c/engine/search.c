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
#define NULL_MOVE_REDUCTION 2
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

    if (depth <= 0) {
        return quiesce(pos, alpha, beta);
    }

    int inCheck = isInCheck(pos, pos->sideToMove);

    /* 2. Null Move Pruning (NMP) */
    /* Apply NMP at non-PV nodes (beta - alpha == 1) at depths >= 3 when not in check. */
    if (depth >= 3 && !inCheck && ply > 0 && (beta - alpha == 1) && beta < MATE_SCORE - 100 && alpha > -MATE_SCORE + 100) {
        Position nullPos = *pos;
        nullPos.sideToMove = -pos->sideToMove;
        nullPos.enPassantRow = -1;
        nullPos.enPassantCol = -1;
        nullPos.halfmoveClock++;
        nullPos.hash = computeHash(&nullPos);

        int nullScore = -search(&nullPos, depth - 1 - NULL_MOVE_REDUCTION, -beta, -beta + 1, ply + 1);
        if (nullScore >= beta) {
            return nullScore;
        }
    }

    Move moves[MAX_MOVES];
    int moveCount = generateLegalMoves(pos, moves);

    if (moveCount == 0) {
        if (inCheck) {
            return -MATE_SCORE + ply;
        } else {
            return 0;
        }
    }

    scoreMoves(moves, moveCount, ply, pos->sideToMove);
    if (entry) {
        for (int i = 0; i < moveCount; i++) {
            if (moves[i].fromRow == entry->bestMove.fromRow &&
                moves[i].fromCol == entry->bestMove.fromCol &&
                moves[i].toRow == entry->bestMove.toRow &&
                moves[i].toCol == entry->bestMove.toCol &&
                moves[i].promotion == entry->bestMove.promotion) {
                moves[i].score = 30000000;
                break;
            }
        }
    }

    int bestScore = -2000000;
    Move bestMove = moves[0];
    int alphaOrig = alpha;
    int firstMove = 1;
    int staticEval = (depth == 1 && !inCheck) ? (evaluate(pos) * pos->sideToMove) : 0;

    for (int i = 0; i < moveCount; i++) {
        pickBestMove(moves, moveCount, i);

        /* 3. Futility Pruning (frontier nodes, after searching at least one move) */
        if (!firstMove && depth == 1 && !inCheck &&
            moves[i].captured == EMPTY && moves[i].type != MOVE_PROMOTION) {
            if (staticEval + FUTILITY_THRESHOLD <= alpha) {
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
            if (i > 3 && depth >= 3 && !inCheck && moves[i].captured == EMPTY && moves[i].type != MOVE_PROMOTION) {
                searchDepth -= 1; // 1-ply reduction
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
    if (standPat >= beta) return standPat;
    if (standPat > alpha) alpha = standPat;

    Move moves[MAX_MOVES];
    int moveCount = generateTacticalMoves(pos, moves);
    if (moveCount == 0) return standPat;

    scoreCaptures(moves, moveCount);

    int bestScore = standPat;
    for (int i = 0; i < moveCount; i++) {
        pickBestMove(moves, moveCount, i);
        makeMove(pos, &moves[i]);
        int score = -quiesce(pos, -beta, -alpha);
        undoMove(pos, &moves[i]);

        if (score > bestScore) {
            bestScore = score;
        }
        if (bestScore >= beta) return bestScore;
        if (bestScore > alpha) alpha = bestScore;
    }

    return bestScore;
}

/* ========================================================
 *  Search Entry Point
 * ======================================================== */

int findBestMove(Position* pos, int depth, Move* bestMove) {
    Move moves[MAX_MOVES];
    int moveCount = generateLegalMoves(pos, moves);
    if (moveCount == 0) return 0;

    Move currentBestMove = moves[0];
    int bestScore = -2000000;

    /* Iterative Deepening (1 .. depth) */
    for (int d = 1; d <= depth; d++) {
        scoreMoves(moves, moveCount, 0, pos->sideToMove);
        if (d > 1) {
            for (int i = 0; i < moveCount; i++) {
                if (moves[i].fromRow == currentBestMove.fromRow &&
                    moves[i].fromCol == currentBestMove.fromCol &&
                    moves[i].toRow == currentBestMove.toRow &&
                    moves[i].toCol == currentBestMove.toCol &&
                    moves[i].promotion == currentBestMove.promotion) {
                    moves[i].score = 30000000;
                    break;
                }
            }
        }

        int iterBestScore = -2000000;
        Move iterBestMove = moves[0];
        int alpha = -2000000;
        int beta = 2000000;

        for (int i = 0; i < moveCount; i++) {
            pickBestMove(moves, moveCount, i);
            makeMove(pos, &moves[i]);
            int score = -search(pos, d - 1, -beta, -alpha, 1);
            undoMove(pos, &moves[i]);

            if (score > iterBestScore) {
                iterBestScore = score;
                iterBestMove = moves[i];
            }
            if (score > alpha) {
                alpha = score;
            }
        }

        currentBestMove = iterBestMove;
        bestScore = iterBestScore;
    }

    if (bestMove) *bestMove = currentBestMove;
    return bestScore;
}
