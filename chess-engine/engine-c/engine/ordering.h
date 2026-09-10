#ifndef ORDERING_H
#define ORDERING_H

#include "../utils/types.h"

#define MAX_DEPTH 64
#define NUM_KILLERS 2

/* Score tier constants - higher = searched first.
 *
 * Ordering priority:
 *   1. Queen promotion captures  (QUEEN_PROMO_SCORE + CAPTURE_SCORE_BASE + mvvlva)
 *   2. Queen promotions           (QUEEN_PROMO_SCORE)
 *   3. All captures               (CAPTURE_SCORE_BASE + mvvlva)
 *   4. Under-promotions           (OTHER_PROMO_SCORE)
 *   5. Killer move #1             (KILLER_SCORE_1)
 *   6. Killer move #2             (KILLER_SCORE_2)
 *   7. History heuristic moves    (0..~1000000)
 *   8. Other quiet moves          (0)
 */
#define QUEEN_PROMO_SCORE  20000000
#define CAPTURE_SCORE_BASE 10000000
#define OTHER_PROMO_SCORE   9500000
#define KILLER_SCORE_1      9000000
#define KILLER_SCORE_2      8000000

/**
 * Initialize ordering data (clear killers and history).
 * Call at the start of each new search.
 */
void initOrdering(void);

/**
 * Score all moves in the list for ordering.
 * Assigns scores based on MVV-LVA (captures), promotions,
 * killer moves (quiet), and history heuristic (quiet).
 *
 * @param moves  Array of legal moves.
 * @param count  Number of moves in the array.
 * @param ply    Current ply (distance from root).
 * @param side   Side to move (WHITE or BLACK).
 */
void scoreMoves(Move moves[], int count, int ply, int side);

/**
 * Score only tactical moves (captures and promotions) using MVV-LVA.
 * Used in quiescence search where killers/history are not needed.
 */
void scoreCaptures(Move moves[], int count);

/**
 * Incremental selection sort: swap the highest-scored move
 * from startIndex..count-1 into position startIndex.
 * Called once per loop iteration so we only fully sort if no cutoff occurs.
 */
void pickBestMove(Move moves[], int count, int startIndex);

/**
 * Record a quiet move that caused a beta cutoff as a killer move.
 * Maintains two killer slots per ply (FIFO replacement).
 */
void updateKillers(const Move* move, int ply);

/**
 * Increment the history score for a quiet move that caused a beta cutoff.
 * Uses depth^2 weighting so deeper cutoffs contribute more.
 *
 * @param move   The move that caused the cutoff.
 * @param side   Side that made the move (WHITE or BLACK).
 * @param depth  Remaining search depth at the cutoff.
 */
void updateHistory(const Move* move, int side, int depth);

#endif
