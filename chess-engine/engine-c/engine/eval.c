#include <stdio.h>
#include "eval.h"
#include "../utils/constants.h"

// Material values
#define PAWN_VALUE   100
#define KNIGHT_VALUE 320
#define BISHOP_VALUE  330
#define ROOK_VALUE    500
#define QUEEN_VALUE   900

// Piece-Square Tables (PST)
// These are defined from White's perspective.
// For Black, we mirror the table.
// Index [r][c] corresponds to board[r][c] (0=Rank 8, 7=Rank 1)

static const int pawnPST[8][8] = {
    { 50, 50, 50, 50, 50, 50, 50, 50 },
    { 30, 20, 20, 30, 30, 20, 20, 30 },
    { 10, 10, 20, 30, 30, 20, 10, 10 },
    { 5,  5,  10, 20, 20, 10,  5,  5 },
    { 0,  0,  0,  0,  0,  0,  0,  0 },
    { 0,  0,  0,  0,  0,  0,  0,  0 },
    { 0,  0,  0,  0,  0,  0,  0,  0 },
    { 0,  0,  0,  0,  0,  0,  0,  0 }
};

static const int knightPST[8][8] = {
    { -50, -40, -30, -30, -30, -30, -40, -50 },
    { -40, -20,  0,  0,  0,  0, -20, -40 },
    { -30,  0,  10, 15, 15, 10,  0, -30 },
    { -30,  5,  15, 20, 20, 15,  5, -30 },
    { -30,  0,  15, 20, 20, 15,  0, -30 },
    { -30,  5,  10, 15, 15, 10,  5, -30 },
    { -40, -20,  0,  5,  5,  0, -20, -40 },
    { -50, -40, -30, -30, -30, -30, -40, -50 }
};

static const int bishopPST[8][8] = {
    { -20, -10, -10, -10, -10, -10, -10, -20 },
    { -10,  0,  0,  0,  0,  0,  0, -10 },
    { -10,  0,  5, 10, 10,  5,  0, -10 },
    { -10,  5,  10, 15, 15, 10,  5, -10 },
    { -10,  0,  10, 15, 15, 10,  0, -10 },
    { -10,  0,  5, 10, 10,  5,  0, -10 },
    { -10,  0,  0,  0,  0,  0,  0, -10 },
    { -20, -10, -10, -10, -10, -10, -10, -20 }
};

static const int rookPST[8][8] = {
    { 0,  0,  0,  5,  5,  0,  0,  0 },
    { -5,  0,  0,  0,  0,  0,  0, -5 },
    { -5,  0,  0,  0,  0,  0,  0, -5 },
    { -5,  0,  0,  0,  0,  0,  0, -5 },
    { -5,  0,  0,  0,  0,  0,  0, -5 },
    { -5,  0,  0,  0,  0,  0,  0, -5 },
    { -5,  0,  0,  0,  0,  0,  0, -5 },
    { 0,  0,  0,  0,  0,  0,  0,  0 }
};

static const int queenPST[8][8] = {
    { -20, -10, -10, -5, -5, -10, -10, -20 },
    { -10,  0,  0,  0,  0,  0,  0, -10 },
    { -10,  0,  5,  5,  5,  5,  0, -10 },
    { -5,   0,  5,  10, 10,  5,  0,  -5 },
    { 0,    0,  5,  10, 10,  5,  0,  0 },
    { -10,  0,  5,  5,  5,  5,  0, -10 },
    { -10,  0,  0,  0,  0,  0,  0, -10 },
    { -20, -10, -10, -5, -5, -10, -10, -20 }
};

static const int kingPST[8][8] = {
    { -30, -40, -40, -50, -50, -40, -40, -30 },
    { -30, -40, -40, -50, -50, -40, -40, -30 },
    { -30, -40, -40, -50, -50, -40, -40, -30 },
    { -30, -40, -40, -50, -50, -40, -40, -30 },
    { -20, -30, -30, -40, -40, -30, -30, -20 },
    { -10, -20, -20, -20, -20, -20, -20, -10 },
    { 20,  20,  0,  0,  0,  0,  20,  20 },
    { 20,  30,  10,  0,  0,  10,  30,  20 }
};

int getPieceValue(int piece) {
    int type = piece > 0 ? piece : -piece;
    switch (type) {
        case WPAWN:   return PAWN_VALUE;
        case WKNIGHT:  return KNIGHT_VALUE;
        case WBISHOP:  return BISHOP_VALUE;
        case WROOK:    return ROOK_VALUE;
        case WQUEEN:   return QUEEN_VALUE;
        case WKING:   return 0;
        default:      return 0;
    }
}

int evaluate(Position* pos) {
    int score = 0;
    int whiteBishops = 0;
    int blackBishops = 0;

    for (int r = 0; r < 8; r++) {
        for (int c = 0; c < 8; c++) {
            int piece = pos->board[r][c];
            if (piece == EMPTY) continue;

            int type = piece > 0 ? piece : -piece;
            int val = getPieceValue(piece);
            int pst = 0;

            if (piece > 0) { // White
                score += val;
                switch (type) {
                    case WPAWN:   pst = pawnPST[r][c]; break;
                    case WKNIGHT: pst = knightPST[r][c]; break;
                    case WBISHOP: pst = bishopPST[r][c]; whiteBishops++; break;
                    case WROOK:   pst = rookPST[r][c]; break;
                    case WQUEEN:  pst = queenPST[r][c]; break;
                    case WKING:   pst = kingPST[r][c]; break;
                }
                score += pst;
            } else { // Black
                score -= val;
                // Mirror PST for Black: row r becomes 7-r
                switch (type) {
                    case WPAWN:   pst = pawnPST[7 - r][c]; break;
                    case WKNIGHT: pst = knightPST[7 - r][c]; break;
                    case WBISHOP: pst = bishopPST[7 - r][c]; blackBishops++; break;
                    case WROOK:   pst = rookPST[7 - r][c]; break;
                    case WQUEEN:  pst = queenPST[7 - r][c]; break;
                    case WKING:   pst = kingPST[7 - r][c]; break;
                }
                score -= pst;
            }
        }
    }

    // Positional Factor: Bishop Pair Bonus
    if (whiteBishops == 2) score += 50;
    if (blackBishops == 2) score -= 50;

    return score;
}
