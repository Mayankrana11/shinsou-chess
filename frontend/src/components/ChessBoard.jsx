import { useState, useMemo } from "react";
import { Chess } from "chess.js";
import Square from "./Square";
import PlayerBar from "./PlayerBar";
import MoveViewer from "./MoveViewer";
import { getBestMove } from "../engine/EngineBridge";
import "../styles/board.css";

const FILES = ["a", "b", "c", "d", "e", "f", "g", "h"];

// Clone a game while preserving its full move history.
// NOTE: `new Chess(game.fen())` must NOT be used for this — loading a FEN
// discards all prior moves, which breaks move-viewing and PGN export.
const cloneGame = (src) => {
  const copy = new Chess();
  for (const san of src.history()) {
    copy.move(san);
  }
  return copy;
};

export default function ChessBoard() {
  const [game, setGame] = useState(new Chess());
  const [userColor, setUserColor] = useState(null); // 'w' | 'b' | null
  const [manualWhiteOpeningMove, setManualWhiteOpeningMove] = useState(false);
  const [isFlipped, setIsFlipped] = useState(false);
  const [selected, setSelected] = useState(null);
  const [validMoves, setValidMoves] = useState([]);
  const [lastMove, setLastMove] = useState(null);
  const [isEngineThinking, setIsEngineThinking] = useState(false);
  const [viewedPly, setViewedPly] = useState(null); // null = live position, 0..N = past ply
  const [copiedPgn, setCopiedPgn] = useState(false);

  const filesToCoord = (col) => FILES[col];

  // Full move history of the live game
  const history = useMemo(() => game.history({ verbose: true }), [game]);
  const totalPlies = history.length;
  const isSpectating = viewedPly !== null && viewedPly < totalPlies;
  const currentPly = viewedPly !== null ? viewedPly : totalPlies;

  // Active chess instance representing either the live game or the spectated past ply
  const currentChess = useMemo(() => {
    if (!isSpectating) return game;
    if (currentPly === 0) {
      return new Chess(); // Initial start position
    }
    const targetFen = history[currentPly - 1]?.after;
    return targetFen ? new Chess(targetFen) : game;
  }, [isSpectating, currentPly, game, history]);

  // Convert current chess board to 8x8 2D array (row 0 = Rank 8, row 7 = Rank 1)
  const currentBoard = useMemo(() => {
    const board = Array(8)
      .fill(null)
      .map(() => Array(8).fill(null));
    currentChess.board().forEach((row, r) => {
      row.forEach((cell, c) => {
        if (cell) {
          board[r][c] =
            cell.color === "w"
              ? cell.type.toUpperCase()
              : cell.type.toLowerCase();
        }
      });
    });
    return board;
  }, [currentChess]);

  // Highlight of the move that led to the currently displayed position
  const effectiveLastMove = useMemo(() => {
    if (isSpectating) {
      if (currentPly === 0) return null;
      const m = history[currentPly - 1];
      return m ? { from: m.from, to: m.to } : null;
    }
    return lastMove;
  }, [isSpectating, currentPly, history, lastMove]);

  // Find king square if current side to move is in check
  const checkedKingSquare = useMemo(() => {
    if (!currentChess.inCheck()) return null;
    const turnColor = currentChess.turn();
    const board = currentChess.board();
    for (let r = 0; r < 8; r++) {
      for (let c = 0; c < 8; c++) {
        const cell = board[r][c];
        if (cell && cell.type === "k" && cell.color === turnColor) {
          return `${filesToCoord(c)}${8 - r}`;
        }
      }
    }
    return null;
  }, [currentChess]);

  // Calculate captured pieces & material difference at the active/spectated ply
  const {
    whiteCaptured,
    blackCaptured,
    whiteAdvantage,
    blackAdvantage,
  } = useMemo(() => {
    const movesSlice = history.slice(0, currentPly);
    const wCap = [];
    const bCap = [];

    for (const m of movesSlice) {
      if (m.captured) {
        if (m.color === "w") {
          // White made the capture -> won a Black piece
          wCap.push(m.captured);
        } else {
          // Black made the capture -> won a White piece
          bCap.push(m.captured);
        }
      }
    }

    const PIECE_VALUES = { p: 1, n: 3, b: 3, r: 5, q: 9, k: 0 };
    let wMat = 0;
    let bMat = 0;

    currentChess.board().forEach((row) => {
      row.forEach((cell) => {
        if (cell) {
          const val = PIECE_VALUES[cell.type] || 0;
          if (cell.color === "w") wMat += val;
          else bMat += val;
        }
      });
    });

    const diff = wMat - bMat;
    return {
      whiteCaptured: wCap,
      blackCaptured: bCap,
      whiteAdvantage: diff > 0 ? diff : 0,
      blackAdvantage: diff < 0 ? -diff : 0,
    };
  }, [history, currentPly, currentChess]);

  // Start or restart game with chosen color ('w' or 'b')
  const startGame = (color) => {
    if (isEngineThinking) return;
    const freshGame = new Chess();
    setUserColor(color);
    setGame(freshGame);
    setSelected(null);
    setValidMoves([]);
    setLastMove(null);
    setViewedPly(null);
    setCopiedPgn(false);
    setIsFlipped(color === "b");
    // When playing as Black, the user manually makes White's very first move
    setManualWhiteOpeningMove(color === "b");
  };

  const toggleFlipBoard = () => {
    setIsFlipped((prev) => !prev);
  };

  const triggerEngineMove = async (currentGame) => {
    if (currentGame.isGameOver()) return;
    setIsEngineThinking(true);
    try {
      const fen = currentGame.fen();
      const bestMove = await getBestMove(fen);
      console.log("Engine raw output:", bestMove);

      const fromFile = FILES[bestMove.fromCol];
      const fromRank = 8 - bestMove.fromRow;
      const toFile = FILES[bestMove.toCol];
      const toRank = 8 - bestMove.toRow;
      const from = `${fromFile}${fromRank}`;
      const to = `${toFile}${toRank}`;

      const newGame = cloneGame(currentGame);
      const moveResult = newGame.move({ from, to, promotion: "q" });

      if (!moveResult) {
        throw new Error(`Chess.js rejected move ${from}${to} as invalid`);
      }

      setGame(newGame);
      setLastMove({ from, to });
      setViewedPly(null); // Keep board pinned to live position
    } catch (e) {
      console.error("Engine move failed:", e);
    } finally {
      setIsEngineThinking(false);
    }
  };

  // Determine which side's pieces the user can currently move
  const controllableColor =
    userColor === "b" && manualWhiteOpeningMove ? "w" : userColor;

  const handleClick = async (actualRow, actualCol) => {
    // In Spectate Mode, clicking never selects or moves pieces
    if (isEngineThinking || !userColor || game.isGameOver() || isSpectating) {
      return;
    }

    const square = `${filesToCoord(actualCol)}${8 - actualRow}`;

    // If a piece is already selected, attempt to move it to the clicked square
    if (selected) {
      try {
        const nextGame = cloneGame(game);
        const move = nextGame.move({
          from: selected,
          to: square,
          promotion: "q",
        });

        if (move) {
          setGame(nextGame);
          setSelected(null);
          setValidMoves([]);
          setLastMove({ from: selected, to: square });
          setViewedPly(null);

          // If the user is playing Black and just made White's opening move manually,
          // hand control directly to the user for Black's 1st move without calling the engine.
          if (userColor === "b" && manualWhiteOpeningMove) {
            setManualWhiteOpeningMove(false);
            return;
          }

          // Otherwise, if it is now the engine's turn, trigger the WASM engine reply
          if (nextGame.turn() !== userColor && !nextGame.isGameOver()) {
            await triggerEngineMove(nextGame);
          }
          return;
        }
      } catch {
        // Clicked square was not a valid move for the currently selected piece
      }
    }

    // Select a piece if it belongs to the currently controllable side
    const piece = game.get(square);
    if (
      piece &&
      piece.color === controllableColor &&
      game.turn() === controllableColor
    ) {
      setSelected(square);
      const moves = game.moves({ square, verbose: true });
      setValidMoves(moves);
    } else {
      setSelected(null);
      setValidMoves([]);
    }
  };

  const getMoveTargetInfo = (actualRow, actualCol) => {
    if (isSpectating) return null;
    const square = `${filesToCoord(actualCol)}${8 - actualRow}`;
    return validMoves.find((m) => m.to === square) || null;
  };

  const getStatusText = () => {
    if (game.isCheckmate()) {
      const winner = game.turn() === "w" ? "Black" : "White";
      return `Checkmate — ${winner} wins!`;
    }
    if (game.isStalemate()) return "Draw by Stalemate";
    if (game.isDraw()) return "Game Drawn";
    if (isEngineThinking) return "Shinsou Engine is thinking...";
    if (userColor === "b" && manualWhiteOpeningMove) {
      return "Opening Move: Play White's 1st move manually";
    }
    if (game.turn() === userColor) {
      return `Your turn (${userColor === "w" ? "White" : "Black"})${
        game.inCheck() ? " — Check!" : ""
      }`;
    }
    return "Engine's turn";
  };

  // Generate standard, analysable PGN and copy to clipboard
  const handleExportPGN = async () => {
    try {
      // Replay the full history (SAN) so the PGN contains every move
      const pgnGame = cloneGame(game);

      const today = new Date();
      const yyyy = today.getFullYear();
      const mm = String(today.getMonth() + 1).padStart(2, "0");
      const dd = String(today.getDate()).padStart(2, "0");
      const dateStr = `${yyyy}.${mm}.${dd}`;

      let resultStr = "*";
      if (game.isCheckmate()) {
        resultStr = game.turn() === "w" ? "0-1" : "1-0";
      } else if (game.isDraw() || game.isStalemate()) {
        resultStr = "1/2-1/2";
      }

      const whiteName =
        userColor === "w" ? "Player (Human)" : "Shinsou C11 Engine";
      const blackName =
        userColor === "b" ? "Player (Human)" : "Shinsou C11 Engine";

      pgnGame.setHeader("Event", "Shinsou Chess Casual Game");
      pgnGame.setHeader("Site", "Shinsou Chess (深層)");
      pgnGame.setHeader("Date", dateStr);
      pgnGame.setHeader("Round", "1");
      pgnGame.setHeader("White", whiteName);
      pgnGame.setHeader("Black", blackName);
      pgnGame.setHeader("Result", resultStr);

      const pgnText = pgnGame.pgn();

      const copyFallback = (text) => {
        const ta = document.createElement("textarea");
        ta.value = text;
        ta.setAttribute("readonly", "");
        ta.style.position = "fixed";
        ta.style.left = "-9999px";
        ta.style.top = "-9999px";
        document.body.appendChild(ta);
        ta.focus();
        ta.select();
        try {
          return document.execCommand("copy");
        } finally {
          ta.remove();
        }
      };

      let copied = false;
      if (navigator.clipboard && window.isSecureContext) {
        try {
          await navigator.clipboard.writeText(pgnText);
          copied = true;
        } catch {
          copied = copyFallback(pgnText);
        }
      } else {
        copied = copyFallback(pgnText);
      }

      if (!copied) {
        throw new Error("Clipboard write was rejected by the browser");
      }

      console.log("Exported PGN:\n" + pgnText);
      setCopiedPgn(true);
      setTimeout(() => setCopiedPgn(false), 2500);
    } catch (err) {
      console.error("Failed to copy PGN:", err);
    }
  };

  // Initial color selection screen before a game starts
  if (!userColor) {
    return (
      <div className="setup-card">
        <h2 className="setup-title">Choose Your Side</h2>
        <p className="setup-description">
          Select a color to begin playing against the Shinsou C11 WebAssembly engine.
        </p>
        <div className="color-choice-grid">
          <button
            type="button"
            className="color-choice-btn choice-white"
            onClick={() => startGame("w")}
          >
            <img src="/pieces/wK.svg" alt="White King" className="choice-piece-icon" />
            <span className="choice-label">Play as White</span>
            <span className="choice-hint">You make the opening move</span>
          </button>

          <button
            type="button"
            className="color-choice-btn choice-black"
            onClick={() => startGame("b")}
          >
            <img src="/pieces/bK.svg" alt="Black King" className="choice-piece-icon" />
            <span className="choice-label">Play as Black</span>
            <span className="choice-hint">
              Play White&apos;s 1st move, then reply as Black
            </span>
          </button>
        </div>
      </div>
    );
  }

  // Perspective mapping: top side vs bottom side
  const topSide = isFlipped ? "w" : "b";
  const bottomSide = isFlipped ? "b" : "w";

  const topCaptured = topSide === "w" ? whiteCaptured : blackCaptured;
  const topAdvantage = topSide === "w" ? whiteAdvantage : blackAdvantage;

  const bottomCaptured = bottomSide === "w" ? whiteCaptured : blackCaptured;
  const bottomAdvantage = bottomSide === "w" ? whiteAdvantage : blackAdvantage;

  const currentTurnSide = currentChess.turn();

  return (
    <div className="game-workspace">
      {/* Top Status & Control Toolbar */}
      <div className="game-toolbar">
        <div className="status-banner">
          <span
            className={`status-dot ${
              isEngineThinking
                ? "thinking"
                : game.isGameOver()
                ? "gameover"
                : userColor === "b" && manualWhiteOpeningMove
                ? "opening"
                : "ready"
            }`}
          />
          <span className="status-text">{getStatusText()}</span>
        </div>

        <div className="toolbar-actions">
          <button
            type="button"
            className="action-btn flip-btn"
            onClick={toggleFlipBoard}
            title="Visually flip the chessboard perspective"
          >
            ⇅ Switch Board
          </button>

          <div className="new-game-group">
            <button
              type="button"
              className={`action-btn color-pill ${
                userColor === "w" ? "active-pill" : ""
              }`}
              onClick={() => startGame("w")}
              disabled={isEngineThinking}
              title="Start new game as White"
            >
              <span className="swatch swatch-white" /> New White
            </button>
            <button
              type="button"
              className={`action-btn color-pill ${
                userColor === "b" ? "active-pill" : ""
              }`}
              onClick={() => startGame("b")}
              disabled={isEngineThinking}
              title="Start new game as Black"
            >
              <span className="swatch swatch-black" /> New Black
            </button>
          </div>
        </div>
      </div>

      {/* Game Over Banner with prominent PGN export */}
      {game.isGameOver() && (
        <div className="game-over-banner">
          <div className="game-over-left">
            <span className="game-over-trophy">🏁</span>
            <div className="game-over-text-col">
              <span className="game-over-title">Game Finished</span>
              <span className="game-over-summary">{getStatusText()}</span>
            </div>
          </div>
          <button
            type="button"
            className={`export-pgn-banner-btn ${copiedPgn ? "copied" : ""}`}
            onClick={handleExportPGN}
          >
            {copiedPgn ? "✓ PGN Copied to Clipboard!" : "📋 Export PGN for Analysis"}
          </button>
        </div>
      )}

      {/* Spectate Mode Read-Only Banner */}
      {isSpectating && !game.isGameOver() && (
        <div className="spectate-mode-banner">
          <div className="spectate-banner-info">
            <span className="spectate-banner-icon">👁</span>
            <span>
              Spectate Mode: Viewing past move (read-only)
            </span>
          </div>
          <button
            type="button"
            className="spectate-return-btn"
            onClick={() => setViewedPly(null)}
          >
            ▶ Return to Live Game
          </button>
        </div>
      )}

      {/* Top Player Bar (Opponent or White when flipped) */}
      <PlayerBar
        side={topSide}
        isUser={topSide === userColor}
        isCurrentTurn={
          currentTurnSide === topSide && !game.isGameOver() && !isSpectating
        }
        isThinking={isEngineThinking && topSide !== userColor}
        capturedPieces={topCaptured}
        advantage={topAdvantage}
      />

      {/* 8x8 Chessboard Grid */}
      <div className="board-frame">
        <div className={`board ${isSpectating ? "spectate-board" : ""}`}>
          {Array.from({ length: 8 }).map((_, displayRow) =>
            Array.from({ length: 8 }).map((__, displayCol) => {
              const actualRow = isFlipped ? 7 - displayRow : displayRow;
              const actualCol = isFlipped ? 7 - displayCol : displayCol;

              const piece = currentBoard[actualRow][actualCol];
              const isWhiteSquare = (actualRow + actualCol) % 2 === 0;
              const squareNotation = `${filesToCoord(actualCol)}${8 - actualRow}`;
              const isSelected = selected === squareNotation;
              const moveTarget = getMoveTargetInfo(actualRow, actualCol);
              const isLastMove =
                effectiveLastMove &&
                (effectiveLastMove.from === squareNotation ||
                  effectiveLastMove.to === squareNotation);
              const isCheck = checkedKingSquare === squareNotation;

              // Rank markings (1-8) on left-most visual column; File markings (a-h) on bottom-most visual row
              const rankLabel =
                displayCol === 0 ? String(8 - actualRow) : null;
              const fileLabel =
                displayRow === 7 ? filesToCoord(actualCol) : null;

              return (
                <Square
                  key={`${actualRow}-${actualCol}`}
                  piece={piece}
                  isWhite={isWhiteSquare}
                  isSelected={isSelected}
                  isMove={Boolean(moveTarget)}
                  isCapture={Boolean(moveTarget && moveTarget.captured)}
                  isLastMove={isLastMove}
                  isCheck={isCheck}
                  rankLabel={rankLabel}
                  fileLabel={fileLabel}
                  onClick={() => handleClick(actualRow, actualCol)}
                />
              );
            })
          )}
        </div>
      </div>

      {/* Bottom Player Bar (User or Black when flipped) */}
      <PlayerBar
        side={bottomSide}
        isUser={bottomSide === userColor}
        isCurrentTurn={
          currentTurnSide === bottomSide && !game.isGameOver() && !isSpectating
        }
        isThinking={isEngineThinking && bottomSide !== userColor}
        capturedPieces={bottomCaptured}
        advantage={bottomAdvantage}
      />

      {/* Move Reverse Viewer & Move History */}
      <MoveViewer
        history={history}
        viewedPly={viewedPly}
        onSelectPly={setViewedPly}
        isEngineThinking={isEngineThinking}
        isGameOver={game.isGameOver()}
        onExportPGN={handleExportPGN}
        copiedPgn={copiedPgn}
      />

      {/* Subtle footer badge showing current player side & board orientation */}
      <div className="board-meta-bar">
        <span>
          Playing as:{" "}
          <strong>{userColor === "w" ? "White" : "Black"}</strong>
        </span>
        <span className="meta-divider">•</span>
        <span>
          Perspective:{" "}
          <strong>{isFlipped ? "Black at bottom" : "White at bottom"}</strong>
        </span>
      </div>
    </div>
  );
}
