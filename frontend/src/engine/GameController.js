import { getBestMove } from './EngineBridge';

export const useChessGame = (gameInstance) => {
    const handleEngineTurn = async (onMoveExecuted) => {
        try {
            const fen = gameInstance.fen();
            const bestMove = await getBestMove(fen);

            // Convert coordinates (0-7) to chess.js notation (e.g., e2e4)
            const moveNotation = coordsToNotation(bestMove);
            onMoveExecuted(moveNotation);
        } catch (error) {
            console.error("Engine Error:", error);
        }
    };

    const coordsToNotation = (move) => {
        const files = ['a', 'b', 'c', 'd', 'e', 'f', 'g', 'h'];
        // C engine uses 0-7 rows (0=top, 7=bottom), chess.js uses 1-8
        const fromFile = files[move.fromCol];
        const fromRank = 8 - move.fromRow;
        const toFile = files[move.toCol];
        const toRank = 8 - move.toRow;
        return `${fromFile}${fromRank}${toFile}${toRank}`;
    };

    return {
        handleEngineTurn,
        coordsToNotation
    };
};
