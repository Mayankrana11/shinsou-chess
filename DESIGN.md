# Shinsou Chess Design Document

This document outlines the development phases and architectural principles of the Shinsou Chess Engine.

## Completed Phases

### Phase 1 - Board
- Project structure and constants
- Position struct with complete game state
- Board initialization and printing

### Phase 2 - Move Structure
- Move struct with source, destination, piece, captured, promotion, type flags, and undo state
- Fixed-size move list (MAX_MOVES = 256)

### Phase 3 - Move Generation
- Coordinate helpers (inBounds, sameColor)
- Pawn moves (single, double, captures, promotions)
- Knight moves (offset table)
- Bishop, rook, queen moves (ray traversal)
- King moves (single square)

### Phase 4 - Attack System
- isSquareAttacked for all piece types
- findKing and king square tracking
- isInCheck using attack detection

### Phase 5 - Make/Undo
- makeMove updates board, side to move, castling rights, en passant, clocks, king positions
- undoMove restores exact position state
- Verified with make/undo round-trip test

### Phase 6 - Special Rules
- Castling with full legality (king not in check, not through check, not into check, squares empty)
- En passant with discovered check handling
- Promotion (queen, rook, bishop, knight) with make/undo integration

### Phase 7 - Legal Moves
- Separate pseudo-legal from legal generation
- Test pinned pieces, discovered checks, double check, castling through check, en passant discovered check

### Phase 8 - Checkmate/Stalemate
- isCheckmate and isStalemate detection
- Terminal state handling

### Phase 9 - Perft
- perft(depth) node counting
- perft divide for debugging
- Validation against known positions

### Phase 10 - FEN
- FEN parsing and generation
- All six fields support

### Phase 11 - Evaluation
- Material evaluation
- Piece-square tables
- Positional factors

### Phase 12 - Search
- Negamax
- Fixed-depth search
- Alpha-beta pruning
- Terminal conditions

### Phase 13 - Quiescence
- Capture and promotion search at depth 0

### Phase 14 - Move Ordering
- MVV-LVA (Most Valuable Victim - Least Valuable Attacker) capture ordering
- Killer moves (2 slots per ply with FIFO replacement)
- History heuristic (depth² weighted, capped)
- Incremental selection sort (pickBestMove) for efficient early cutoffs
- Quiescence search MVV-LVA ordering
- Fixed mate-distance scoring (ply-based instead of depth-based)

### Phase 15 - Iterative Deepening

### Phase 16 - Transposition Table
- Zobrist hashing
- Transposition table implementation

### Phase 17 - Search Upgrades
- Principal variation search
- Aspiration windows
- Null-move pruning
- Late move reductions
- Futility pruning

### Phase 18 - Bitboards
- Bitboard representation
- Magic bitboard attack generation

## Next Steps

### Phase 19 - Optimization
- Memory layout
- Hot path optimization
- Profiling-guided improvements

### Phase 20 - UCI
- UCI protocol support

### Phase 21 - WASM
- Emscripten compilation
- WASM interface

### Phase 22-27 - Frontend Integration
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
