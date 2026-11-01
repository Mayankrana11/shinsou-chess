#include <stdio.h>
#include <string.h>
#include <emscripten.h>
#include "board.h"
#include "movegen/movegen.h"
#include "fen/fen.h"
#include "engine/search.h"
#include "engine/ordering.h"
#include "engine/tt.h"

// Global position to avoid repeated allocation
Position global_pos;
static int engine_initialized = 0;

EMSCRIPTEN_KEEPALIVE
const char* get_best_move(const char* fen) {
    if (!engine_initialized) {
        initZobrist();
        initTT();
        engine_initialized = 1;
    }

    initOrdering();

    // 1. Parse the FEN string provided by the frontend
    parseFEN(&global_pos, fen);

    // 2. Call your engine's search function
    Move bestMove = {0};
    // Depth 6 is a good balance for WASM browser performance
    findBestMove(&global_pos, 6, &bestMove);

    // 3. Convert the move to a string format JS can understand (e.g., "fromRow,fromCol,toRow,toCol")
    static char move_buffer[32];
    snprintf(move_buffer, sizeof(move_buffer), "%d,%d,%d,%d",
             bestMove.fromRow, bestMove.fromCol, bestMove.toRow, bestMove.toCol);

    return move_buffer;
}
