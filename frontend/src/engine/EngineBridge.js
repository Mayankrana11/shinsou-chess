import createEngineModule from '../../public/engine/engine.js';

let engine = null;

/**
 * Initializes the WASM engine module.
 */
export const initEngine = async () => {
    if (!engine) {
        try {
            engine = await createEngineModule();
        } catch (e) {
            console.error("Failed to initialize WASM engine:", e);
        }
    }
    return engine;
};

/**
 * Calls the C engine to get the best move for a given FEN.
 * @param {string} fen - The current board state in FEN format.
 * @returns {Promise<{fromRow: number, fromCol: number, toRow: number, toCol: number}>}
 */
export const getBestMove = async (fen) => {
    const mod = await initEngine();
    if (!mod) throw new Error("Engine not initialized");

    // ccall(functionName, returnType, argTypes, args)
    // 'get_best_move' is the function name in wasm_bridge.c
    const moveStr = mod.ccall('get_best_move', 'string', ['string'], [fen]);

    // The C engine returns "fromRow,fromCol,toRow,toCol"
    const [fromRow, fromCol, toRow, toCol] = moveStr.split(',').map(Number);

    return { fromRow, fromCol, toRow, toCol };
};
