#ifndef TYPES_H
#define TYPES_H

#include <stdint.h>

#define MAX_MOVES 256

typedef enum {
    MOVE_NORMAL = 0,
    MOVE_PROMOTION = 1,
    MOVE_CASTLE_KINGSIDE = 2,
    MOVE_CASTLE_QUEENSIDE = 3,
    MOVE_EN_PASSANT = 4
} MoveType;

typedef struct {
    int fromRow;
    int fromCol;
    int toRow;
    int toCol;
    int piece;
    int captured;
    int promotion;
    MoveType type;
    int prevEnPassantRow;
    int prevEnPassantCol;
    int prevCastlingRights;
    int prevHalfmoveClock;
    uint64_t prevHash;
    int score;
} Move;

typedef struct {
    int board[8][8];
    int sideToMove;
    int whiteKingMoved;
    int blackKingMoved;
    int whiteLeftRookMoved;
    int whiteRightRookMoved;
    int blackLeftRookMoved;
    int blackRightRookMoved;
    int enPassantRow;
    int enPassantCol;
    int halfmoveClock;
    int fullmoveNumber;
    int whiteKingRow;
    int whiteKingCol;
    int blackKingRow;
    int blackKingCol;
    uint64_t hash;

    /* Bitboards */
    uint64_t whitePieces;
    uint64_t blackPieces;
    uint64_t allPieces;
    uint64_t pawns;
    uint64_t knights;
    uint64_t bishops;
    uint64_t rooks;
    uint64_t queens;
    uint64_t kings;
} Position;

#endif