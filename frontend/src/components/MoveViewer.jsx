import { useEffect, useRef, useMemo } from "react";

export default function MoveViewer({
  history = [],
  viewedPly = null,
  onSelectPly,
  isEngineThinking = false,
  isGameOver = false,
  onExportPGN,
  copiedPgn = false,
}) {
  const activeChipRef = useRef(null);
  const totalPlies = history.length;
  const isSpectating = viewedPly !== null && viewedPly < totalPlies;
  const currentPly = viewedPly !== null ? viewedPly : totalPlies;

  // Group moves into turns: [ { num: 1, white: { san, ply }, black: { san, ply } } ]
  const movePairs = useMemo(() => {
    const pairs = [];
    for (let i = 0; i < history.length; i += 2) {
      pairs.push({
        num: Math.floor(i / 2) + 1,
        white: {
          san: history[i].san,
          ply: i + 1,
        },
        black: history[i + 1]
          ? {
              san: history[i + 1].san,
              ply: i + 2,
            }
          : null,
      });
    }
    return pairs;
  }, [history]);

  // Auto-scroll active move chip into view
  useEffect(() => {
    if (activeChipRef.current) {
      activeChipRef.current.scrollIntoView({
        behavior: "smooth",
        block: "nearest",
        inline: "center",
      });
    }
  }, [currentPly]);

  const handleFirst = () => {
    if (currentPly > 0 && !isEngineThinking) {
      onSelectPly(0);
    }
  };

  const handlePrev = () => {
    if (currentPly > 0 && !isEngineThinking) {
      onSelectPly(currentPly - 1);
    }
  };

  const handleNext = () => {
    if (isSpectating && !isEngineThinking) {
      if (currentPly + 1 >= totalPlies) {
        onSelectPly(null); // return to live
      } else {
        onSelectPly(currentPly + 1);
      }
    }
  };

  const handleLast = () => {
    if (isSpectating && !isEngineThinking) {
      onSelectPly(null);
    }
  };

  // Keyboard navigation
  useEffect(() => {
    const handleKeyDown = (e) => {
      if (
        e.target.tagName === "INPUT" ||
        e.target.tagName === "TEXTAREA" ||
        e.target.isContentEditable
      ) {
        return;
      }
      if (isEngineThinking) return;

      if (e.key === "ArrowLeft") {
        e.preventDefault();
        const cur = viewedPly !== null ? viewedPly : totalPlies;
        if (cur > 0) {
          onSelectPly(cur - 1);
        }
      } else if (e.key === "ArrowRight") {
        e.preventDefault();
        if (viewedPly !== null) {
          if (viewedPly + 1 >= totalPlies) {
            onSelectPly(null);
          } else {
            onSelectPly(viewedPly + 1);
          }
        }
      }
    };

    window.addEventListener("keydown", handleKeyDown);
    return () => window.removeEventListener("keydown", handleKeyDown);
  }, [viewedPly, totalPlies, isEngineThinking, onSelectPly]);

  return (
    <div className="move-viewer-panel">
      {/* Navigation Controls Bar */}
      <div className="viewer-controls-bar">
        <div className="nav-buttons-group">
          <button
            type="button"
            className="nav-btn"
            onClick={handleFirst}
            disabled={currentPly === 0 || totalPlies === 0 || isEngineThinking}
            title="Start Position (First move)"
          >
            ⏮
          </button>
          <button
            type="button"
            className="nav-btn"
            onClick={handlePrev}
            disabled={currentPly === 0 || totalPlies === 0 || isEngineThinking}
            title="Previous Move (← Arrow Key)"
          >
            ◀
          </button>
          <button
            type="button"
            className="nav-btn"
            onClick={handleNext}
            disabled={!isSpectating || isEngineThinking}
            title="Next Move (→ Arrow Key)"
          >
            ▶
          </button>
          <button
            type="button"
            className="nav-btn"
            onClick={handleLast}
            disabled={!isSpectating || isEngineThinking}
            title="Jump to Live Position"
          >
            ⏭
          </button>
        </div>

        {/* Center Indicator */}
        <div className="viewer-status-indicator">
          {totalPlies === 0 ? (
            <span className="indicator-muted">Game has not started</span>
          ) : isSpectating ? (
            <div className="indicator-spectating">
              <span className="spectate-badge">SPECTATE</span>
              <span className="indicator-text">
                {currentPly === 0
                  ? "Start Position"
                  : `Move ${Math.ceil(currentPly / 2)} (${
                      history[currentPly - 1]?.san
                    }) • Ply ${currentPly}/${totalPlies}`}
              </span>
              <button
                type="button"
                className="live-jump-pill"
                onClick={() => onSelectPly(null)}
                title="Return to the latest move"
              >
                Return to Live
              </button>
            </div>
          ) : (
            <div className="indicator-live">
              <span className="live-pulse-dot" />
              <span className="indicator-text">
                Live Game • Move {Math.ceil(totalPlies / 2) || 1} ({totalPlies}{" "}
                plies)
              </span>
            </div>
          )}
        </div>

        {/* Export PGN Button if game is over or in progress */}
        {totalPlies > 0 && (
          <button
            type="button"
            className={`action-btn export-btn ${isGameOver ? "prominent" : ""} ${
              copiedPgn ? "copied" : ""
            }`}
            onClick={onExportPGN}
            title="Export standard PGN to clipboard for analysis on Lichess / Chess.com"
          >
            {copiedPgn ? "✓ PGN Copied!" : "📋 Export PGN"}
          </button>
        )}
      </div>

      {/* Clickable Move History Strip */}
      {totalPlies > 0 && (
        <div className="move-history-strip">
          <div className="move-history-scroll">
            <button
              type="button"
              className={`start-chip ${currentPly === 0 ? "active" : ""}`}
              onClick={() => onSelectPly(0)}
              title="Initial board position"
            >
              Start
            </button>
            {movePairs.map((pair) => {
              const isWhiteActive = currentPly === pair.white.ply;
              const isBlackActive = pair.black && currentPly === pair.black.ply;

              return (
                <div key={pair.num} className="turn-pair">
                  <span className="turn-num">{pair.num}.</span>
                  <button
                    ref={isWhiteActive ? activeChipRef : null}
                    type="button"
                    className={`move-chip ${isWhiteActive ? "active" : ""}`}
                    onClick={() => onSelectPly(pair.white.ply)}
                  >
                    {pair.white.san}
                  </button>
                  {pair.black && (
                    <button
                      ref={isBlackActive ? activeChipRef : null}
                      type="button"
                      className={`move-chip ${isBlackActive ? "active" : ""}`}
                      onClick={() => onSelectPly(pair.black.ply)}
                    >
                      {pair.black.san}
                    </button>
                  )}
                </div>
              );
            })}
          </div>
        </div>
      )}
    </div>
  );
}
