import Piece from "./Piece";

export default function Square({ piece, isWhite }) {
  return (
    <div className={`square ${isWhite ? "white" : "black"}`}>
      {piece && <Piece type={piece} />}
    </div>
  );
}