import { useState } from "react";
import Square from "./Square";
import "../styles/board.css";

const initialBoard = [
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
  const [board, setBoard] = useState(initialBoard);
  const [selected, setSelected] = useState(null);

  function isWhitePiece(p) {
    return p && p === p.toUpperCase();
  }

  function handleClick(row, col) {
    const piece = board[row][col];

    // 1. Select piece
    if (!selected) {
      if (piece) {
        setSelected({ row, col });
      }
      return;
    }

    const from = selected;
    const to = { row, col };

    const fromPiece = board[from.row][from.col];
    const toPiece = board[to.row][to.col];

    // 2. Prevent capturing own piece
    if (toPiece && isWhitePiece(toPiece) === isWhitePiece(fromPiece)) {
      setSelected({ row, col }); // switch selection
      return;
    }

    // 3. Move piece (no legality yet)
    const newBoard = board.map(r => [...r]);

    newBoard[to.row][to.col] = fromPiece;
    newBoard[from.row][from.col] = null;

    setBoard(newBoard);
    setSelected(null);
  }

  return (
    <div className="board">
      {board.map((row, i) =>
        row.map((piece, j) => {
          const isWhite = (i + j) % 2 === 0;

          const isSelected =
            selected && selected.row === i && selected.col === j;

          return (
            <Square
              key={`${i}-${j}`}
              piece={piece}
              isWhite={isWhite}
              isSelected={isSelected}
              onClick={() => handleClick(i, j)}
            />
          );
        })
      )}
    </div>
  );
}