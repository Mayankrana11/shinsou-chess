#ifndef BITBOARD_H
#define BITBOARD_H

#include <stdint.h>
#include <stdbool.h>

/* Square indexing: 0 = a1, 63 = h8 */

static inline int square_to_index(int row, int col) {
    /* Current board[row][col] has row 0 = rank 8, row 7 = rank 1 */
    /* Bitboard index 0 = rank 1, col 0 (a1) */
    return (7 - row) * 8 + col;
}

static inline int index_to_row(int index) {
    return 7 - (index / 8);
}

static inline int index_to_col(int index) {
    return index % 8;
}

static inline void set_bit(uint64_t* bb, int square) {
    *bb |= (1ULL << square);
}

static inline void clear_bit(uint64_t* bb, int square) {
    *bb &= ~(1ULL << square);
}

static inline bool get_bit(uint64_t bb, int square) {
    return (bb & (1ULL << square)) != 0;
}

static inline int pop_count(uint64_t bb) {
    return __builtin_popcountll(bb);
}

static inline int lsb(uint64_t bb) {
    return __builtin_ctzll(bb);
}

#endif