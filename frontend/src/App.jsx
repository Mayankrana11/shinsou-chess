import ChessBoard from "./components/ChessBoard";

export default function App() {
  return (
    <div className="app-shell">
      <header className="app-header">
        <div className="brand-title-row">
          <h1 className="brand-title">Shinsou</h1>
          <span className="brand-kanji">深層</span>
        </div>
        <p className="brand-subtitle">C11 WebAssembly Chess Engine</p>
      </header>
      <main className="app-main">
        <ChessBoard />
      </main>
    </div>
  );
}
