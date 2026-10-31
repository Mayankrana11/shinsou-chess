import { useState, useEffect } from "react";
import { Chess } from "chess.js";
import Square from "./Square";
import { getBestMove } from "../engine/EngineBridge";
import "../styles/board.css";

const initialBoardLayout = [
  ["r","n","b","q","k","b","n","r"],
  ["p","p","p","p","p","p","p","p"],
  [null,null,null,null,null,null,null,null],
  [null,null,null,null,null,null,null,null],
  [null,null,null,null,null,null,null,null],
  [null,null,null,null,null,null,null,null],
  ["P","P","P","P","P","P","P","P"],
  ["R","N","B","Q","K","B","N","R"]
];

export default function ChessBoard() {
  const [game, setGame] = useState(new Chess());
  const [userColor, setUserColor] = useState(null); // 'w' or 'b'
  const [selected, setSelected] = useState(null);
  const [validMoves, setValidMoves] = useState([]);
  const [isEngineThinking, setIsEngineThinking] = useState(false);

  // Convert chess.js board to 2D array for rendering
  const renderBoard = () => {
    const board = Array(8).fill(null).map(() => Array(8).fill(null));
    game.board().forEach((row, r) => {
      row.forEach((cell, c) => {
        if (cell) {
          const p = cell.type;
          const color = cell.color === 'w' ? 'upper' : 'lower';
          board[r][c] = color === 'upper' ? p.toUpperCase() : p.toLowerCase();
        }
      });
    });
    return board;
  };

  const restartGame = () => {
    const color = window.confirm("Click OK for White, Cancel for Black") ? 'w' : 'b';
    setUserColor(color);
    setGame(new Chess());
    setSelected(null);
    setValidMoves([]);

    // If user is black, engine (white) moves first
    if (color === 'b') {
      triggerEngineMove(new Chess());
    }
  };

  const triggerEngineMove = async (currentGame) => {
    setIsEngineThinking(true);
    try {
      const fen = currentGame.fen();
      const bestMove = await getBestMove(fen);

      // C engine returns "fromRow,fromCol,toRow,toCol" (0-7)
      // chess.js expects "e2e4"
      const files = ['a', 'b', 'c', 'd', 'e', 'f', 'g', 'h'];
      const fromFile = files[bestMove.fromCol];
      const fromRank = 8 - bestMove.fromRow;
      const toFile = files[bestMove.toCol];
      const toRank = 8 - bestMove.toRow;
      const notation = `${fromFile}${fromRank}${toFile}${toRank}`;

      const newGame = new Chess(currentGame.fen());
      newGame.move(notation);
      setGame(newGame);
    } catch (e) {
      console.error("Engine move failed:", e);
    } finally {
      setIsEngineThinking(false);
    }
  };

  const handleClick = async (r, c) => {
    if (isEngineThinking || !userColor) return;

    const square = `${filesToCoord(c)}${8 - r}`;

    // If a piece is already selected, try to move it
    if (selected) {
      try {
        const move = game.move({
          from: selected,
          to: square,
          promotion: 'q' // always promote to queen for simplicity
        });

        if (move) {
          setGame(new Chess(game.fen()));
          setSelected(null);
          setValidMoves([]);

          // It's now engine's turn
          if (game.turn() !== userColor) {
            await triggerEngineMove(game);
          }
          return;
        }
      } catch (e) {
        // invalid move
      }
    }

    // Selecting a piece
    const piece = game.get(square);
    if (piece && piece.color === userColor) {
      setSelected(square);
      const moves = game.moves({ square });
      setValidMoves(moves);
    } else {
      setSelected(null);
      setValidMoves([]);
    }
  };

  const filesToCoord = (col) => ['a', 'b', 'c', 'd', 'e', 'f', 'g', 'h'][col];

  const isSquareValidMove = (r, c) => {
    const square = `${filesToCoord(c)}${8 - r}`;
    return validMoves.some(m => m.endsWith(square));
  };

  if (!userColor) {
    return (
      <div style={{ textAlign: 'center', marginTop: '20px' }}>
        <button onClick={restartGame} style={{ padding: '10px 20px', fontSize: '16px' }}>
          Start Game (Choose Color)
        </button>
      </div>
    );
  }

  const currentBoard = renderBoard();

  return (
    <div style={{ display: 'flex', flexDirection: 'column', alignItems: 'center', gap: '20px' }}>
      <div style={{ display: 'flex', gap: '10px' }}>
        <button onClick={restartGame}>Restart Game</button>
        <span>{isEngineThinking ? "Engine is thinking..." : game.turn() === userColor ? "Your turn" : "Engine's turn"}</span>
      </div>
      <div className="board">
        {currentBoard.map((row, i) =>
          row.map((piece, j) => {
            const isWhiteSquare = (i + j) % 2 === 0;
            const squareNotation = `${filesToCoord(j)}${8 - i}`;
            const isSelected = selected === squareNotation;
            const isMove = isSquareValidMove(i, j);

            return (
              <Square
                key={`${i}-${j}`}
                piece={piece}
                isWhite={isWhiteSquare}
                isSelected={isSelected}
                isMove={isMove}
                onClick={() => handleClick(i, j)}
              />
            );
          })
        )}
      </div>
    </div>
  );
}