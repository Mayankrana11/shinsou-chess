const pieceMap = {
  P: "♙", R: "♖", N: "♘", B: "♗", Q: "♕", K: "♔",
  p: "♟", r: "♜", n: "♞", b: "♝", q: "♛", k: "♚",
};

export default function Piece({ type }) {
  const isWhite = type === type.toUpperCase();

  return (
    <span className={`piece ${isWhite ? "white-piece" : "black-piece"}`}>
      {pieceMap[type]}
    </span>
  );
}