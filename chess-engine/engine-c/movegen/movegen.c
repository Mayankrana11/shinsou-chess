#include "movegen.h"
#include "attacks.h"
#include "../utils/constants.h"
#include "../utils/bitboard.h"
#include "../board/board.h"

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
    moves[*count].score = 0;
    (*count)++;
}

int generatePseudoLegalMoves(Position* pos, Move moves[]) {
    int count = 0;
    int side = pos->sideToMove;
    uint64_t myPieces = (side == WHITE) ? pos->whitePieces : pos->blackPieces;
    uint64_t allPieces = pos->allPieces;

    /* 1. Pawn Moves */
    uint64_t pawns = pos->pawns & myPieces;
    while (pawns) {
        int sq = lsb(pawns);
        pawns &= ~(1ULL << sq);
        int r = index_to_row(sq);
        int c = index_to_col(sq);
        int piece = pos->board[r][c];

        int pawnDir = (side == WHITE) ? -1 : 1;
        int pawnStartRow = (side == WHITE) ? 6 : 1;
        int promotionRow = (side == WHITE) ? 0 : 7;

        /* Advance */
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
                    if (r == pawnStartRow && inBounds(r + 2 * pawnDir, c) && pos->board[r + 2 * pawnDir][c] == EMPTY) {
                        addMove(moves, &count, r, c, r + 2 * pawnDir, c, piece, EMPTY, 0, MOVE_NORMAL);
                    }
                }
            }
        }

        /* Captures */
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

    /* 2. Knight Moves */
    uint64_t knights = pos->knights & myPieces;
    while (knights) {
        int sq = lsb(knights);
        knights &= ~(1ULL << sq);
        uint64_t attacks = knight_attacks[sq];
        uint64_t legalAttacks = attacks & ~myPieces;
        while (legalAttacks) {
            int targetSq = lsb(legalAttacks);
            legalAttacks &= ~(1ULL << targetSq);
            int fr = index_to_row(sq), fc = index_to_col(sq);
            int tr = index_to_row(targetSq), tc = index_to_col(targetSq);
            addMove(moves, &count, fr, fc, tr, tc, pos->board[fr][fc], pos->board[tr][tc], 0, MOVE_NORMAL);
        }
    }

    /* 3. Bishop Moves */
    uint64_t bishops = pos->bishops & myPieces;
    while (bishops) {
        int sq = lsb(bishops);
        bishops &= ~(1ULL << sq);
        uint64_t attacks = get_bishop_attacks(sq, allPieces);
        uint64_t legalAttacks = attacks & ~myPieces;
        while (legalAttacks) {
            int targetSq = lsb(legalAttacks);
            legalAttacks &= ~(1ULL << targetSq);
            int fr = index_to_row(sq), fc = index_to_col(sq);
            int tr = index_to_row(targetSq), tc = index_to_col(targetSq);
            addMove(moves, &count, fr, fc, tr, tc, pos->board[fr][fc], pos->board[tr][tc], 0, MOVE_NORMAL);
        }
    }

    /* 4. Rook Moves */
    uint64_t rooks = pos->rooks & myPieces;
    while (rooks) {
        int sq = lsb(rooks);
        rooks &= ~(1ULL << sq);
        uint64_t attacks = get_rook_attacks(sq, allPieces);
        uint64_t legalAttacks = attacks & ~myPieces;
        while (legalAttacks) {
            int targetSq = lsb(legalAttacks);
            legalAttacks &= ~(1ULL << targetSq);
            int fr = index_to_row(sq), fc = index_to_col(sq);
            int tr = index_to_row(targetSq), tc = index_to_col(targetSq);
            addMove(moves, &count, fr, fc, tr, tc, pos->board[fr][fc], pos->board[tr][tc], 0, MOVE_NORMAL);
        }
    }

    /* 5. Queen Moves */
    uint64_t queens = pos->queens & myPieces;
    while (queens) {
        int sq = lsb(queens);
        queens &= ~(1ULL << sq);
        uint64_t attacks = get_rook_attacks(sq, allPieces) | get_bishop_attacks(sq, allPieces);
        uint64_t legalAttacks = attacks & ~myPieces;
        while (legalAttacks) {
            int targetSq = lsb(legalAttacks);
            legalAttacks &= ~(1ULL << targetSq);
            int fr = index_to_row(sq), fc = index_to_col(sq);
            int tr = index_to_row(targetSq), tc = index_to_col(targetSq);
            addMove(moves, &count, fr, fc, tr, tc, pos->board[fr][fc], pos->board[tr][tc], 0, MOVE_NORMAL);
        }
    }

    /* 6. King Moves */
    uint64_t kings = pos->kings & myPieces;
    while (kings) {
        int sq = lsb(kings);
        kings &= ~(1ULL << sq);
        uint64_t attacks = king_attacks[sq];
        uint64_t legalAttacks = attacks & ~myPieces;
        while (legalAttacks) {
            int targetSq = lsb(legalAttacks);
            legalAttacks &= ~(1ULL << targetSq);
            int fr = index_to_row(sq), fc = index_to_col(sq);
            int tr = index_to_row(targetSq), tc = index_to_col(targetSq);
            addMove(moves, &count, fr, fc, tr, tc, pos->board[fr][fc], pos->board[tr][tc], 0, MOVE_NORMAL);
        }
    }

    /* Castling */
    int rights = getCastlingRights(pos);
    int kingRow = (side == WHITE) ? pos->whiteKingRow : pos->blackKingRow;
    int kingCol = (side == WHITE) ? pos->whiteKingCol : pos->blackKingCol;
    int enemyColor = (side == WHITE) ? BLACK : WHITE;

    if (!isInCheck(pos, side)) {
        if (side == WHITE && kingRow == RANK_1 && kingCol == 4) {
            if ((rights & CASTLE_WHITE_KINGSIDE) && pos->board[RANK_1][5] == EMPTY && pos->board[RANK_1][6] == EMPTY) {
                if (!isSquareAttacked(pos, RANK_1, 5, enemyColor) && !isSquareAttacked(pos, RANK_1, 6, enemyColor)) {
                    addMove(moves, &count, RANK_1, 4, RANK_1, 6, WKING, EMPTY, 0, MOVE_CASTLE_KINGSIDE);
                }
            }
            if ((rights & CASTLE_WHITE_QUEENSIDE) && pos->board[RANK_1][3] == EMPTY && pos->board[RANK_1][2] == EMPTY && pos->board[RANK_1][1] == EMPTY) {
                if (!isSquareAttacked(pos, RANK_1, 3, enemyColor) && !isSquareAttacked(pos, RANK_1, 2, enemyColor)) {
                    addMove(moves, &count, RANK_1, 4, RANK_1, 2, WKING, EMPTY, 0, MOVE_CASTLE_QUEENSIDE);
                }
            }
        } else if (side == BLACK && kingRow == RANK_8 && kingCol == 4) {
            if ((rights & CASTLE_BLACK_KINGSIDE) && pos->board[RANK_8][5] == EMPTY && pos->board[RANK_8][6] == EMPTY) {
                if (!isSquareAttacked(pos, RANK_8, 5, enemyColor) && !isSquareAttacked(pos, RANK_8, 6, enemyColor)) {
                    addMove(moves, &count, RANK_8, 4, RANK_8, 6, BKING, EMPTY, 0, MOVE_CASTLE_KINGSIDE);
                }
            }
            if ((rights & CASTLE_BLACK_QUEENSIDE) && pos->board[RANK_8][3] == EMPTY && pos->board[RANK_8][2] == EMPTY && pos->board[RANK_8][1] == EMPTY) {
                if (!isSquareAttacked(pos, RANK_8, 3, enemyColor) && !isSquareAttacked(pos, RANK_8, 2, enemyColor)) {
                    addMove(moves, &count, RANK_8, 4, RANK_8, 2, BKING, EMPTY, 0, MOVE_CASTLE_QUEENSIDE);
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

int generateTacticalMoves(Position* pos, Move moves[]) {
    Move pseudoMoves[MAX_MOVES];
    int pseudoCount = generatePseudoLegalMoves(pos, pseudoMoves);
    int tacticalCount = 0;

    for (int i = 0; i < pseudoCount; i++) {
        Move* m = &pseudoMoves[i];
        if (m->captured != EMPTY || m->type == MOVE_PROMOTION) {
            makeMove(pos, m);
            if (!isInCheck(pos, pos->sideToMove == WHITE ? BLACK : WHITE)) {
                moves[tacticalCount++] = *m;
            }
            undoMove(pos, m);
        }
    }
    return tacticalCount;
});
            if (!isInCheck(pos, pos->sideToMove == WHITE ? BLACK : WHITE)) {
                moves[tacticalCount++] = *m;
            }
            undoMove(pos, m);
        }
    }
    return tacticalCount;
}
);
            if (!isInCheck(pos, pos->sideToMove == WHITE ? BLACK : WHITE)) {
                moves[tacticalCount++] = *m;
            }
            undoMove(pos, m);
        }
    }
    return tacticalCount;
}
