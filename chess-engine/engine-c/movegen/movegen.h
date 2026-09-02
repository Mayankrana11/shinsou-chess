#ifndef MOVEGEN_H
#define MOVEGEN_H

#include "../utils/types.h"

#define MAX_MOVES 256

int generatePseudoLegalMoves(Position* pos, Move moves[]);
int generateLegalMoves(Position* pos, Move moves[]);

#endif