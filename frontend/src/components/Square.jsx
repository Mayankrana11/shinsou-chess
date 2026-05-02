import Piece from "./Piece";

export default function Square({ piece, isWhite, isSelected, onClick }) {
  return (
    <div
      className={`square ${isWhite ? "white" : "black"} ${
        isSelected ? "selected" : ""
      }`}
      onClick={onClick}
    >
      {piece && <Piece type={piece} />}
    </div>
  );
}