#include "tt.h"
#include <string.h>

#define TT_SIZE (1 << 20)
static TTEntry tt[TT_SIZE];

void initTT() {
    memset(tt, 0, sizeof(tt));
}

TTEntry* ttProbe(uint64_t hash) {
    uint64_t index = hash % TT_SIZE;
    if (tt[index].hash == hash) {
        return &tt[index];
    }
    return NULL;
}

void ttStore(uint64_t hash, int depth, int score, TTFlag flag, Move bestMove) {
    uint64_t index = hash % TT_SIZE;

    /* Replace if depth is better or if it's a fresh entry */
    if (tt[index].hash == 0 || depth > tt[index].depth) {
        tt[index].hash = hash;
        tt[index].depth = depth;
        tt[index].score = score;
        tt[index].flag = flag;
        tt[index].bestMove = bestMove;
    }
}
