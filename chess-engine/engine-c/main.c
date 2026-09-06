#include <stdio.h>

#include "board/board.h"
#include "movegen/movegen.h"
#include "utils/constants.h"

void testInitialPosition() {
    Position pos;
    initBoard(&pos);
    printBoard(&pos);

    Move moves[MAX_MOVES];
    int pseudoCount = generatePseudoLegalMoves(&pos, moves);
    int legalCount = generateLegalMoves(&pos, moves);

    printf("Pseudo-legal moves: %d\n", pseudoCount);
    printf("Legal moves: %d\n\n", legalCount);

    for (int i = 0; i < legalCount; i++) {
        printf("(%d,%d) -> (%d,%d) ",
            moves[i].fromRow, moves[i].fromCol,
            moves[i].toRow, moves[i].toCol);
        if (moves[i].type == MOVE_PROMOTION) printf("(promo)");
        if (moves[i].type == MOVE_CASTLE_KINGSIDE) printf("(O-O)");
        if (moves[i].type == MOVE_CASTLE_QUEENSIDE) printf("(O-O-O)");
        if (moves[i].type == MOVE_EN_PASSANT) printf("(e.p.)");
        printf("\n");
    }

    printf("\nTesting make/undo...\n");
    Position original;
    copyPosition(&original, &pos);

    for (int i = 0; i < legalCount; i++) {
        makeMove(&pos, &moves[i]);
        undoMove(&pos, &moves[i]);
    }

    int match = 1;
    for (int r = 0; r < 8; r++) {
        for (int c = 0; c < 8; c++) {
            if (pos.board[r][c] != original.board[r][c]) match = 0;
        }
    }
    if (pos.sideToMove != original.sideToMove) match = 0;
    if (pos.whiteKingMoved != original.whiteKingMoved) match = 0;
    if (pos.blackKingMoved != original.blackKingMoved) match = 0;
    if (pos.whiteLeftRookMoved != original.whiteLeftRookMoved) match = 0;
    if (pos.whiteRightRookMoved != original.whiteRightRookMoved) match = 0;
    if (pos.blackLeftRookMoved != original.blackLeftRookMoved) match = 0;
    if (pos.blackRightRookMoved != original.blackRightRookMoved) match = 0;
    if (pos.enPassantRow != original.enPassantRow) match = 0;
    if (pos.enPassantCol != original.enPassantCol) match = 0;
    if (pos.halfmoveClock != original.halfmoveClock) match = 0;
    if (pos.fullmoveNumber != original.fullmoveNumber) match = 0;
    if (pos.whiteKingRow != original.whiteKingRow) match = 0;
    if (pos.whiteKingCol != original.whiteKingCol) match = 0;
    if (pos.blackKingRow != original.blackKingRow) match = 0;
    if (pos.blackKingCol != original.blackKingCol) match = 0;

    printf("Make/Undo test: %s\n", match ? "PASSED" : "FAILED");
}

void testCheckDetection() {
    printf("\n--- Check Detection Test ---\n");
    Position pos;
    clearBoard(&pos);
    pos.sideToMove = WHITE;
    pos.whiteKingRow = 0; pos.whiteKingCol = 4;
    pos.blackKingRow = 7; pos.blackKingCol = 4;
    pos.board[0][4] = WKING;
    pos.board[7][4] = BKING;
    pos.board[1][4] = BROOK;

    printBoard(&pos);
    printf("White in check: %s\n", isInCheck(&pos, WHITE) ? "YES" : "NO");
    printf("Black in check: %s\n", isInCheck(&pos, BLACK) ? "YES" : "NO");
}

void testCastling() {
    printf("\n--- Castling Test ---\n");
    Position pos;
    clearBoard(&pos);
    pos.sideToMove = WHITE;
    pos.whiteKingRow = 7; pos.whiteKingCol = 4;
    pos.blackKingRow = 0; pos.blackKingCol = 4;
    pos.board[7][4] = WKING;
    pos.board[7][7] = WROOK;
    pos.board[7][0] = WROOK;
    pos.board[0][4] = BKING;
    pos.whiteKingMoved = 0;
    pos.whiteLeftRookMoved = 0;
    pos.whiteRightRookMoved = 0;

    printBoard(&pos);
    Move moves[MAX_MOVES];
    int count = generateLegalMoves(&pos, moves);
    printf("Legal moves: %d\n", count);
    for (int i = 0; i < count; i++) {
        if (moves[i].type == MOVE_CASTLE_KINGSIDE || moves[i].type == MOVE_CASTLE_QUEENSIDE) {
            printf("Castling: (%d,%d) -> (%d,%d)\n",
                moves[i].fromRow, moves[i].fromCol,
                moves[i].toRow, moves[i].toCol);
        }
    }
}

void testEnPassant() {
    printf("\n--- En Passant Test ---\n");
    Position pos;
    clearBoard(&pos);
    pos.sideToMove = WHITE;
    pos.whiteKingRow = 7; pos.whiteKingCol = 4;
    pos.blackKingRow = 0; pos.blackKingCol = 4;
    pos.board[7][4] = WKING;
    pos.board[0][4] = BKING;
    pos.board[4][4] = WPAWN;
    pos.board[4][3] = BPAWN;
    pos.enPassantRow = 3;
    pos.enPassantCol = 3;

    printBoard(&pos);
    Move moves[MAX_MOVES];
    int count = generateLegalMoves(&pos, moves);
    printf("Legal moves: %d\n", count);
    for (int i = 0; i < count; i++) {
        if (moves[i].type == MOVE_EN_PASSANT) {
            printf("En passant: (%d,%d) -> (%d,%d)\n",
                moves[i].fromRow, moves[i].fromCol,
                moves[i].toRow, moves[i].toCol);
        }
    }
}

void testPromotion() {
    printf("\n--- Promotion Test ---\n");
    Position pos;
    clearBoard(&pos);
    pos.sideToMove = WHITE;
    pos.whiteKingRow = 7; pos.whiteKingCol = 4;
    pos.blackKingRow = 0; pos.blackKingCol = 4;
    pos.board[7][4] = WKING;
    pos.board[0][4] = BKING;
    pos.board[1][0] = WPAWN;

    printBoard(&pos);
    Move moves[MAX_MOVES];
    int count = generateLegalMoves(&pos, moves);
    printf("Legal moves: %d\n", count);
    for (int i = 0; i < count; i++) {
        if (moves[i].type == MOVE_PROMOTION) {
            printf("Promotion: (%d,%d) -> (%d,%d) promo=%d\n",
                moves[i].fromRow, moves[i].fromCol,
                moves[i].toRow, moves[i].toCol, moves[i].promotion);
        }
    }
}

void testCastlingThroughCheck() {
    printf("\n--- Castling Through Check Test ---\n");
    Position pos;
    clearBoard(&pos);
    pos.sideToMove = WHITE;
    pos.whiteKingRow = 7; pos.whiteKingCol = 4;
    pos.blackKingRow = 0; pos.blackKingCol = 4;
    pos.board[7][4] = WKING;
    pos.board[7][7] = WROOK;
    pos.board[0][4] = BKING;
    pos.board[4][2] = BBISHOP;
    pos.whiteKingMoved = 0;
    pos.whiteRightRookMoved = 0;

    printBoard(&pos);
    Move moves[MAX_MOVES];
    int count = generateLegalMoves(&pos, moves);
    printf("Legal moves: %d\n", count);
    int hasKingside = 0;
    for (int i = 0; i < count; i++) {
        if (moves[i].type == MOVE_CASTLE_KINGSIDE) hasKingside = 1;
    }
    printf("Kingside castling allowed: %s (expected: NO)\n", hasKingside ? "YES" : "NO");
}

void testCastlingIntoCheck() {
    printf("\n--- Castling Into Check Test ---\n");
    Position pos;
    clearBoard(&pos);
    pos.sideToMove = WHITE;
    pos.whiteKingRow = 7; pos.whiteKingCol = 4;
    pos.blackKingRow = 0; pos.blackKingCol = 4;
    pos.board[7][4] = WKING;
    pos.board[7][7] = WROOK;
    pos.board[0][4] = BKING;
    pos.board[4][6] = BROOK;
    pos.whiteKingMoved = 0;
    pos.whiteRightRookMoved = 0;

    printBoard(&pos);
    Move moves[MAX_MOVES];
    int count = generateLegalMoves(&pos, moves);
    printf("Legal moves: %d\n", count);
    int hasKingside = 0;
    for (int i = 0; i < count; i++) {
        if (moves[i].type == MOVE_CASTLE_KINGSIDE) hasKingside = 1;
    }
    printf("Kingside castling allowed: %s (expected: NO)\n", hasKingside ? "YES" : "NO");
}

void testCastlingKingInCheck() {
    printf("\n--- Castling King In Check Test ---\n");
    Position pos;
    clearBoard(&pos);
    pos.sideToMove = WHITE;
    pos.whiteKingRow = 7; pos.whiteKingCol = 4;
    pos.blackKingRow = 0; pos.blackKingCol = 4;
    pos.board[7][4] = WKING;
    pos.board[7][7] = WROOK;
    pos.board[0][4] = BKING;
    pos.board[7][1] = BROOK;
    pos.whiteKingMoved = 0;
    pos.whiteRightRookMoved = 0;

    printBoard(&pos);
    Move moves[MAX_MOVES];
    int count = generateLegalMoves(&pos, moves);
    printf("Legal moves: %d\n", count);
    int hasKingside = 0, hasQueenside = 0;
    for (int i = 0; i < count; i++) {
        if (moves[i].type == MOVE_CASTLE_KINGSIDE) hasKingside = 1;
        if (moves[i].type == MOVE_CASTLE_QUEENSIDE) hasQueenside = 1;
    }
    printf("Kingside castling allowed: %s (expected: NO)\n", hasKingside ? "YES" : "NO");
    printf("Queenside castling allowed: %s (expected: NO)\n", hasQueenside ? "YES" : "NO");
}

void testEnPassantDiscoveredCheck() {
    printf("\n--- En Passant Discovered Check Test ---\n");
    Position pos;
    clearBoard(&pos);
    pos.sideToMove = WHITE;
    pos.whiteKingRow = 7; pos.whiteKingCol = 4;
    pos.blackKingRow = 0; pos.blackKingCol = 4;
    pos.board[7][4] = WKING;
    pos.board[0][4] = BKING;
    pos.board[4][4] = WPAWN;
    pos.board[4][3] = BPAWN;
    pos.board[3][4] = BROOK;
    pos.enPassantRow = 3;
    pos.enPassantCol = 3;

    printBoard(&pos);
    Move moves[MAX_MOVES];
    int count = generateLegalMoves(&pos, moves);
    printf("Legal moves: %d\n", count);
    int hasEnPassant = 0;
    for (int i = 0; i < count; i++) {
        if (moves[i].type == MOVE_EN_PASSANT) hasEnPassant = 1;
    }
    printf("En passant allowed: %s (expected: NO - discovered check)\n", hasEnPassant ? "YES" : "NO");
}

void testEnPassantLegal() {
    printf("\n--- En Passant Legal Test ---\n");
    Position pos;
    clearBoard(&pos);
    pos.sideToMove = WHITE;
    pos.whiteKingRow = 7; pos.whiteKingCol = 4;
    pos.blackKingRow = 0; pos.blackKingCol = 4;
    pos.board[7][4] = WKING;
    pos.board[0][4] = BKING;
    pos.board[4][4] = WPAWN;
    pos.board[4][3] = BPAWN;
    pos.enPassantRow = 3;
    pos.enPassantCol = 3;

    printBoard(&pos);
    Move moves[MAX_MOVES];
    int count = generateLegalMoves(&pos, moves);
    printf("Legal moves: %d\n", count);
    int hasEnPassant = 0;
    for (int i = 0; i < count; i++) {
        if (moves[i].type == MOVE_EN_PASSANT) hasEnPassant = 1;
    }
    printf("En passant allowed: %s (expected: YES)\n", hasEnPassant ? "YES" : "NO");
}

void testPromotionMakeUndo() {
    printf("\n--- Promotion Make/Undo Test ---\n");
    Position pos;
    clearBoard(&pos);
    pos.sideToMove = WHITE;
    pos.whiteKingRow = 7; pos.whiteKingCol = 4;
    pos.blackKingRow = 0; pos.blackKingCol = 4;
    pos.board[7][4] = WKING;
    pos.board[0][4] = BKING;
    pos.board[1][0] = WPAWN;

    Move moves[MAX_MOVES];
    int count = generateLegalMoves(&pos, moves);
    
    int promoIdx = -1;
    for (int i = 0; i < count; i++) {
        if (moves[i].type == MOVE_PROMOTION && moves[i].promotion == WQUEEN) {
            promoIdx = i;
            break;
        }
    }
    
    if (promoIdx >= 0) {
        Position original;
        copyPosition(&original, &pos);
        makeMove(&pos, &moves[promoIdx]);
        printf("After promotion: piece at (0,0) = %d (expected: %d)\n", pos.board[0][0], WQUEEN);
        undoMove(&pos, &moves[promoIdx]);
        
        int match = 1;
        for (int r = 0; r < 8; r++) {
            for (int c = 0; c < 8; c++) {
                if (pos.board[r][c] != original.board[r][c]) match = 0;
            }
        }
        if (pos.sideToMove != original.sideToMove) match = 0;
        if (pos.halfmoveClock != original.halfmoveClock) match = 0;
        if (pos.fullmoveNumber != original.fullmoveNumber) match = 0;
        printf("Promotion make/undo: %s\n", match ? "PASSED" : "FAILED");
    } else {
        printf("No queen promotion found\n");
    }
}

void testPinnedPiece() {
    printf("\n--- Pinned Piece Test ---\n");
    Position pos;
    clearBoard(&pos);
    pos.sideToMove = WHITE;
    pos.whiteKingRow = 7; pos.whiteKingCol = 4;
    pos.blackKingRow = 0; pos.blackKingCol = 4;
    pos.board[7][4] = WKING;
    pos.board[0][4] = BKING;
    pos.board[6][4] = WPAWN;
    pos.board[4][4] = BROOK;

    printBoard(&pos);
    Move moves[MAX_MOVES];
    int count = generateLegalMoves(&pos, moves);
    printf("Legal moves: %d\n", count);
    int pawnCanMove = 0;
    int pawnCanCapture = 0;
    for (int i = 0; i < count; i++) {
        if (moves[i].fromRow == 6 && moves[i].fromCol == 4) {
            if (moves[i].toRow == 5 && moves[i].toCol == 4) pawnCanMove = 1;
            if (moves[i].toRow == 5 && (moves[i].toCol == 3 || moves[i].toCol == 5)) pawnCanCapture = 1;
        }
    }
    printf("Pawn can move forward: %s (expected: YES - moving along pin line)\n", pawnCanMove ? "YES" : "NO");
    printf("Pawn can capture diagonally: %s (expected: NO - would expose king)\n", pawnCanCapture ? "YES" : "NO");
}

void testDiscoveredCheck() {
    printf("\n--- Discovered Check Test ---\n");
    Position pos;
    clearBoard(&pos);
    pos.sideToMove = WHITE;
    pos.whiteKingRow = 7; pos.whiteKingCol = 4;
    pos.blackKingRow = 0; pos.blackKingCol = 4;
    pos.board[7][4] = WKING;
    pos.board[0][4] = BKING;
    pos.board[6][4] = WROOK;
    pos.board[1][4] = BPAWN;

    printBoard(&pos);
    Move moves[MAX_MOVES];
    int count = generateLegalMoves(&pos, moves);
    printf("Legal moves: %d\n", count);
    int rookMoves = 0;
    for (int i = 0; i < count; i++) {
        if (moves[i].fromRow == 6 && moves[i].fromCol == 4) {
            rookMoves++;
        }
    }
    printf("Rook legal moves: %d (expected: > 0, moving gives discovered check)\n", rookMoves);
    int givesCheck = 0;
    for (int i = 0; i < count; i++) {
        if (moves[i].fromRow == 6 && moves[i].fromCol == 4) {
            makeMove(&pos, &moves[i]);
            if (isInCheck(&pos, BLACK)) {
                givesCheck = 1;
                printf("Move to (%d,%d) gives discovered check\n", moves[i].toRow, moves[i].toCol);
            }
            undoMove(&pos, &moves[i]);
        }
    }
    printf("At least one move gives discovered check: %s\n", givesCheck ? "YES" : "NO");
}

void testDoubleCheck() {
    printf("\n--- Double Check Test ---\n");
    Position pos;
    clearBoard(&pos);
    pos.sideToMove = BLACK;
    pos.whiteKingRow = 0; pos.whiteKingCol = 4;
    pos.blackKingRow = 7; pos.blackKingCol = 4;
    pos.board[0][4] = WKING;
    pos.board[7][4] = BKING;
    pos.board[2][4] = WROOK;
    pos.board[1][3] = WBISHOP;

    printBoard(&pos);
    printf("Black in check: %s (expected: YES - double check)\n", isInCheck(&pos, BLACK) ? "YES" : "NO");
    Move moves[MAX_MOVES];
    int count = generateLegalMoves(&pos, moves);
    printf("Black legal moves: %d\n", count);
    int kingMoves = 0;
    for (int i = 0; i < count; i++) {
        if (moves[i].piece == BKING) kingMoves++;
    }
    printf("King moves: %d (expected: only king moves, cannot block double check)\n", kingMoves);
    int canBlock = 0;
    for (int i = 0; i < count; i++) {
        if (moves[i].piece != BKING) {
            canBlock = 1;
        }
    }
    printf("Can block/capture: %s (expected: NO - double check)\n", canBlock ? "YES" : "NO");
}

void testPinnedKnight() {
    printf("\n--- Pinned Knight Test ---\n");
    Position pos;
    clearBoard(&pos);
    pos.sideToMove = WHITE;
    pos.whiteKingRow = 7; pos.whiteKingCol = 4;
    pos.blackKingRow = 0; pos.blackKingCol = 4;
    pos.board[7][4] = WKING;
    pos.board[0][4] = BKING;
    pos.board[5][4] = WKNIGHT;
    pos.board[3][4] = BROOK;

    printBoard(&pos);
    Move moves[MAX_MOVES];
    int count = generateLegalMoves(&pos, moves);
    printf("Legal moves: %d\n", count);
    int knightMoves = 0;
    for (int i = 0; i < count; i++) {
        if (moves[i].fromRow == 5 && moves[i].fromCol == 4) {
            knightMoves++;
        }
    }
    printf("Knight legal moves: %d (expected: 0 - pinned to king)\n", knightMoves);
}

void testPinnedBishopDiagonal() {
    printf("\n--- Pinned Bishop Diagonal Test ---\n");
    Position pos;
    clearBoard(&pos);
    pos.sideToMove = WHITE;
    pos.whiteKingRow = 7; pos.whiteKingCol = 4;
    pos.blackKingRow = 0; pos.blackKingCol = 4;
    pos.board[7][4] = WKING;
    pos.board[0][4] = BKING;
    pos.board[6][3] = WBISHOP;
    pos.board[4][5] = BBISHOP;

    printBoard(&pos);
    Move moves[MAX_MOVES];
    int count = generateLegalMoves(&pos, moves);
    printf("Legal moves: %d\n", count);
    int bishopMoves = 0;
    for (int i = 0; i < count; i++) {
        if (moves[i].fromRow == 6 && moves[i].fromCol == 3) {
            bishopMoves++;
        }
    }
    printf("Bishop legal moves: %d (expected: can move along diagonal pin line)\n", bishopMoves);
    for (int i = 0; i < count; i++) {
        if (moves[i].fromRow == 6 && moves[i].fromCol == 3) {
            printf("  -> (%d,%d)\n", moves[i].toRow, moves[i].toCol);
        }
    }
}

int main() {
    testInitialPosition();
    testCheckDetection();
    testCastling();
    testEnPassant();
    testPromotion();
    testCastlingThroughCheck();
    testCastlingIntoCheck();
    testCastlingKingInCheck();
    testEnPassantDiscoveredCheck();
    testEnPassantLegal();
    testPromotionMakeUndo();
    testPinnedPiece();
    testDiscoveredCheck();
    testDoubleCheck();
    testPinnedKnight();
    testPinnedBishopDiagonal();
    return 0;
}