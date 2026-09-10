#include <string.h>
#include "ordering.h"
#include "../utils/constants.h"

/* ========================================================
 *  Killer moves table
 *  Two killer slots per ply. Killers are quiet moves (non-capture,
 *  non-promotion) that caused beta cutoffs at a given ply.
 * ======================================================== */
static Move killers[MAX_DEPTH][NUM_KILLERS];

/* ========================================================
 *  History heuristic table
 *  Indexed by [side_index][fromRow][fromCol][toRow][toCol].
 *  side_index: 0 = WHITE, 1 = BLACK.
 *  Incremented when a quiet move causes a beta cutoff.
 * ======================================================== */
static int history[2][8][8][8][8];

/* --------------------------------------------------------
 *  MVV-LVA piece index
 *  Maps piece type to a simple value scale used for
 *  Most Valuable Victim - Least Valuable Attacker scoring.
 *  Higher index = more valuable.
 * -------------------------------------------------------- */
static int pieceIndex(int piece) {
    int type = piece > 0 ? piece : -piece;
    switch (type) {
        case 1:  return 1;   /* Pawn   */
        case 2:  return 3;   /* Knight */
        case 3:  return 3;   /* Bishop */
        case 4:  return 5;   /* Rook   */
        case 5:  return 9;   /* Queen  */
        case 6:  return 10;  /* King   */
        default: return 0;
    }
}

/* --------------------------------------------------------
 *  Compute MVV-LVA score for a capture.
 *  victimValue * 100 - attackerValue
 *  Examples:
 *    PxQ = 9*100 - 1 = 899 (highest common)
 *    NxQ = 9*100 - 3 = 897
 *    QxP = 1*100 - 9 =  91 (lowest common)
 * -------------------------------------------------------- */
static int mvvLvaScore(int victim, int attacker) {
    return pieceIndex(victim) * 100 - pieceIndex(attacker);
}

/* Check if two moves match by their coordinates */
static int movesMatch(const Move* a, const Move* b) {
    if (b->fromRow < 0) return 0; /* Killer slot not set */
    return a->fromRow == b->fromRow &&
           a->fromCol == b->fromCol &&
           a->toRow   == b->toRow   &&
           a->toCol   == b->toCol;
}

/* ========================================================
 *  Public API
 * ======================================================== */

void initOrdering(void) {
    /* Clear killer moves (set coordinates to -1 = unused) */
    for (int d = 0; d < MAX_DEPTH; d++) {
        for (int k = 0; k < NUM_KILLERS; k++) {
            killers[d][k].fromRow = -1;
            killers[d][k].fromCol = -1;
            killers[d][k].toRow   = -1;
            killers[d][k].toCol   = -1;
        }
    }

    /* Clear history table */
    memset(history, 0, sizeof(history));
}

void scoreMoves(Move moves[], int count, int ply, int side) {
    int sideIdx = (side == WHITE) ? 0 : 1;

    for (int i = 0; i < count; i++) {
        Move* m = &moves[i];
        m->score = 0;

        /* 1. Captures: MVV-LVA scoring */
        if (m->captured != EMPTY) {
            m->score += CAPTURE_SCORE_BASE + mvvLvaScore(m->captured, m->piece);
        }

        /* 2. Promotions */
        if (m->type == MOVE_PROMOTION) {
            int promoType = m->promotion > 0 ? m->promotion : -(m->promotion);
            if (promoType == WQUEEN) {
                m->score += QUEEN_PROMO_SCORE;
            } else {
                m->score += OTHER_PROMO_SCORE;
            }
        }

        /* 3. Quiet moves: check killers, then history */
        if (m->captured == EMPTY && m->type != MOVE_PROMOTION) {
            if (ply >= 0 && ply < MAX_DEPTH) {
                if (movesMatch(m, &killers[ply][0])) {
                    m->score = KILLER_SCORE_1;
                } else if (movesMatch(m, &killers[ply][1])) {
                    m->score = KILLER_SCORE_2;
                } else {
                    m->score = history[sideIdx]
                                      [m->fromRow][m->fromCol]
                                      [m->toRow][m->toCol];
                }
            }
        }
    }
}

void scoreCaptures(Move moves[], int count) {
    for (int i = 0; i < count; i++) {
        Move* m = &moves[i];
        m->score = 0;

        if (m->captured != EMPTY) {
            m->score += CAPTURE_SCORE_BASE + mvvLvaScore(m->captured, m->piece);
        }

        if (m->type == MOVE_PROMOTION) {
            int promoType = m->promotion > 0 ? m->promotion : -(m->promotion);
            if (promoType == WQUEEN) {
                m->score += QUEEN_PROMO_SCORE;
            } else {
                m->score += OTHER_PROMO_SCORE;
            }
        }
    }
}

void pickBestMove(Move moves[], int count, int startIndex) {
    int bestIdx = startIndex;
    int bestScore = moves[startIndex].score;

    for (int i = startIndex + 1; i < count; i++) {
        if (moves[i].score > bestScore) {
            bestScore = moves[i].score;
            bestIdx = i;
        }
    }

    if (bestIdx != startIndex) {
        Move temp = moves[startIndex];
        moves[startIndex] = moves[bestIdx];
        moves[bestIdx] = temp;
    }
}

void updateKillers(const Move* move, int ply) {
    if (ply < 0 || ply >= MAX_DEPTH) return;

    /* Don't store duplicates: skip if already killer #1 */
    if (movesMatch(move, &killers[ply][0])) return;

    /* Shift killer #1 → #2, store new move as #1 */
    killers[ply][1] = killers[ply][0];
    killers[ply][0] = *move;
}

void updateHistory(const Move* move, int side, int depth) {
    int sideIdx = (side == WHITE) ? 0 : 1;

    /* Increment by depth^2 — deeper cutoffs are more significant */
    history[sideIdx][move->fromRow][move->fromCol]
                    [move->toRow][move->toCol] += depth * depth;

    /* Cap to prevent overflow and keep history below killer bonuses */
    if (history[sideIdx][move->fromRow][move->fromCol]
                        [move->toRow][move->toCol] > 1000000) {
        history[sideIdx][move->fromRow][move->fromCol]
                        [move->toRow][move->toCol] = 1000000;
    }
}
