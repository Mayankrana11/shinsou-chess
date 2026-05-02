import Square from "./Square";
import "../styles/board.css";

// Initial board (simple starting position)
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
  return (
    <div className="board">
      {initialBoard.map((row, i) =>
        row.map((piece, j) => {
          const isWhite = (i + j) % 2 === 0;

          return (
            <Square
              key={`${i}-${j}`}
              piece={piece}
              isWhite={isWhite}
            />
          );
        })
      )}
    </div>
  );
}