#ifndef TYPES_H
#define TYPES_H

typedef struct {

    int fromRow;
    int fromCol;

    int toRow;
    int toCol;

    int piece;
    int captured;

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

} Position;

#endif