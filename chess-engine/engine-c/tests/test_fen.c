#include <stdio.h>
#include <string.h>
#include "../board/board.h"
#include "../fen/fen.h"
#include "../utils/constants.h"

void test_fen_roundtrip(const char* original_fen) {
    Position pos;
    char generated_fen[128];

    printf("Testing FEN: %s\n", original_fen);

    if (!parseFEN(&pos, original_fen)) {
        printf("FAILED: parseFEN returned 0 (Invalid FEN or parsing error)\n");
        printf("--------------------------------------------------\n");
        return;
    }

    generateFEN(&pos, generated_fen);
    printf("Generated: %s\n", generated_fen);

    if (strcmp(original_fen, generated_fen) == 0) {
        printf("SUCCESS: Roundtrip match!\n");
    } else {
        printf("FAILED: Roundtrip mismatch!\n");
        printf("Expected: %s\n", original_fen);
        printf("Got:      %s\n", generated_fen);
    }
    printf("--------------------------------------------------\n");
}

int main() {
    // Starting position - 8 ranks
    test_fen_roundtrip("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");

    // Custom position - 8 ranks
    test_fen_roundtrip("rnbqkbnr/pppp1ppp/8/3p4/2PP4/2P5/PP1PPPPP/8 w KQkq - 0 1");

    // Position with En Passant target - 8 ranks
    test_fen_roundtrip("rnbqkbnr/pp1ppppp/8/2p5/4P3/8/PPPP1PPP/8 w KQkq c3 0 1");

    // Position with no castling and no EP - 8 ranks
    test_fen_roundtrip("r1bqkbnr/ppp1pppp/2n5/4p3/4P3/5P2/PPPP1PPP/8 w KQkq - 0 1");

    return 0;
}
