#ifndef ATTACKS_H
#define ATTACKS_H

#include "../utils/types.h"
#include "../utils/bitboard.h"

/* Magic Bitboard Tables */
extern uint64_t rook_attacks[64][4096];
extern uint64_t bishop_attacks[64][4096];
extern uint64_t knight_attacks[64];
extern uint64_t king_attacks[64];

void init_attacks();
uint64_t get_rook_attacks(int sq, uint64_t occupancy);
uint64_t get_bishop_attacks(int sq, uint64_t occupancy);

#endif