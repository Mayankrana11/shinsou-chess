#include "movegen.h"
#include "attacks.h"
#include "../utils/constants.h"
#include "../board/board.h"

static int isWhitePiece(int p) { return p > 0; }
static int isBlackPiece(int p) { return p < 0; }
static int sameColor(int a, int b) { return (a > 0 && b > 0) || (a < 0 && b < 0); }
static int inBounds(int r, int c) { return r >= 0 && r < 8 && c >= 0 && c < 8; }

void addMove(Move moves[], int* count, int fr, int fc, int tr, int tc, int piece, int captured, int promotion, int type) {
    moves[*count].fromRow = fr;
    moves[*count].fromCol = fc;
    moves[*count].toRow = tr;
    moves[*count].toCol = tc;
    moves[*count].piece = piece;
    moves[*count].captured = captured;
    moves[*count].promotion = promotion;
    moves[*count].type = type;
    moves[*count].prevEnPassantRow = -1;
    moves[*count].prevEnPassantCol = -1;
    moves[*count].prevCastlingRights = 0;
    moves[*count].prevHalfmoveClock = 0;
    (*count)++;
}

int generatePseudoLegalMoves(Position* pos, Move moves[]) {
    int count = 0;
    int side = pos->sideToMove;
    int pawnDir = (side == WHITE) ? -1 : 1;
    int pawnStartRow = (side == WHITE) ? 6 : 1;
    int promotionRow = (side == WHITE) ? 0 : 7;
    int kingRow = (side == WHITE) ? pos->whiteKingRow : pos->blackKingRow;
    int kingCol = (side == WHITE) ? pos->whiteKingCol : pos->blackKingCol;
    int rights = getCastlingRights(pos);

    for (int r = 0; r < 8; r++) {
        for (int c = 0; c < 8; c++) {
            int piece = pos->board[r][c];
            if (piece == EMPTY) continue;
            if (side == WHITE && piece < 0) continue;
            if (side == BLACK && piece > 0) continue;

            int type = piece > 0 ? piece : -piece;

            if (type == WPAWN) {
                int nr = r + pawnDir;
                if (inBounds(nr, c)) {
                    if (pos->board[nr][c] == EMPTY) {
                        if (nr == promotionRow) {
                            addMove(moves, &count, r, c, nr, c, piece, EMPTY, WQUEEN, MOVE_PROMOTION);
                            addMove(moves, &count, r, c, nr, c, piece, EMPTY, WROOK, MOVE_PROMOTION);
                            addMove(moves, &count, r, c, nr, c, piece, EMPTY, WBISHOP, MOVE_PROMOTION);
                            addMove(moves, &count, r, c, nr, c, piece, EMPTY, WKNIGHT, MOVE_PROMOTION);
                        } else {
                            addMove(moves, &count, r, c, nr, c, piece, EMPTY, 0, MOVE_NORMAL);
                            if (r == pawnStartRow && pos->board[r + 2 * pawnDir][c] == EMPTY) {
                                addMove(moves, &count, r, c, r + 2 * pawnDir, c, piece, EMPTY, 0, MOVE_NORMAL);
                            }
                        }
                    }
                    for (int dc = -1; dc <= 1; dc += 2) {
                        int nc = c + dc;
                        if (inBounds(nr, nc)) {
                            int target = pos->board[nr][nc];
                            if (target != EMPTY && !sameColor(piece, target)) {
                                if (nr == promotionRow) {
                                    addMove(moves, &count, r, c, nr, nc, piece, target, WQUEEN, MOVE_PROMOTION);
                                    addMove(moves, &count, r, c, nr, nc, piece, target, WROOK, MOVE_PROMOTION);
                                    addMove(moves, &count, r, c, nr, nc, piece, target, WBISHOP, MOVE_PROMOTION);
                                    addMove(moves, &count, r, c, nr, nc, piece, target, WKNIGHT, MOVE_PROMOTION);
                                } else {
                                    addMove(moves, &count, r, c, nr, nc, piece, target, 0, MOVE_NORMAL);
                                }
                            } else if (nr == pos->enPassantRow && nc == pos->enPassantCol) {
                                addMove(moves, &count, r, c, nr, nc, piece, (side == WHITE) ? BPAWN : WPAWN, 0, MOVE_EN_PASSANT);
                            }
                        }
                    }
                }
            }

            if (type == WKNIGHT) {
                static const int dirs[8][2] = {{2,1},{2,-1},{-2,1},{-2,-1},{1,2},{1,-2},{-1,2},{-1,-2}};
                for (int i = 0; i < 8; i++) {
                    int nr = r + dirs[i][0];
                    int nc = c + dirs[i][1];
                    if (!inBounds(nr, nc)) continue;
                    int target = pos->board[nr][nc];
                    if (target == EMPTY || !sameColor(piece, target)) {
                        addMove(moves, &count, r, c, nr, nc, piece, target, 0, MOVE_NORMAL);
                    }
                }
            }

            static const int bishopDirs[4][2] = {{1,1},{1,-1},{-1,1},{-1,-1}};
            static const int rookDirs[4][2] = {{1,0},{-1,0},{0,1},{0,-1}};
            static const int queenDirs[8][2] = {{1,1},{1,-1},{-1,1},{-1,-1},{1,0},{-1,0},{0,1},{0,-1}};

            if (type == WBISHOP || type == WROOK || type == WQUEEN) {
                const int (*dirs)[2] = (type == WBISHOP) ? bishopDirs : (type == WROOK) ? rookDirs : queenDirs;
                int dirCount = (type == WQUEEN) ? 8 : 4;
                for (int d = 0; d < dirCount; d++) {
                    int dr = dirs[d][0], dc = dirs[d][1];
                    int nr = r + dr, nc = c + dc;
                    while (inBounds(nr, nc)) {
                        int target = pos->board[nr][nc];
                        if (target == EMPTY) {
                            addMove(moves, &count, r, c, nr, nc, piece, EMPTY, 0, MOVE_NORMAL);
                        } else {
                            if (!sameColor(piece, target)) {
                                addMove(moves, &count, r, c, nr, nc, piece, target, 0, MOVE_NORMAL);
                            }
                            break;
                        }
                        nr += dr; nc += dc;
                    }
                }
            }

            if (type == WKING) {
                for (int dr = -1; dr <= 1; dr++) {
                    for (int dc = -1; dc <= 1; dc++) {
                        if (dr == 0 && dc == 0) continue;
                        int nr = r + dr, nc = c + dc;
                        if (!inBounds(nr, nc)) continue;
                        int target = pos->board[nr][nc];
                        if (target == EMPTY || !sameColor(piece, target)) {
                            addMove(moves, &count, r, c, nr, nc, piece, target, 0, MOVE_NORMAL);
                        }
                    }
                }

                if (side == WHITE && kingRow == RANK_1 && kingCol == 4) {
                    if ((rights & CASTLE_WHITE_KINGSIDE) && pos->board[RANK_1][5] == EMPTY && pos->board[RANK_1][6] == EMPTY) {
                        addMove(moves, &count, RANK_1, 4, RANK_1, 6, WKING, EMPTY, 0, MOVE_CASTLE_KINGSIDE);
                    }
                    if ((rights & CASTLE_WHITE_QUEENSIDE) && pos->board[RANK_1][3] == EMPTY && pos->board[RANK_1][2] == EMPTY && pos->board[RANK_1][1] == EMPTY) {
                        addMove(moves, &count, RANK_1, 4, RANK_1, 2, WKING, EMPTY, 0, MOVE_CASTLE_QUEENSIDE);
                    }
                } else if (side == BLACK && kingRow == RANK_8 && kingCol == 4) {
                    if ((rights & CASTLE_BLACK_KINGSIDE) && pos->board[RANK_8][5] == EMPTY && pos->board[RANK_8][6] == EMPTY) {
                        addMove(moves, &count, RANK_8, 4, RANK_8, 6, BKING, EMPTY, 0, MOVE_CASTLE_KINGSIDE);
                    }
                    if ((rights & CASTLE_BLACK_QUEENSIDE) && pos->board[RANK_8][3] == EMPTY && pos->board[RANK_8][2] == EMPTY && pos->board[RANK_8][1] == EMPTY) {
                        addMove(moves, &count, RANK_8, 4, RANK_8, 2, BKING, EMPTY, 0, MOVE_CASTLE_QUEENSIDE);
                    }
                }
            }
        }
    }
    return count;
}

int generateLegalMoves(Position* pos, Move moves[]) {
    Move pseudoMoves[MAX_MOVES];
    int pseudoCount = generatePseudoLegalMoves(pos, pseudoMoves);
    int legalCount = 0;

    for (int i = 0; i < pseudoCount; i++) {
        makeMove(pos, &pseudoMoves[i]);
        if (!isInCheck(pos, pos->sideToMove == WHITE ? BLACK : WHITE)) {
            moves[legalCount] = pseudoMoves[i];
            legalCount++;
        }
        undoMove(pos, &pseudoMoves[i]);
    }
    return legalCount;
}