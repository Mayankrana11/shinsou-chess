#ifndef BOARD_H
#define BOARD_H

#include "../utils/types.h"

void initBoard(Position* pos);
void clearBoard(Position* pos);
void printBoard(Position* pos);
void copyPosition(Position* dst, const Position* src);
int findKing(Position* pos, int color);
int isInCheck(Position* pos, int color);
int getCastlingRights(Position* pos);
void setCastlingRights(Position* pos, int rights);
void makeMove(Position* pos, Move* move);
void undoMove(Position* pos, Move* move);
int isCheckmate(Position* pos);
int isStalemate(Position* pos);
int isTerminal(Position* pos, int* result);

/* Zobrist Hashing */
void initZobrist();
uint64_t computeHash(Position* pos);
extern uint64_t side_key;

#endif