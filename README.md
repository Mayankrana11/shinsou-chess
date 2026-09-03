# Shinsou Chess Engine

Shinsou means "a deeper layer" in Japanese. This project builds a chess engine from first principles with a focus on correctness, testability, and clean architecture.

## Current Status: Steps 1-22 Complete

The C engine implements:

- Board representation with full game state (side to move, castling rights, en passant, halfmove clock, fullmove number, king positions)
- Pseudo-legal move generation for all pieces (pawn, knight, bishop, rook, queen, king)
- Legal move generation with check detection
- Special moves with full legality: castling (kingside and queenside), en passant, promotion (queen, rook, bishop, knight)
- Castling rules: king not in check, does not move through check, does not land in check
- En passant with discovered check detection
- Attack detection system (isSquareAttacked, isInCheck)
- Make and undo move with complete state restoration
- Comprehensive test suite verifying move generation, make/undo integrity, check detection, and special move legality

## Project Structure

```
shinsou-chess/
├── chess-engine/
│   └── engine-c/
│       ├── board/
│       │   ├── board.h
│       │   └── board.c
│       ├── movegen/
│       │   ├── movegen.h
│       │   ├── movegen.c
│       │   └── attacks.h
│       ├── engine/
│       │   ├── search.h
│       │   ├── search.c
│       │   ├── eval.h
│       │   └── eval.c
│       ├── utils/
│       │   ├── types.h
│       │   └── constants.h
│       ├── main.c
│       └── Makefile
├── frontend/
│   ├── public/
│   │   ├── pieces/
│   │   └── engine/
│   ├── src/
│   │   ├── components/
│   │   ├── engine/
│   │   ├── state/
│   │   ├── utils/
│   │   └── styles/
│   ├── package.json
│   └── vite.config.js
├── local/
│   └── masterp.txt
└── README.md
```

## Build and Test

```bash
cd chess-engine/engine-c
gcc -Wall -Wextra -Wpedantic -std=c11 -I. -o shinsou.exe board/board.c movegen/movegen.c main.c
./shinsou.exe
```

Expected output shows 20 legal moves from the starting position, make/undo test passing, and special move tests for check, castling (through check, into check, king in check), en passant (discovered check, legal), and promotion make/undo.

## Completed Phases

### Phase 1 - Board (Steps 1-5)
- Project structure and constants
- Position struct with complete game state
- Board initialization and printing

### Phase 2 - Move Structure (Steps 6-7)
- Move struct with source, destination, piece, captured, promotion, type flags, and undo state
- Fixed-size move list (MAX_MOVES = 256)

### Phase 3 - Move Generation (Steps 8-14)
- Coordinate helpers (inBounds, sameColor)
- Pawn moves (single, double, captures, promotions)
- Knight moves (offset table)
- Bishop, rook, queen moves (ray traversal)
- King moves (single square)

### Phase 4 - Attack System (Steps 15-17)
- isSquareAttacked for all piece types
- findKing and king square tracking
- isInCheck using attack detection

### Phase 5 - Make/Undo (Steps 18-19)
- makeMove updates board, side to move, castling rights, en passant, clocks, king positions
- undoMove restores exact position state
- Verified with make/undo round-trip test

### Phase 6 - Special Rules (Steps 20-22)
- Castling with full legality (king not in check, not through check, not into check, squares empty)
- En passant with discovered check handling
- Promotion (queen, rook, bishop, knight) with make/undo integration

## Next Steps (Steps 23-30)

### Phase 7 - Legal Moves (Steps 23-24)
- Separate pseudo-legal from legal generation (done)
- Test pinned pieces, discovered checks, double check, castling through check, en passant discovered check

### Phase 8 - Checkmate/Stalemate (Steps 25-27)
- isCheckmate and isStalemate detection
- Terminal state handling

### Phase 9 - Perft (Steps 28-30)
- perft(depth) node counting
- perft divide for debugging
- Validation against known positions

### Phase 10 - FEN (Steps 31-33)
- FEN parsing and generation
- All six fields support

### Phase 11 - Evaluation (Steps 34-36)
- Material evaluation
- Piece-square tables
- Positional factors

### Phase 12 - Search (Steps 37-40)
- Negamax
- Fixed-depth search
- Alpha-beta pruning
- Terminal conditions

### Phase 13 - Quiescence (Step 41)
- Capture and promotion search at depth 0

### Phase 14 - Move Ordering (Steps 42-45)
- MVV-LVA
- Killer moves
- History heuristic

### Phase 15 - Iterative Deepening (Step 46)

### Phase 16 - Transposition Table (Steps 47-50)
- Zobrist hashing
- Transposition table implementation

### Phase 17 - Search Upgrades (Steps 51+)
- Principal variation search
- Aspiration windows
- Null-move pruning
- Late move reductions
- Futility pruning

### Phase 18 - Bitboards (Future)
- Bitboard representation
- Magic bitboard attack generation

### Phase 19 - Optimization (Future)
- Memory layout
- Hot path optimization
- Profiling-guided improvements

### Phase 20 - UCI (Future)
- UCI protocol support

### Phase 21 - WASM (Future)
- Emscripten compilation
- WASM interface

### Phase 22-27 - Frontend Integration (Future)
- React UI
- Web Worker for engine
- Engine statistics display

## Design Principles

1. Correctness before optimization
2. Chess logic separate from UI logic
3. C engine usable independently
4. WASM as bridge between C and JavaScript
5. No dynamic allocation in search
6. Contiguous, cache-friendly data structures
7. APIs designed for future bitboard migration
8. Perft testing for move generator verification

