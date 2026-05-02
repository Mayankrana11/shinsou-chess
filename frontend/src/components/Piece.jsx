const pieceMap = {
  P: "wP",
  R: "wR",
  N: "wN",
  B: "wB",
  Q: "wQ",
  K: "wK",

  p: "bP",
  r: "bR",
  n: "bN",
  b: "bB",
  q: "bQ",
  k: "bK",
};

export default function Piece({ type }) {
  const src = `/pieces/${pieceMap[type]}.svg`;

  return (
    <img
      src={src}
      alt={type}
      className="piece-img"
      draggable={false}
    />
  );
}