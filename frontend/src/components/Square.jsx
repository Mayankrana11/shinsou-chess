import Piece from "./Piece";

export default function Square({
  piece,
  isWhite,
  isSelected,
  isMove,
  isCapture,
  isLastMove,
  isCheck,
  rankLabel,
  fileLabel,
  onClick,
}) {
  const squareClasses = [
    "square",
    isWhite ? "white" : "black",
    isSelected ? "selected" : "",
    isLastMove ? "last-move" : "",
    isCheck ? "in-check" : "",
    isMove ? "move-target" : "",
  ]
    .filter(Boolean)
    .join(" ");

  return (
    <div className={squareClasses} onClick={onClick}>
      {rankLabel && <span className="coord-label coord-rank">{rankLabel}</span>}
      {piece && <Piece type={piece} />}
      {isMove && (isCapture || piece ? (
        <span className="capture-ring" />
      ) : (
        <span className="move-dot" />
      ))}
      {fileLabel && <span className="coord-label coord-file">{fileLabel}</span>}
    </div>
  );
}
