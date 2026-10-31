let createEngineModule = null;

const loadModule = async () => {
    if (createEngineModule) return createEngineModule;

    // Dynamically load the script from the public folder
    return new Promise((resolve, reject) => {
        const script = document.createElement('script');
        script.src = '/engine/engine.js';
        script.onload = () => {
            // Emscripten MODULARIZE=1 puts the factory function on the window
            // using the EXPORT_NAME specified in the Makefile
            if (window.createEngineModule) {
                createEngineModule = window.createEngineModule;
                resolve(createEngineModule);
            } else {
                reject(new Error("createEngineModule not found on window after loading engine.js"));
            }
        };
        script.onerror = reject;
        document.head.appendChild(script);
    });
};

let engine = null;

/**
 * Initializes the WASM engine module.
 */
export const initEngine = async () => {
    if (!engine) {
        try {
            const moduleFactory = await loadModule();
            engine = await moduleFactory();
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
    const moveStr = mod.ccall('get_best_move', 'string', ['string'], [fen]);

    // The C engine returns "fromRow,fromCol,toRow,toCol"
    const [fromRow, fromCol, toRow, toCol] = moveStr.split(',').map(Number);

    return { fromRow, fromCol, toRow, toCol };
};
