#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "fen.h"
#include "../utils/constants.h"

static int charToPiece(char c) {
    switch (c) {
        case 'P': return WPAWN;
        case 'N': return WKNIGHT;
        case 'B': return WBISHOP;
        case 'R': return WROOK;
        case 'Q': return WQUEEN;
        case 'K': return WKING;
        case 'p': return BPAWN;
        case 'n': return BKNIGHT;
        case 'b': return BBISHOP;
        case 'r': return BROOK;
        case 'q': return BQUEEN;
        case 'k': return BKING;
        default: return EMPTY;
    }
}

static char pieceToChar(int p) {
    switch (p) {
        case WPAWN:   return 'P';
        case WKNIGHT: return 'N';
        case WBISHOP: return 'B';
        case WROOK:   return 'R';
        case WQUEEN:  return 'Q';
        case WKING:   return 'K';
        case BPAWN:   return 'p';
        case BKNIGHT: return 'n';
        case BBISHOP: return 'b';
        case BROOK:   return 'r';
        case BQUEEN:  return 'q';
        case BKING:   return 'k';
        default:      return '.';
    }
}

int parseFEN(Position* pos, const char* fen) {
    clearBoard(pos);

    int rank = 0;
    int file = 0;
    const char* p = fen;

    // 1. Piece Placement
    while (*p && *p != ' ') {
        if (*p == '/') {
            rank++;
            file = 0;
        } else if (isdigit(*p)) {
            file += (*p - '0');
        } else {
            if (rank < 8 && file < 8) {
                pos->board[rank][file] = charToPiece(*p);
            }
            file++;
        }
        p++;
    }
    if (rank != 7) return 0;
    p++; // skip space

    // 2. Side to move
    if (*p == 'w') pos->sideToMove = WHITE;
    else if (*p == 'b') pos->sideToMove = BLACK;
    else return 0;
    p++; // skip char
    p++; // skip space

    // 3. Castling Rights
    int rights = 0;
    while (*p && *p != ' ') {
        if (*p == 'K') rights |= CASTLE_WHITE_KINGSIDE;
        else if (*p == 'Q') rights |= CASTLE_WHITE_QUEENSIDE;
        else if (*p == 'k') rights |= CASTLE_BLACK_KINGSIDE;
        else if (*p == 'q') rights |= CASTLE_BLACK_QUEENSIDE;
        p++;
    }
    setCastlingRights(pos, rights);
    p++; // skip space

    // 4. En Passant Target
    if (*p == '-') {
        pos->enPassantRow = -1;
        pos->enPassantCol = -1;
    } else {
        pos->enPassantCol = *p - 'a';
        pos->enPassantRow = 8 - (*(p + 1) - '0');
    }
    p += 2;
    p++; // skip space

    // 5. Halfmove Clock
    pos->halfmoveClock = 0;
    while (isdigit(*p)) {
        pos->halfmoveClock = pos->halfmoveClock * 10 + (*p - '0');
        p++;
    }
    p++; // skip space

    // 6. Fullmove Number
    pos->fullmoveNumber = 0;
    while (isdigit(*p)) {
        pos->fullmoveNumber = pos->fullmoveNumber * 10 + (*p - '0');
        p++;
    }

    // Update king positions
    findKing(pos, WHITE);
    findKing(pos, BLACK);

    return 1;
}

void generateFEN(const Position* pos, char* buffer) {
    char piecePart[64] = {0};
    int pIdx = 0;

    for (int r = 0; r < 8; r++) {
        int emptyCount = 0;
        for (int c = 0; c < 8; c++) {
            if (pos->board[r][c] == EMPTY) {
                emptyCount++;
            } else {
                if (emptyCount > 0) {
                    piecePart[pIdx++] = emptyCount + '0';
                    emptyCount = 0;
                }
                piecePart[pIdx++] = pieceToChar(pos->board[r][c]);
            }
        }
        if (emptyCount > 0) {
            piecePart[pIdx++] = emptyCount + '0';
        }
        if (r < 7) {
            piecePart[pIdx++] = '/';
        }
    }
    piecePart[pIdx] = '\0';

    char side = (pos->sideToMove == WHITE) ? 'w' : 'b';

    // Castling
    char castling[5] = {0};
    int cIdx = 0;
    int rights = getCastlingRights((Position*)pos);
    if (rights & CASTLE_WHITE_KINGSIDE) castling[cIdx++] = 'K';
    if (rights & CASTLE_WHITE_QUEENSIDE) castling[cIdx++] = 'Q';
    if (rights & CASTLE_BLACK_KINGSIDE) castling[cIdx++] = 'k';
    if (rights & CASTLE_BLACK_QUEENSIDE) castling[cIdx++] = 'q';
    if (cIdx == 0) castling[cIdx++] = '-';
    castling[cIdx] = '\0';

    // En Passant
    char ep[3] = {0};
    if (pos->enPassantRow == -1) {
        strcpy(ep, "-");
    } else {
        ep[0] = pos->enPassantCol + 'a';
        ep[1] = '0' + (8 - pos->enPassantRow);
    }

    sprintf(buffer, "%s %c %s %s %d %d",
            piecePart, side, castling, ep, pos->halfmoveClock, pos->fullmoveNumber);
}
