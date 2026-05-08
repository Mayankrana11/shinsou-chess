#include <stdio.h>

#include "board.h"
#include "../utils/constants.h"

char pieceChar(int p) {

    switch(p) {

        case WPAWN: return 'P';
        case WKNIGHT: return 'N';
        case WBISHOP: return 'B';
        case WROOK: return 'R';
        case WQUEEN: return 'Q';
        case WKING: return 'K';

        case BPAWN: return 'p';
        case BKNIGHT: return 'n';
        case BBISHOP: return 'b';
        case BROOK: return 'r';
        case BQUEEN: return 'q';
        case BKING: return 'k';

        default: return '.';
    }
}

void initBoard(Position* pos) {

    int start[8][8] = {

        {BROOK,BKNIGHT,BBISHOP,BQUEEN,BKING,BBISHOP,BKNIGHT,BROOK},
        {BPAWN,BPAWN,BPAWN,BPAWN,BPAWN,BPAWN,BPAWN,BPAWN},

        {0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0},

        {WPAWN,WPAWN,WPAWN,WPAWN,WPAWN,WPAWN,WPAWN,WPAWN},
        {WROOK,WKNIGHT,WBISHOP,WQUEEN,WKING,WBISHOP,WKNIGHT,WROOK}
    };

    for(int i=0;i<8;i++) {

        for(int j=0;j<8;j++) {

            pos->board[i][j] = start[i][j];
        }
    }

    pos->sideToMove = WHITE;
}

void printBoard(Position* pos) {

    printf("\n");

    for(int i=0;i<8;i++) {

        printf("%d  ", 8 - i);

        for(int j=0;j<8;j++) {

            printf("%c ", pieceChar(pos->board[i][j]));
        }

        printf("\n");
    }

    printf("\n   a b c d e f g h\n\n");
}