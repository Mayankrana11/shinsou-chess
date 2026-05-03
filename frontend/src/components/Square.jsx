import Piece from "./Piece";

export default function Square({ piece, isWhite, isSelected, isMove, onClick }) {
  return (
    <div
      className={`square ${isWhite ? "white" : "black"} 
      ${isSelected ? "selected" : ""} 
      ${isMove ? "move" : ""}`}
      onClick={onClick}
    >
      {piece && <Piece type={piece} />}
    </div>
  );
}