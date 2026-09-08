#ifndef EVAL_H
#define EVAL_H

#include "../board/board.h"

/**
 * Evaluates the current position from White's perspective.
 * Positive score means White is better, negative means Black is better.
 *
 * @param pos The current board position.
 * @return The evaluation score.
 */
int evaluate(Position* pos);

#endif
