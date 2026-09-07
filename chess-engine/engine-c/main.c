#include <stdio.h>

#include "board/board.h"
#include "movegen/movegen.h"
#include "engine/search.h"
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

void testCheckmate() {
    printf("\n--- Checkmate Test ---\n");
    Position pos;
    clearBoard(&pos);
    pos.sideToMove = BLACK;
    pos.whiteKingRow = 1; pos.whiteKingCol = 2;
    pos.blackKingRow = 0; pos.blackKingCol = 0;
    pos.board[1][2] = WKING;
    pos.board[0][0] = BKING;
    pos.board[2][0] = WQUEEN;

    printBoard(&pos);
    printf("Black in check: %s\n", isInCheck(&pos, BLACK) ? "YES" : "NO");
    printf("Is checkmate: %s (expected: YES)\n", isCheckmate(&pos) ? "YES" : "NO");
    Move moves[MAX_MOVES];
    int count = generateLegalMoves(&pos, moves);
    printf("Black legal moves: %d (expected: 0)\n", count);
    for (int i = 0; i < count; i++) {
        printf("  Legal move: (%d,%d) -> (%d,%d)\n", 
            moves[i].fromRow, moves[i].fromCol, moves[i].toRow, moves[i].toCol);
    }
}

void testStalemate() {
    printf("\n--- Stalemate Test ---\n");
    Position pos;
    clearBoard(&pos);
    pos.sideToMove = BLACK;
    pos.whiteKingRow = 2; pos.whiteKingCol = 2;
    pos.blackKingRow = 0; pos.blackKingCol = 0;
    pos.board[2][2] = WKING;
    pos.board[0][0] = BKING;
    pos.board[1][2] = WQUEEN;

    printBoard(&pos);
    printf("Black in check: %s\n", isInCheck(&pos, BLACK) ? "YES" : "NO");
    printf("Is stalemate: %s (expected: YES)\n", isStalemate(&pos) ? "YES" : "NO");
    Move moves[MAX_MOVES];
    int count = generateLegalMoves(&pos, moves);
    printf("Black legal moves: %d (expected: 0)\n", count);
}

void testTerminalState() {
    printf("\n--- Terminal State Test ---\n");
    Position pos;
    clearBoard(&pos);
    pos.sideToMove = BLACK;
    pos.whiteKingRow = 1; pos.whiteKingCol = 2;
    pos.blackKingRow = 0; pos.blackKingCol = 0;
    pos.board[1][2] = WKING;
    pos.board[0][0] = BKING;
    pos.board[2][0] = WQUEEN;

    printBoard(&pos);
    int result;
    int isTerm = isTerminal(&pos, &result);
    printf("Is terminal: %s (expected: YES)\n", isTerm ? "YES" : "NO");
    printf("Result: %d (expected: 1 for white win)\n", result);

    clearBoard(&pos);
    pos.sideToMove = BLACK;
    pos.whiteKingRow = 2; pos.whiteKingCol = 2;
    pos.blackKingRow = 0; pos.blackKingCol = 0;
    pos.board[2][2] = WKING;
    pos.board[0][0] = BKING;
    pos.board[1][2] = WQUEEN;

    printBoard(&pos);
    isTerm = isTerminal(&pos, &result);
    printf("Is terminal: %s (expected: YES)\n", isTerm ? "YES" : "NO");
    printf("Result: %d (expected: 0 for draw)\n", result);

    clearBoard(&pos);
    initBoard(&pos);
    isTerm = isTerminal(&pos, &result);
    printf("Starting position - Is terminal: %s (expected: NO)\n", isTerm ? "YES" : "NO");
}

void testFiftyMoveRule() {
    printf("\n--- 50-Move Rule Test ---\n");
    Position pos;
    clearBoard(&pos);
    pos.sideToMove = WHITE;
    pos.whiteKingRow = 7; pos.whiteKingCol = 4;
    pos.blackKingRow = 0; pos.blackKingCol = 4;
    pos.board[7][4] = WKING;
    pos.board[0][4] = BKING;
    pos.halfmoveClock = 100;

    printBoard(&pos);
    int result;
    int isTerm = isTerminal(&pos, &result);
    printf("Is terminal (100 halfmoves): %s (expected: YES)\n", isTerm ? "YES" : "NO");
    printf("Result: %d (expected: 0 for draw)\n", result);

    pos.halfmoveClock = 99;
    isTerm = isTerminal(&pos, &result);
    printf("Is terminal (99 halfmoves): %s (expected: NO)\n", isTerm ? "YES" : "NO");
}

void testPerft() {
    printf("\n--- Perft Test (Starting Position) ---\n");
    Position pos;
    initBoard(&pos);

    printf("Depth 1: %llu (expected: 20)\n", (unsigned long long)perft(&pos, 1));
    printf("Depth 2: %llu (expected: 400)\n", (unsigned long long)perft(&pos, 2));
    printf("Depth 3: %llu (expected: 8902)\n", (unsigned long long)perft(&pos, 3));
    printf("Depth 4: %llu (expected: 197281)\n", (unsigned long long)perft(&pos, 4));
    printf("Depth 5: %llu (expected: 4865609)\n", (unsigned long long)perft(&pos, 5));
}

void testPerftDivide() {
    printf("\n--- Perft Divide Test (Starting Position, Depth 3) ---\n");
    Position pos;
    initBoard(&pos);
    perftDivide(&pos, 3);
}

void testPerftKiwipete() {
    printf("\n--- Perft Test (Kiwipete Position) ---\n");
    Position pos;
    clearBoard(&pos);
    pos.sideToMove = WHITE;
    pos.whiteKingRow = 7; pos.whiteKingCol = 4;
    pos.blackKingRow = 0; pos.blackKingCol = 4;
    
    // r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1
    // Rank 8 (row 0): r 3 k 2 r -> r . . . k . . r
    pos.board[0][0] = BROOK;
    pos.board[0][4] = BKING;
    pos.board[0][7] = BROOK;
    // Rank 7 (row 1): p 1 p p q p b 1 -> p . p p q p b .
    pos.board[1][0] = BPAWN;
    pos.board[1][2] = BPAWN;
    pos.board[1][3] = BPAWN;
    pos.board[1][4] = BQUEEN;
    pos.board[1][5] = BPAWN;
    pos.board[1][6] = BBISHOP;
    // Rank 6 (row 2): b n 2 p n p 1 -> b n . . p n p .
    pos.board[2][0] = BBISHOP;
    pos.board[2][1] = BKNIGHT;
    pos.board[2][4] = BPAWN;
    pos.board[2][5] = BKNIGHT;
    pos.board[2][6] = BPAWN;
    // Rank 5 (row 3): 3 P N 3 -> . . . P N . . .
    pos.board[3][3] = WPAWN;
    pos.board[3][4] = WKNIGHT;
    // Rank 4 (row 4): 1 p 2 P 3 -> . p . . P . . .
    pos.board[4][1] = BPAWN;
    pos.board[4][4] = WPAWN;
    // Rank 3 (row 5): 2 N 2 Q 1 p -> . . N . . Q . p
    pos.board[5][2] = WKNIGHT;
    pos.board[5][5] = WQUEEN;
    pos.board[5][7] = BPAWN;
    // Rank 2 (row 6): P P P B B P P P -> P P P B B P P P
    pos.board[6][0] = WPAWN;
    pos.board[6][1] = WPAWN;
    pos.board[6][2] = WPAWN;
    pos.board[6][3] = WBISHOP;
    pos.board[6][4] = WBISHOP;
    pos.board[6][5] = WPAWN;
    pos.board[6][6] = WPAWN;
    pos.board[6][7] = WPAWN;
    // Rank 1 (row 7): R 3 K 2 R -> R . . . K . . R
    pos.board[7][0] = WROOK;
    pos.board[7][4] = WKING;
    pos.board[7][7] = WROOK;

    pos.whiteKingMoved = 0;
    pos.whiteLeftRookMoved = 0;
    pos.whiteRightRookMoved = 0;
    pos.blackKingMoved = 0;
    pos.blackLeftRookMoved = 0;
    pos.blackRightRookMoved = 0;
    pos.halfmoveClock = 0;
    pos.fullmoveNumber = 1;
    pos.enPassantRow = -1;
    pos.enPassantCol = -1;

    printf("Depth 1: %llu (expected: 48)\n", (unsigned long long)perft(&pos, 1));
    printf("Depth 2: %llu (expected: 2039)\n", (unsigned long long)perft(&pos, 2));
    printf("Depth 3: %llu (expected: 97862)\n", (unsigned long long)perft(&pos, 3));
    printf("Depth 4: %llu (expected: 4085603)\n", (unsigned long long)perft(&pos, 4));
}

void testPerftPosition3() {
    printf("\n--- Perft Test (Position 3: Promotions/Castling/En Passant) ---\n");
    Position pos;
    clearBoard(&pos);
    pos.sideToMove = WHITE;
    pos.whiteKingRow = 4; pos.whiteKingCol = 0;
    pos.blackKingRow = 3; pos.blackKingCol = 6;
    
    // 8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1
    // Rank 8 (row 0): 8 -> all empty
    // Rank 7 (row 1): 2p5 -> . . p . . . . .
    pos.board[1][2] = BPAWN;
    // Rank 6 (row 2): 3p4 -> . . . p . . . .
    pos.board[2][3] = BPAWN;
    // Rank 5 (row 3): KP5r -> K P . . . . . r
    pos.board[3][0] = WKING;
    pos.board[3][1] = WPAWN;
    pos.board[3][7] = BROOK;
    // Rank 4 (row 4): 1R3p1k -> . R . . . p . k
    pos.board[4][1] = WROOK;
    pos.board[4][5] = BPAWN;
    pos.board[4][7] = BKING;
    // Rank 3 (row 5): 8 -> all empty
    // Rank 2 (row 6): 4P1P1 -> . . . . P . P .
    pos.board[6][4] = WPAWN;
    pos.board[6][6] = WPAWN;
    // Rank 1 (row 7): 8 -> all empty

    pos.whiteKingMoved = 1;
    pos.whiteLeftRookMoved = 1;
    pos.whiteRightRookMoved = 1;
    pos.blackKingMoved = 1;
    pos.blackLeftRookMoved = 1;
    pos.blackRightRookMoved = 1;
    pos.halfmoveClock = 0;
    pos.fullmoveNumber = 1;
    pos.enPassantRow = -1;
    pos.enPassantCol = -1;

    printf("Depth 1: %llu (expected: 14)\n", (unsigned long long)perft(&pos, 1));
    printf("Depth 2: %llu (expected: 191)\n", (unsigned long long)perft(&pos, 2));
    printf("Depth 3: %llu (expected: 2812)\n", (unsigned long long)perft(&pos, 3));
    printf("Depth 4: %llu (expected: 43238)\n", (unsigned long long)perft(&pos, 4));
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
    testCheckmate();
    testStalemate();
    testTerminalState();
    testFiftyMoveRule();
    testPerft();
    testPerftDivide();
    testPerftKiwipete();
    testPerftPosition3();
    return 0;
}