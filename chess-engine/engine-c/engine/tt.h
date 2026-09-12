#ifndef TT_H
#define TT_H

#include "../utils/types.h"
#include <stdint.h>

typedef enum {
    EXACT,
    LOWERBOUND,
    UPPERBOUND
} TTFlag;

typedef struct {
    uint64_t hash;
    int depth;
    int score;
    TTFlag flag;
    Move bestMove;
} TTEntry;

void initTT();
TTEntry* ttProbe(uint64_t hash);
void ttStore(uint64_t hash, int depth, int score, TTFlag flag, Move bestMove);

#endif
