#include "movegen.h"

#include "../utils/constants.h"

int isWhitePiece(int p) {
    return p > 0;
}

int isBlackPiece(int p) {
    return p < 0;
}

int sameColor(int a, int b) {

    if(a > 0 && b > 0) return 1;
    if(a < 0 && b < 0) return 1;

    return 0;
}

int inBounds(int r, int c) {

    return r >= 0 && r < 8 && c >= 0 && c < 8;
}

void addMove(
    Move moves[],
    int* count,
    int fr,
    int fc,
    int tr,
    int tc,
    int piece,
    int captured
) {

    moves[*count].fromRow = fr;
    moves[*count].fromCol = fc;

    moves[*count].toRow = tr;
    moves[*count].toCol = tc;

    moves[*count].piece = piece;
    moves[*count].captured = captured;

    (*count)++;
}

int generateMoves(Position* pos, Move moves[]) {

    int count = 0;

    for(int r=0;r<8;r++) {

        for(int c=0;c<8;c++) {

            int piece = pos->board[r][c];

            if(piece == EMPTY)
                continue;

            // side to move
            if(pos->sideToMove == WHITE && piece < 0)
                continue;

            if(pos->sideToMove == BLACK && piece > 0)
                continue;

            int type = piece > 0 ? piece : -piece;

            // =====================
            // PAWN
            // =====================

            if(type == WPAWN) {

                int dir = piece > 0 ? -1 : 1;

                int startRow = piece > 0 ? 6 : 1;

                // 1 step
                int nr = r + dir;

                if(inBounds(nr,c) &&
                   pos->board[nr][c] == EMPTY) {

                    addMove(
                        moves,
                        &count,
                        r,c,
                        nr,c,
                        piece,
                        EMPTY
                    );

                    // 2 step
                    if(r == startRow &&
                       pos->board[r + 2*dir][c] == EMPTY) {

                        addMove(
                            moves,
                            &count,
                            r,c,
                            r + 2*dir,c,
                            piece,
                            EMPTY
                        );
                    }
                }

                // captures
                for(int dc=-1; dc<=1; dc+=2) {

                    int nc = c + dc;

                    if(inBounds(nr,nc)) {

                        int target = pos->board[nr][nc];

                        if(target != EMPTY &&
                           !sameColor(piece,target)) {

                            addMove(
                                moves,
                                &count,
                                r,c,
                                nr,nc,
                                piece,
                                target
                            );
                        }
                    }
                }
            }

            // =====================
            // KNIGHT
            // =====================

            if(type == WKNIGHT) {

                int dirs[8][2] = {

                    {2,1},{2,-1},
                    {-2,1},{-2,-1},

                    {1,2},{1,-2},
                    {-1,2},{-1,-2}
                };

                for(int i=0;i<8;i++) {

                    int nr = r + dirs[i][0];
                    int nc = c + dirs[i][1];

                    if(!inBounds(nr,nc))
                        continue;

                    int target = pos->board[nr][nc];

                    if(target == EMPTY ||
                       !sameColor(piece,target)) {

                        addMove(
                            moves,
                            &count,
                            r,c,
                            nr,nc,
                            piece,
                            target
                        );
                    }
                }
            }

            // =====================
            // BISHOP / ROOK / QUEEN
            // =====================

            int bishopDirs[4][2] = {

                {1,1},
                {1,-1},
                {-1,1},
                {-1,-1}
            };

            int rookDirs[4][2] = {

                {1,0},
                {-1,0},
                {0,1},
                {0,-1}
            };

            if(type == WBISHOP ||
               type == WROOK ||
               type == WQUEEN) {

                int (*dirs)[2];
                int dirCount;

                if(type == WBISHOP) {
                    dirs = bishopDirs;
                    dirCount = 4;
                }

                else if(type == WROOK) {
                    dirs = rookDirs;
                    dirCount = 4;
                }

                else {

                    static int queenDirs[8][2] = {

                        {1,1},
                        {1,-1},
                        {-1,1},
                        {-1,-1},

                        {1,0},
                        {-1,0},
                        {0,1},
                        {0,-1}
                    };

                    dirs = queenDirs;
                    dirCount = 8;
                }

                for(int d=0; d<dirCount; d++) {

                    int dr = dirs[d][0];
                    int dc = dirs[d][1];

                    int nr = r + dr;
                    int nc = c + dc;

                    while(inBounds(nr,nc)) {

                        int target = pos->board[nr][nc];

                        if(target == EMPTY) {

                            addMove(
                                moves,
                                &count,
                                r,c,
                                nr,nc,
                                piece,
                                EMPTY
                            );
                        }

                        else {

                            if(!sameColor(piece,target)) {

                                addMove(
                                    moves,
                                    &count,
                                    r,c,
                                    nr,nc,
                                    piece,
                                    target
                                );
                            }

                            break;
                        }

                        nr += dr;
                        nc += dc;
                    }
                }
            }

            // =====================
            // KING
            // =====================

            if(type == WKING) {

                for(int dr=-1; dr<=1; dr++) {

                    for(int dc=-1; dc<=1; dc++) {

                        if(dr == 0 && dc == 0)
                            continue;

                        int nr = r + dr;
                        int nc = c + dc;

                        if(!inBounds(nr,nc))
                            continue;

                        int target = pos->board[nr][nc];

                        if(target == EMPTY ||
                           !sameColor(piece,target)) {

                            addMove(
                                moves,
                                &count,
                                r,c,
                                nr,nc,
                                piece,
                                target
                            );
                        }
                    }
                }
            }
        }
    }

    return count;
}