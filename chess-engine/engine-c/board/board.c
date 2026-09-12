#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include "board.h"
#include "../utils/constants.h"
#include "../movegen/movegen.h"

/* Zobrist Hashing Tables */
static uint64_t piece_sq[12][8][8];
static uint64_t side_key;
static uint64_t castle_keys[4];
static uint64_t ep_keys[8];

static int pieceToZIndex(int p) {
    if (p == 0) return -1;
    return (p > 0) ? (p - 1) : (abs(p) + 5);
}

char pieceChar(int p) {
    switch(p) {
        case WPAWN: return 'P';
        case WKNIGHT: return 'N';
        case WBISHOP: return 'B';
        case WROOK: return 'R';
        case WQUEEN: return 'Q';
        case WKING: return 'K';
        case BPAWN: return 'p';
        case BKNIGHT: return 'n';
        case BBISHOP: return 'b';
        case BROOK: return 'r';
        case BQUEEN: return 'q';
        case BKING: return 'k';
        default: return '.';
    }
}

void clearBoard(Position* pos) {
    memset(pos, 0, sizeof(Position));
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            pos->board[i][j] = EMPTY;
        }
    }
}

void initBoard(Position* pos) {
    int start[8][8] = {
        {BROOK,BKNIGHT,BBISHOP,BQUEEN,BKING,BBISHOP,BKNIGHT,BROOK},
        {BPAWN,BPAWN,BPAWN,BPAWN,BPAWN,BPAWN,BPAWN,BPAWN},
        {0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0},
        {WPAWN,WPAWN,WPAWN,WPAWN,WPAWN,WPAWN,WPAWN,WPAWN},
        {WROOK,WKNIGHT,WBISHOP,WQUEEN,WKING,WBISHOP,WKNIGHT,WROOK}
    };

    clearBoard(pos);
    for(int i=0;i<8;i++) {
        for(int j=0;j<8;j++) {
            pos->board[i][j] = start[i][j];
        }
    }

    pos->sideToMove = WHITE;
    pos->whiteKingRow = RANK_1;
    pos->whiteKingCol = 4;
    pos->blackKingRow = RANK_8;
    pos->blackKingCol = 4;
    pos->halfmoveClock = 0;
    pos->fullmoveNumber = 1;
    pos->enPassantRow = -1;
    pos->enPassantCol = -1;
}

void printBoard(Position* pos) {
    printf("\n");
    for(int i=0;i<8;i++) {
        printf("%d  ", 8 - i);
        for(int j=0;j<8;j++) {
            printf("%c ", pieceChar(pos->board[i][j]));
        }
        printf("\n");
    }
    printf("\n   a b c d e f g h\n\n");
}

void copyPosition(Position* dst, const Position* src) {
    memcpy(dst, src, sizeof(Position));
}

int findKing(Position* pos, int color) {
    int kingPiece = (color == WHITE) ? WKING : BKING;
    for (int r = 0; r < 8; r++) {
        for (int c = 0; c < 8; c++) {
            if (pos->board[r][c] == kingPiece) {
                if (color == WHITE) {
                    pos->whiteKingRow = r;
                    pos->whiteKingCol = c;
                } else {
                    pos->blackKingRow = r;
                    pos->blackKingCol = c;
                }
                return 1;
            }
        }
    }
    return 0;
}

static int inBounds(int r, int c) {
    return r >= 0 && r < 8 && c >= 0 && c < 8;
}

static int isWhitePiece(int p) { return p > 0; }
static int isBlackPiece(int p) { return p < 0; }

int pawnAttacks(int r, int c, int color, int targetR, int targetC) {
    int dir = (color == WHITE) ? -1 : 1;
    return (targetR == r + dir) && (targetC == c - 1 || targetC == c + 1);
}

int knightAttacks(int r, int c, int targetR, int targetC) {
    int dr = targetR - r;
    int dc = targetC - c;
    return ((dr == 2 || dr == -2) && (dc == 1 || dc == -1)) ||
           ((dr == 1 || dr == -1) && (dc == 2 || dc == -2));
}

int bishopAttacks(Position* pos, int r, int c, int targetR, int targetC) {
    int dr = targetR - r;
    int dc = targetC - c;
    if (dr == 0 || dc == 0 || (dr != dc && dr != -dc)) return 0;
    int stepR = (dr > 0) ? 1 : -1;
    int stepC = (dc > 0) ? 1 : -1;
    int nr = r + stepR;
    int nc = c + stepC;
    while (nr != targetR && nc != targetC) {
        if (pos->board[nr][nc] != EMPTY) return 0;
        nr += stepR;
        nc += stepC;
    }
    return 1;
}

int rookAttacks(Position* pos, int r, int c, int targetR, int targetC) {
    if (r != targetR && c != targetC) return 0;
    int stepR = (targetR > r) ? 1 : (targetR < r) ? -1 : 0;
    int stepC = (targetC > c) ? 1 : (targetC < c) ? -1 : 0;
    int nr = r + stepR;
    int nc = c + stepC;
    while (nr != targetR || nc != targetC) {
        if (pos->board[nr][nc] != EMPTY) return 0;
        nr += stepR;
        nc += stepC;
    }
    return 1;
}

int queenAttacks(Position* pos, int r, int c, int targetR, int targetC) {
    return bishopAttacks(pos, r, c, targetR, targetC) || rookAttacks(pos, r, c, targetR, targetC);
}

int kingAttacks(int r, int c, int targetR, int targetC) {
    int dr = targetR - r;
    int dc = targetC - c;
    return (dr >= -1 && dr <= 1) && (dc >= -1 && dc <= 1) && (dr != 0 || dc != 0);
}

int isSquareAttacked(Position* pos, int row, int col, int byColor) {
    for (int r = 0; r < 8; r++) {
        for (int c = 0; c < 8; c++) {
            int piece = pos->board[r][c];
            if (piece == EMPTY) continue;
            if ((byColor == WHITE && !isWhitePiece(piece)) || (byColor == BLACK && !isBlackPiece(piece))) continue;
            int type = piece > 0 ? piece : -piece;
            switch (type) {
                case WPAWN: if (pawnAttacks(r, c, byColor, row, col)) return 1; break;
                case WKNIGHT: if (knightAttacks(r, c, row, col)) return 1; break;
                case WBISHOP: if (bishopAttacks(pos, r, c, row, col)) return 1; break;
                case WROOK: if (rookAttacks(pos, r, c, row, col)) return 1; break;
                case WQUEEN: if (queenAttacks(pos, r, c, row, col)) return 1; break;
                case WKING: if (kingAttacks(r, c, row, col)) return 1; break;
            }
        }
    }
    return 0;
}

int isInCheck(Position* pos, int color) {
    int kingRow = (color == WHITE) ? pos->whiteKingRow : pos->blackKingRow;
    int kingCol = (color == WHITE) ? pos->whiteKingCol : pos->blackKingCol;
    int enemyColor = (color == WHITE) ? BLACK : WHITE;
    return isSquareAttacked(pos, kingRow, kingCol, enemyColor);
}

int getCastlingRights(Position* pos) {
    int rights = 0;
    if (!pos->whiteKingMoved) {
        if (!pos->whiteRightRookMoved) rights |= CASTLE_WHITE_KINGSIDE;
        if (!pos->whiteLeftRookMoved) rights |= CASTLE_WHITE_QUEENSIDE;
    }
    if (!pos->blackKingMoved) {
        if (!pos->blackRightRookMoved) rights |= CASTLE_BLACK_KINGSIDE;
        if (!pos->blackLeftRookMoved) rights |= CASTLE_BLACK_QUEENSIDE;
    }
    return rights;
}

void setCastlingRights(Position* pos, int rights) {
    pos->whiteKingMoved = !(rights & (CASTLE_WHITE_KINGSIDE | CASTLE_WHITE_QUEENSIDE));
    pos->whiteRightRookMoved = !(rights & CASTLE_WHITE_KINGSIDE);
    pos->whiteLeftRookMoved = !(rights & CASTLE_WHITE_QUEENSIDE);
    pos->blackKingMoved = !(rights & (CASTLE_BLACK_KINGSIDE | CASTLE_BLACK_QUEENSIDE));
    pos->blackRightRookMoved = !(rights & CASTLE_BLACK_KINGSIDE);
    pos->blackLeftRookMoved = !(rights & CASTLE_BLACK_QUEENSIDE);
}

void makeMove(Position* pos, Move* move) {
    move->prevHash = pos->hash;
    move->prevEnPassantRow = pos->enPassantRow;
    move->prevEnPassantCol = pos->enPassantCol;
    move->prevCastlingRights = getCastlingRights(pos);
    move->prevHalfmoveClock = pos->halfmoveClock;

    int piece = pos->board[move->fromRow][move->fromCol];
    int captured = pos->board[move->toRow][move->toCol];
    move->piece = piece;
    move->captured = captured;

    /* Update Hash incrementally */
    uint64_t h = pos->hash;
    h ^= piece_sq[pieceToZIndex(piece)][move->fromRow][move->fromCol];
    if (captured != EMPTY) {
        h ^= piece_sq[pieceToZIndex(captured)][move->toRow][move->toCol];
    }

    pos->board[move->fromRow][move->fromCol] = EMPTY;

    if (move->type == MOVE_EN_PASSANT) {
        int capturedRow = (pos->sideToMove == WHITE) ? move->toRow + 1 : move->toRow - 1;
        pos->board[capturedRow][move->toCol] = EMPTY;
        /* Captured pawn was not at move->toRow, but at capturedRow */
        h ^= piece_sq[pieceToZIndex(pos->sideToMove == WHITE ? BPAWN : WPAWN)][capturedRow][move->toCol];
    } else {
        if (captured != EMPTY) {
            // Captured piece already XORed out at the start of makeMove
        }
    }

    pos->board[move->toRow][move->toCol] = move->promotion ? move->promotion : piece;
    h ^= piece_sq[pieceToZIndex(pos->board[move->toRow][move->toCol])][move->toRow][move->toCol];

    if (move->type == MOVE_CASTLE_KINGSIDE) {
        if (pos->sideToMove == WHITE) {
            pos->board[RANK_1][5] = WROOK; pos->board[RANK_1][7] = EMPTY;
            h ^= piece_sq[pieceToZIndex(WROOK)][RANK_1][7];
            h ^= piece_sq[pieceToZIndex(WROOK)][RANK_1][5];
        } else {
            pos->board[RANK_8][5] = BROOK; pos->board[RANK_8][7] = EMPTY;
            h ^= piece_sq[pieceToZIndex(BROOK)][RANK_8][7];
            h ^= piece_sq[pieceToZIndex(BROOK)][RANK_8][5];
        }
    } else if (move->type == MOVE_CASTLE_QUEENSIDE) {
        if (pos->sideToMove == WHITE) {
            pos->board[RANK_1][3] = WROOK; pos->board[RANK_1][0] = EMPTY;
            h ^= piece_sq[pieceToZIndex(WROOK)][RANK_1][0];
            h ^= piece_sq[pieceToZIndex(WROOK)][RANK_1][3];
        } else {
            pos->board[RANK_8][3] = BROOK; pos->board[RANK_8][0] = EMPTY;
            h ^= piece_sq[pieceToZIndex(BROOK)][RANK_8][0];
            h ^= piece_sq[pieceToZIndex(BROOK)][RANK_8][3];
        }
    }

    if (piece == WKING) {
        pos->whiteKingRow = move->toRow; pos->whiteKingCol = move->toCol;
        pos->whiteKingMoved = 1;
    } else if (piece == BKING) {
        pos->blackKingRow = move->toRow; pos->blackKingCol = move->toCol;
        pos->blackKingMoved = 1;
    }

    if (move->type == MOVE_CASTLE_KINGSIDE) {
        if (pos->sideToMove == WHITE) pos->whiteRightRookMoved = 1;
        else pos->blackRightRookMoved = 1;
    } else if (move->type == MOVE_CASTLE_QUEENSIDE) {
        if (pos->sideToMove == WHITE) pos->whiteLeftRookMoved = 1;
        else pos->blackLeftRookMoved = 1;
    }

    if (piece == WROOK) {
        if (move->fromRow == RANK_1 && move->fromCol == 0) pos->whiteLeftRookMoved = 1;
        if (move->fromRow == RANK_1 && move->fromCol == 7) pos->whiteRightRookMoved = 1;
    } else if (piece == BROOK) {
        if (move->fromRow == RANK_8 && move->fromCol == 0) pos->blackLeftRookMoved = 1;
        if (move->fromRow == RANK_8 && move->fromCol == 7) pos->blackRightRookMoved = 1;
    }

    int ptype = piece > 0 ? piece : -piece;
    if (ptype == WPAWN) {
        pos->halfmoveClock = 0;
        if (move->fromRow - move->toRow == 2 || move->toRow - move->fromRow == 2) {
            pos->enPassantRow = (move->fromRow + move->toRow) / 2;
            pos->enPassantCol = move->fromCol;
            h ^= ep_keys[pos->enPassantCol];
        } else {
            pos->enPassantRow = -1; pos->enPassantCol = -1;
        }
    } else {
        pos->halfmoveClock++;
        pos->enPassantRow = -1; pos->enPassantCol = -1;
    }

    if (captured != EMPTY) {
        pos->halfmoveClock = 0;
    }

    /* Update Hash for Side and Castling */
    h ^= side_key;
    int oldRights = move->prevCastlingRights;
    int newRights = getCastlingRights(pos);
    if ((oldRights & CASTLE_WHITE_KINGSIDE) != (newRights & CASTLE_WHITE_KINGSIDE)) h ^= castle_keys[0];
    if ((oldRights & CASTLE_WHITE_QUEENSIDE) != (newRights & CASTLE_WHITE_QUEENSIDE)) h ^= castle_keys[1];
    if ((oldRights & CASTLE_BLACK_KINGSIDE) != (newRights & CASTLE_BLACK_KINGSIDE)) h ^= castle_keys[2];
    if ((oldRights & CASTLE_BLACK_QUEENSIDE) != (newRights & CASTLE_BLACK_QUEENSIDE)) h ^= castle_keys[3];

    pos->sideToMove = -pos->sideToMove;
    if (pos->sideToMove == WHITE) pos->fullmoveNumber++;
    pos->hash = h;
}

void undoMove(Position* pos, Move* move) {
    int piece = move->piece;
    int captured = move->captured;

    pos->board[move->fromRow][move->fromCol] = piece;
    pos->board[move->toRow][move->toCol] = captured;

    if (move->type == MOVE_EN_PASSANT) {
        int capturedRow = (pos->sideToMove == BLACK) ? move->toRow + 1 : move->toRow - 1;
        pos->board[capturedRow][move->toCol] = (pos->sideToMove == BLACK) ? BPAWN : WPAWN;
        pos->board[move->toRow][move->toCol] = EMPTY;
    } else if (move->type == MOVE_PROMOTION) {
        pos->board[move->fromRow][move->fromCol] = piece;
        pos->board[move->toRow][move->toCol] = captured;
    }

    if (move->type == MOVE_CASTLE_KINGSIDE) {
        if (pos->sideToMove == BLACK) {
            pos->board[RANK_1][7] = WROOK; pos->board[RANK_1][5] = EMPTY;
        } else {
            pos->board[RANK_8][7] = BROOK; pos->board[RANK_8][5] = EMPTY;
        }
    } else if (move->type == MOVE_CASTLE_QUEENSIDE) {
        if (pos->sideToMove == BLACK) {
            pos->board[RANK_1][0] = WROOK; pos->board[RANK_1][3] = EMPTY;
        } else {
            pos->board[RANK_8][0] = BROOK; pos->board[RANK_8][3] = EMPTY;
        }
    }

    if (piece == WKING) {
        pos->whiteKingRow = move->fromRow; pos->whiteKingCol = move->fromCol;
    } else if (piece == BKING) {
        pos->blackKingRow = move->fromRow; pos->blackKingCol = move->fromCol;
    }

    setCastlingRights(pos, move->prevCastlingRights);
    pos->enPassantRow = move->prevEnPassantRow;
    pos->enPassantCol = move->prevEnPassantCol;
    pos->halfmoveClock = move->prevHalfmoveClock;
    pos->sideToMove = -pos->sideToMove;
    if (pos->sideToMove == BLACK) pos->fullmoveNumber--;
    pos->hash = move->prevHash;
}

int isCheckmate(Position* pos) {
    if (!isInCheck(pos, pos->sideToMove)) return 0;
    Move moves[MAX_MOVES];
    int legalCount = generateLegalMoves(pos, moves);
    return legalCount == 0;
}

int isStalemate(Position* pos) {
    if (isInCheck(pos, pos->sideToMove)) return 0;
    Move moves[MAX_MOVES];
    int legalCount = generateLegalMoves(pos, moves);
    return legalCount == 0;
}

int isTerminal(Position* pos, int* result) {
    if (isCheckmate(pos)) {
        if (result) *result = (pos->sideToMove == WHITE) ? -1 : 1;
        return 1;
    }
    if (isStalemate(pos)) {
        if (result) *result = 0;
        return 1;
    }
    if (pos->halfmoveClock >= 100) {
        if (result) *result = 0;
        return 1;
    }
    return 0;
}

void initZobrist() {
    srand(time(NULL));
    for (int i = 0; i < 12; i++) {
        for (int r = 0; r < 8; r++) {
            for (int c = 0; c < 8; c++) {
                piece_sq[i][r][c] = ((uint64_t)rand() << 32) | rand();
            }
        }
    }
    side_key = ((uint64_t)rand() << 32) | rand();
    for (int i = 0; i < 4; i++) {
        castle_keys[i] = ((uint64_t)rand() << 32) | rand();
    }
    for (int i = 0; i < 8; i++) {
        ep_keys[i] = ((uint64_t)rand() << 32) | rand();
    }
}

uint64_t computeHash(Position* pos) {
    uint64_t h = 0;
    for (int r = 0; r < 8; r++) {
        for (int c = 0; c < 8; c++) {
            int p = pos->board[r][c];
            if (p != EMPTY) {
                h ^= piece_sq[pieceToZIndex(p)][r][c];
            }
        }
    }
    if (pos->sideToMove == BLACK) h ^= side_key;

    int rights = getCastlingRights(pos);
    if (rights & CASTLE_WHITE_KINGSIDE) h ^= castle_keys[0];
    if (rights & CASTLE_WHITE_QUEENSIDE) h ^= castle_keys[1];
    if (rights & CASTLE_BLACK_KINGSIDE) h ^= castle_keys[2];
    if (rights & CASTLE_BLACK_QUEENSIDE) h ^= castle_keys[3];

    if (pos->enPassantCol != -1) {
        h ^= ep_keys[pos->enPassantCol];
    }
    return h;
}
