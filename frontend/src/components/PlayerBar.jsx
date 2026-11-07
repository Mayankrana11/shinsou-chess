import { useMemo } from "react";

const PIECE_VALUES = {
  p: 1,
  n: 3,
  b: 3,
  r: 5,
  q: 9,
};

const PIECE_NAMES = {
  p: "Pawn",
  n: "Knight",
  b: "Bishop",
  r: "Rook",
  q: "Queen",
};

const PIECE_SORT_ORDER = {
  p: 1,
  n: 2,
  b: 3,
  r: 4,
  q: 5,
};

export default function PlayerBar({
  side, // 'w' | 'b'
  isUser,
  isCurrentTurn,
  isThinking,
  capturedPieces = [],
  advantage = 0,
}) {
  const isWhite = side === "w";
  const opponentColorPrefix = isWhite ? "b" : "w"; // Pieces captured from opponent

  // Sort captured pieces: Pawns -> Knights -> Bishops -> Rooks -> Queens
  const sortedCaptures = useMemo(() => {
    return [...capturedPieces].sort(
      (a, b) => (PIECE_SORT_ORDER[a] || 0) - (PIECE_SORT_ORDER[b] || 0)
    );
  }, [capturedPieces]);

  const playerName = isUser
    ? `You (${isWhite ? "White" : "Black"})`
    : `Shinsou Engine (${isWhite ? "White" : "Black"})`;

  return (
    <div className={`player-bar ${isCurrentTurn ? "active-turn" : ""}`}>
      {/* Left: Player Identity */}
      <div className="player-info">
        <div className={`player-avatar ${isWhite ? "avatar-white" : "avatar-black"}`}>
          <img
            src={`/pieces/${isWhite ? "wK" : "bK"}.svg`}
            alt={isWhite ? "White King" : "Black King"}
            className="avatar-piece-img"
          />
        </div>
        <div className="player-details">
          <div className="player-name-row">
            <span className="player-name">{playerName}</span>
            {!isUser && <span className="engine-chip">C11 WASM</span>}
          </div>
          {isThinking ? (
            <span className="player-subtext thinking">
              <span className="pulse-spinner" /> Thinking...
            </span>
          ) : isCurrentTurn ? (
            <span className="player-subtext to-move">To move</span>
          ) : null}
        </div>
      </div>

      {/* Right: Captured Pieces & Material Score */}
      <div className="player-captures-section">
        {sortedCaptures.length > 0 && (
          <div className="captured-pieces-list" title="Captured pieces won by this player">
            {sortedCaptures.map((type, idx) => {
              const svgSrc = `/pieces/${opponentColorPrefix}${type.toUpperCase()}.svg`;
              const value = PIECE_VALUES[type] || 0;
              const name = PIECE_NAMES[type] || "Piece";
              return (
                <img
                  key={`${type}-${idx}`}
                  src={svgSrc}
                  alt={`${name} (+${value})`}
                  title={`${name} (+${value})`}
                  className="captured-piece-img"
                  draggable={false}
                />
              );
            })}
          </div>
        )}

        {/* Material Advantage Badge (e.g. +1, +5) */}
        {advantage > 0 && (
          <span
            className="material-advantage-badge"
            title={`Material advantage: +${advantage}`}
          >
            +{advantage}
          </span>
        )}
      </div>
    </div>
  );
}
