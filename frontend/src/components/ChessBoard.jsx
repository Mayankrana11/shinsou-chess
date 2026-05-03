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
  const [moves, setMoves] = useState([]);

  function isWhite(p) {
    return p && p === p.toUpperCase();
  }

  function inBounds(r, c) {
    return r >= 0 && r < 8 && c >= 0 && c < 8;
  }

  function generateMoves(r, c) {
    const piece = board[r][c];
    if (!piece) return [];

    const result = [];
    const white = isWhite(piece);

    // ♙ Pawn
    if (piece.toLowerCase() === "p") {
    const dir = white ? -1 : 1;
    const startRow = white ? 6 : 1;

    // 1 step forward
    if (inBounds(r + dir, c) && !board[r + dir][c]) {
        result.push([r + dir, c]);

        // 2 steps from starting position
        if (r === startRow && !board[r + 2 * dir][c]) {
        result.push([r + 2 * dir, c]);
        }
    }

    // diagonal captures
    for (let dc of [-1, 1]) {
        let nr = r + dir;
        let nc = c + dc;

        if (
        inBounds(nr, nc) &&
        board[nr][nc] &&
        isWhite(board[nr][nc]) !== white
        ) {
        result.push([nr, nc]);
        }
    }
    }

    // ♞ Knight
    if (piece.toLowerCase() === "n") {
      const dirs = [
        [2,1],[2,-1],[-2,1],[-2,-1],
        [1,2],[1,-2],[-1,2],[-1,-2]
      ];

      for (let [dr, dc] of dirs) {
        let nr = r + dr;
        let nc = c + dc;

        if (!inBounds(nr, nc)) continue;

        if (!board[nr][nc] || isWhite(board[nr][nc]) !== white) {
          result.push([nr, nc]);
        }
      }
    }

    // ♜ ♝ ♛ sliding pieces
    const directions = {
      r: [[1,0],[-1,0],[0,1],[0,-1]],
      b: [[1,1],[1,-1],[-1,1],[-1,-1]],
      q: [[1,0],[-1,0],[0,1],[0,-1],[1,1],[1,-1],[-1,1],[-1,-1]]
    };

    const type = piece.toLowerCase();

    if (directions[type]) {
      for (let [dr, dc] of directions[type]) {
        let nr = r + dr;
        let nc = c + dc;

        while (inBounds(nr, nc)) {
          if (!board[nr][nc]) {
            result.push([nr, nc]);
          } else {
            if (isWhite(board[nr][nc]) !== white) {
              result.push([nr, nc]);
            }
            break;
          }

          nr += dr;
          nc += dc;
        }
      }
    }

    // ♚ King
    if (type === "k") {
      for (let dr = -1; dr <= 1; dr++) {
        for (let dc = -1; dc <= 1; dc++) {
          if (dr === 0 && dc === 0) continue;

          let nr = r + dr;
          let nc = c + dc;

          if (!inBounds(nr, nc)) continue;

          if (!board[nr][nc] || isWhite(board[nr][nc]) !== white) {
            result.push([nr, nc]);
          }
        }
      }
    }

    return result;
  }

  function handleClick(r, c) {
    const piece = board[r][c];

    // selecting piece
    if (!selected) {
      if (piece) {
        setSelected({ r, c });
        setMoves(generateMoves(r, c));
      }
      return;
    }

    // check if clicked move is valid
    const valid = moves.some(([mr, mc]) => mr === r && mc === c);

    if (valid) {
      const newBoard = board.map(row => [...row]);

      newBoard[r][c] = board[selected.r][selected.c];
      newBoard[selected.r][selected.c] = null;

      setBoard(newBoard);
    }

    setSelected(null);
    setMoves([]);
  }

  return (
    <div className="board">
      {board.map((row, i) =>
        row.map((piece, j) => {
          const isWhiteSquare = (i + j) % 2 === 0;

          const isSelected =
            selected && selected.r === i && selected.c === j;

          const isMove = moves.some(([r, c]) => r === i && c === j);

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
  );
}