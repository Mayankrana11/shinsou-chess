#ifndef SEARCH_H
#define SEARCH_H

#include "../utils/types.h"
#include <stdint.h>

// Perft utilities
uint64_t perft(Position* pos, int depth);
void perftDivide(Position* pos, int depth);

/**
 * Searches for the best move for the current player.
 *
 * @param pos The current position.
 * @param depth The search depth.
 * @param bestMove Pointer to a Move struct where the best move will be stored.
 * @return The score of the best move from the current player's perspective.
 */
int findBestMove(Position* pos, int depth, Move* bestMove);

/**
 * Core Negamax search with Alpha-Beta pruning.
 *
 * @param pos The current position.
 * @param depth Remaining search depth.
 * @param alpha The lower bound of the search window.
 * @param beta The upper bound of the search window.
 * @return The evaluated score for the player to move.
 */
int search(Position* pos, int depth, int alpha, int beta);

#endif
