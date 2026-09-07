#ifndef FEN_H
#define FEN_H

#include "../board/board.h"

/**
 * Parses a FEN string and updates the given Position.
 * @param pos The position to update.
 * @param fen The FEN string to parse.
 * @return 1 on success, 0 on failure.
 */
int parseFEN(Position* pos, const char* fen);

/**
 * Generates a FEN string for the given Position.
 * @param pos The position to generate FEN for.
 * @param buffer The buffer to store the FEN string. Must be large enough (at least 128 bytes).
 */
void generateFEN(const Position* pos, char* buffer);

#endif