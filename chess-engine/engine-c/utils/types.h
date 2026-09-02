#ifndef TYPES_H
#define TYPES_H

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
} Position;

#endif