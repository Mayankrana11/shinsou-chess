#ifndef SEARCH_H
#define SEARCH_H

#include "../utils/types.h"
#include <stdint.h>

uint64_t perft(Position* pos, int depth);
void perftDivide(Position* pos, int depth);

#endif