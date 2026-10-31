# Shinsou Chess Engine

Shinsou means "a deeper layer" in Japanese. This project builds a chess engine from first principles with a focus on correctness, testability, and clean architecture.

The project consists of a high-performance chess engine written in C and a modern React frontend. The C engine is compiled to WebAssembly (WASM) using Emscripten, allowing the engine to run directly in the browser.

### C Engine Implementation
The C engine implements:
- Board representation with full game state (side to move, castling rights, en passant, halfmove clock, fullmove number, king positions).
- Pseudo-legal and legal move generation with full check detection and special rules (castling, en passant, promotion).
- A sophisticated search pipeline featuring Negamax, Alpha-Beta pruning, Principal Variation Search (PVS), Null-Move Pruning (NMP), and Late Move Reductions (LMR).
- Move ordering using MVV-LVA, Killer moves, and History heuristics.
- Transposition Table (TT) using Zobrist hashing for move caching.
- Evaluation based on material and piece-square tables.

### Frontend Implementation
The frontend is a React application that provides a professional chess interface:
- WASM Bridge: Interfaces with the C engine via Emscripten's `ccall`.
- Game Logic: Integrated with `chess.js` for move validation and FEN management.
- User Experience: Supports color selection, game restarts, and real-time engine responses.

## Project Structure

```
shinsou-chess/
├── chess-engine/
│   └── engine-c/           # C Source code and build files
│       ├── board/          # Board representation and state
│       ├── movegen/        # Move generation logic
│       ├── engine/         # Search and evaluation
│       ├── fen/            # FEN parsing and generation
│       ├── tests/          # Test suite
│       ├── utils/          # Constants and types
│       ├── main.c          # Engine entry point
│       └── Makefile        # Build configurations (native and WASM)
├── emsdk/                  # Emscripten SDK (requires download)
├── frontend/               # React frontend
│   ├── public/             # Static assets and WASM binaries
│   │   └── engine/         # Compiled engine.js and engine.wasm
│   └── src/                # React source code
│       ├── components/     # UI components (Board, Square, etc.)
│       ├── engine/         # WASM loader and Engine bridge
│       ├── utils/          # Chess utility functions
│       └── App.jsx         # Main application entry
└── DESIGN.md               # Detailed design phases and principles
```

## Running Instructions

### 1. Setup Emscripten (WASM Compiler)
The engine requires the Emscripten SDK to compile C to WASM.
- Download and install the `emsdk` into the root directory.
- Configure the environment in your terminal:
  ```bash
  cd emsdk
  ./emsdk install latest
  ./emsdk activate latest
  # Windows:
  .\emsdk_env.ps1
  # Linux/Mac:
  source ./emsdk_env.sh
  ```

### 2. Build the WASM Engine
Compile the C code and move the binaries to the frontend public folder:
```bash
cd chess-engine/engine-c
make wasm
```

### 3. Run the Frontend
Launch the React application:
```bash
cd frontend
npm install
npm run dev
```
The application will be available at `http://localhost:5173` by default.
