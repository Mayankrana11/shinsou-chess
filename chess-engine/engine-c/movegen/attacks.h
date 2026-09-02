#ifndef ATTACKS_H
#define ATTACKS_H

#include "../utils/types.h"

int isSquareAttacked(Position* pos, int row, int col, int byColor);
int isInCheck(Position* pos, int color);

#endif