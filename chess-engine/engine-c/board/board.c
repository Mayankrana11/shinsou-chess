#include <stdio.h>
#include <string.h>

#include "board.h"
#include "../utils/constants.h"

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
                case WPAWN:
                    if (pawnAttacks(r, c, byColor, row, col)) return 1;
                    break;
                case WKNIGHT:
                    if (knightAttacks(r, c, row, col)) return 1;
                    break;
                case WBISHOP:
                    if (bishopAttacks(pos, r, c, row, col)) return 1;
                    break;
                case WROOK:
                    if (rookAttacks(pos, r, c, row, col)) return 1;
                    break;
                case WQUEEN:
                    if (queenAttacks(pos, r, c, row, col)) return 1;
                    break;
                case WKING:
                    if (kingAttacks(r, c, row, col)) return 1;
                    break;
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
    move->prevEnPassantRow = pos->enPassantRow;
    move->prevEnPassantCol = pos->enPassantCol;
    move->prevCastlingRights = getCastlingRights(pos);
    move->prevHalfmoveClock = pos->halfmoveClock;
    
    int piece = pos->board[move->fromRow][move->fromCol];
    int captured = pos->board[move->toRow][move->toCol];
    move->piece = piece;
    move->captured = captured;
    
    pos->board[move->fromRow][move->fromCol] = EMPTY;
    
    if (move->type == MOVE_EN_PASSANT) {
        int capturedRow = (pos->sideToMove == WHITE) ? move->toRow + 1 : move->toRow - 1;
        pos->board[capturedRow][move->toCol] = EMPTY;
    } else {
        pos->board[move->toRow][move->toCol] = move->promotion ? move->promotion : piece;
    }
    
    if (move->type == MOVE_CASTLE_KINGSIDE) {
        if (pos->sideToMove == WHITE) {
            pos->board[RANK_1][5] = WROOK;
            pos->board[RANK_1][7] = EMPTY;
        } else {
            pos->board[RANK_8][5] = BROOK;
            pos->board[RANK_8][7] = EMPTY;
        }
    } else if (move->type == MOVE_CASTLE_QUEENSIDE) {
        if (pos->sideToMove == WHITE) {
            pos->board[RANK_1][3] = WROOK;
            pos->board[RANK_1][0] = EMPTY;
        } else {
            pos->board[RANK_8][3] = BROOK;
            pos->board[RANK_8][0] = EMPTY;
        }
    }
    
    if (piece == WKING) {
        pos->whiteKingRow = move->toRow;
        pos->whiteKingCol = move->toCol;
        pos->whiteKingMoved = 1;
    } else if (piece == BKING) {
        pos->blackKingRow = move->toRow;
        pos->blackKingCol = move->toCol;
        pos->blackKingMoved = 1;
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
        } else {
            pos->enPassantRow = -1;
            pos->enPassantCol = -1;
        }
    } else {
        pos->halfmoveClock++;
        pos->enPassantRow = -1;
        pos->enPassantCol = -1;
    }
    
    if (captured != EMPTY) {
        pos->halfmoveClock = 0;
    }
    
    pos->sideToMove = -pos->sideToMove;
    if (pos->sideToMove == WHITE) pos->fullmoveNumber++;
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
            pos->board[RANK_1][7] = WROOK;
            pos->board[RANK_1][5] = EMPTY;
        } else {
            pos->board[RANK_8][7] = BROOK;
            pos->board[RANK_8][5] = EMPTY;
        }
    } else if (move->type == MOVE_CASTLE_QUEENSIDE) {
        if (pos->sideToMove == BLACK) {
            pos->board[RANK_1][0] = WROOK;
            pos->board[RANK_1][3] = EMPTY;
        } else {
            pos->board[RANK_8][0] = BROOK;
            pos->board[RANK_8][3] = EMPTY;
        }
    }
    
    if (piece == WKING) {
        pos->whiteKingRow = move->fromRow;
        pos->whiteKingCol = move->fromCol;
    } else if (piece == BKING) {
        pos->blackKingRow = move->fromRow;
        pos->blackKingCol = move->fromCol;
    }
    
    setCastlingRights(pos, move->prevCastlingRights);
    pos->enPassantRow = move->prevEnPassantRow;
    pos->enPassantCol = move->prevEnPassantCol;
    pos->halfmoveClock = move->prevHalfmoveClock;
    pos->sideToMove = -pos->sideToMove;
    if (pos->sideToMove == BLACK) pos->fullmoveNumber--;
}