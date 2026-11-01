import { useState } from "react";
import { Chess } from "chess.js";
import Square from "./Square";
import { getBestMove } from "../engine/EngineBridge";
import "../styles/board.css";

const FILES = ["a", "b", "c", "d", "e", "f", "g", "h"];

export default function ChessBoard() {
  const [game, setGame] = useState(new Chess());
  const [userColor, setUserColor] = useState(null); // 'w' | 'b' | null
  const [manualWhiteOpeningMove, setManualWhiteOpeningMove] = useState(false);
  const [isFlipped, setIsFlipped] = useState(false);
  const [selected, setSelected] = useState(null);
  const [validMoves, setValidMoves] = useState([]);
  const [lastMove, setLastMove] = useState(null);
  const [isEngineThinking, setIsEngineThinking] = useState(false);

  const filesToCoord = (col) => FILES[col];

  // Convert chess.js board to 8x8 2D array (row 0 = Rank 8, row 7 = Rank 1)
  const renderBoard = () => {
    const board = Array(8)
      .fill(null)
      .map(() => Array(8).fill(null));
    game.board().forEach((row, r) => {
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
  };

  // Start or restart game with chosen color ('w' or 'b')
  const startGame = (color) => {
    if (isEngineThinking) return;
    const freshGame = new Chess();
    setUserColor(color);
    setGame(freshGame);
    setSelected(null);
    setValidMoves([]);
    setLastMove(null);
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

      const newGame = new Chess(currentGame.fen());
      const moveResult = newGame.move({ from, to, promotion: "q" });

      if (!moveResult) {
        throw new Error(`Chess.js rejected move ${from}${to} as invalid`);
      }

      setGame(newGame);
      setLastMove({ from, to });
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
    if (isEngineThinking || !userColor || game.isGameOver()) return;

    const square = `${filesToCoord(actualCol)}${8 - actualRow}`;

    // If a piece is already selected, attempt to move it to the clicked square
    if (selected) {
      try {
        const nextGame = new Chess(game.fen());
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
    if (piece && piece.color === controllableColor && game.turn() === controllableColor) {
      setSelected(square);
      const moves = game.moves({ square, verbose: true });
      setValidMoves(moves);
    } else {
      setSelected(null);
      setValidMoves([]);
    }
  };

  const getMoveTargetInfo = (actualRow, actualCol) => {
    const square = `${filesToCoord(actualCol)}${8 - actualRow}`;
    return validMoves.find((m) => m.to === square) || null;
  };

  // Find king square if current side to move is in check
  const getCheckedKingSquare = () => {
    if (!game.inCheck()) return null;
    const turnColor = game.turn();
    const board = game.board();
    for (let r = 0; r < 8; r++) {
      for (let c = 0; c < 8; c++) {
        const cell = board[r][c];
        if (cell && cell.type === "k" && cell.color === turnColor) {
          return `${filesToCoord(c)}${8 - r}`;
        }
      }
    }
    return null;
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

  const currentBoard = renderBoard();
  const checkedKingSquare = getCheckedKingSquare();

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

      {/* 8x8 Chessboard Grid */}
      <div className="board-frame">
        <div className="board">
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
                lastMove &&
                (lastMove.from === squareNotation ||
                  lastMove.to === squareNotation);
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
