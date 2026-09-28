#include "attacks.h"
#include <stdio.h>
#include <stdlib.h>

uint64_t rook_attacks[64][4096];
uint64_t bishop_attacks[64][4096];
uint64_t knight_attacks[64];
uint64_t king_attacks[64];

/* Simple precomputation for non-sliding pieces */
static void init_knight_attacks() {
    int dr[] = {2, 2, -2, -2, 1, 1, -1, -1};
    int dc[] = {1, -1, 1, -1, 2, -2, 2, -2};
    for (int sq = 0; sq < 64; sq++) {
        int r = sq / 8;
        int c = sq % 8;
        uint64_t bb = 0;
        for (int i = 0; i < 8; i++) {
            int nr = r + dr[i];
            int nc = c + dc[i];
            if (nr >= 0 && nr < 8 && nc >= 0 && nc < 8) {
                bb |= (1ULL << (nr * 8 + nc));
            }
        }
        knight_attacks[sq] = bb;
    }
}

static void init_king_attacks() {
    for (int sq = 0; sq < 64; sq++) {
        int r = sq / 8;
        int c = sq % 8;
        uint64_t bb = 0;
        for (int dr = -1; dr <= 1; dr++) {
            for (int dc = -1; dc <= 1; dc++) {
                if (dr == 0 && dc == 0) continue;
                int nr = r + dr;
                int nc = c + dc;
                if (nr >= 0 && nr < 8 && nc >= 0 && nc < 8) {
                    bb |= (1ULL << (nr * 8 + nc));
                }
            }
        }
        king_attacks[sq] = bb;
    }
}

/*
   Note: For now, I'll implement a simplified version of sliding attacks
   to get the engine running, then upgrade to full Magic Bitboards.
   A simple ray-casting attack generator.
*/
uint64_t get_rook_attacks(int sq, uint64_t occupancy) {
    uint64_t attacks = 0;
    int r = sq / 8;
    int c = sq % 8;
    int dr[] = {1, -1, 0, 0};
    int dc[] = {0, 0, 1, -1};
    for (int i = 0; i < 4; i++) {
        for (int dist = 1; dist < 8; dist++) {
            int nr = r + dr[i] * dist;
            int nc = c + dc[i] * dist;
            if (nr < 0 || nr >= 8 || nc < 0 || nc >= 8) break;
            int targetSq = nr * 8 + nc;
            attacks |= (1ULL << targetSq);
            if (occupancy & (1ULL << targetSq)) break;
        }
    }
    return attacks;
}

uint64_t get_bishop_attacks(int sq, uint64_t occupancy) {
    uint64_t attacks = 0;
    int r = sq / 8;
    int c = sq % 8;
    int dr[] = {1, 1, -1, -1};
    int dc[] = {1, -1, 1, -1};
    for (int i = 0; i < 4; i++) {
        for (int dist = 1; dist < 8; dist++) {
            int nr = r + dr[i] * dist;
            int nc = c + dc[i] * dist;
            if (nr < 0 || nr >= 8 || nc < 0 || nc >= 8) break;
            int targetSq = nr * 8 + nc;
            attacks |= (1ULL << targetSq);
            if (occupancy & (1ULL << targetSq)) break;
        }
    }
    return attacks;
}

void init_attacks() {
    init_knight_attacks();
    init_king_attacks();
    /* Magic table initialization will go here */
}
