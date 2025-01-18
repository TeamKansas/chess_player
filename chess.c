#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <termios.h>
#include <time.h>

#include "pieces.h"

#define PAWN   1
#define ROOK   2
#define KNIGHT 3
#define BISHOP 4
#define QUEEN  5
#define KING   6

#define GHOST_PAWN 7
#define UNMOVED_KING 13
#define UNMOVED_ROOK 9

#define MOVE      0b00000001
#define CAPTURE   0b00000010
#define CHECK     0b00000100
#define CHECKMATE 0b00001000

#define MODE_SELECTMOVE  1
#define MODE_SELECTPIECE 0

#define oppositensign(A,B) ((A < 0 && B > 0) || (A > 0 && B < 0))
#define inrange(I) (I >= 0 && I < 64)

int8_t nsign(int8_t a) {
    return a < 0 ? -1 : (a > 0 ? 1 : 0);
}

int validMoves(int8_t board[64], int8_t piece, int8_t moves[30], int8_t *info) {
    int8_t type = abs(board[piece]);
    int8_t tmp;
    int8_t sign = nsign(board[piece]);
    int8_t psign;
    int8_t to;
    int8_t moveIndex = 0;
    int8_t boardIndex;
    int8_t row = piece / 8;
    int8_t col = piece % 8;

    if(type == PAWN) {
        boardIndex = piece + sign * 8; /* up one */
        /* move forward option */
        if(inrange(boardIndex) && !board[boardIndex]) { 
            moves[moveIndex++] = boardIndex;
            boardIndex += sign * 8;
            if(inrange(boardIndex) && !board[boardIndex] && boardIndex / 8 == (sign < 0 ? 4 : 3)) { /* optional second move forward if still on second row */
                *info |= MOVE;
                moves[moveIndex++] = boardIndex;
            }
        }

        /* captures */
        tmp = sign == 1 ? 7 : 9;
        if(col != 0 && (boardIndex = piece + sign * tmp) < 64 && boardIndex >= 0 && (psign = nsign(to = board[boardIndex])) == -sign ) { /* right capture */
            *info |= abs(board[boardIndex])%7 == KING ? CHECK : CAPTURE;
            moves[moveIndex++] = boardIndex;
        }
        tmp = sign == 1 ? 9 : 7;
        if(col != 7 && (boardIndex = piece + sign * tmp) < 64 && boardIndex >= 0 && (psign = nsign(to = board[boardIndex])) == -sign ) { /* left capture */
            *info |= abs(board[boardIndex])%7 == KING ? CHECK : CAPTURE;
            moves[moveIndex++] = boardIndex;
        }


    }
    if(type == ROOK || type == QUEEN || type == UNMOVED_ROOK) {
        int direction = 8;
        int psign;
        for(int d = 0; d < 4; ++d) {
            boardIndex = piece + direction;
            while(boardIndex >= 0 && boardIndex < 64 && ((d >= 2 && boardIndex / 8 == row) || (d < 2 && boardIndex%8 == col))) {
                if((psign = nsign(tmp = board[boardIndex]%7)) != sign || (type == UNMOVED_ROOK && psign == sign && abs(board[boardIndex]) == UNMOVED_KING) ) {
                    *info |= (psign == 0 || psign == sign) ? MOVE : (abs(tmp)%7 == KING ? CHECK : CAPTURE);
                    moves[moveIndex++] = boardIndex;
                }
                if(psign) { /* capture or blocked by piece of same color */
                    break;
                }
                boardIndex += direction;
            }
            direction = (direction < 0) ? 1 : -direction;
        }

    }
    if(type == KNIGHT) {

        if((boardIndex = piece + 6) < 64 && col > 1 && nsign(tmp = board[boardIndex]%7) != sign) {
            *info |= tmp == 0 ? MOVE : (abs(tmp)%7 == KING ? CHECK : CAPTURE);
            moves[moveIndex++] = boardIndex;
        }
        if((boardIndex = piece + 10) < 64 && col < 6 && nsign(tmp = board[boardIndex]%7) != sign) {
            *info |= tmp == 0 ? MOVE : (abs(tmp)%7 == KING ? CHECK : CAPTURE);
            moves[moveIndex++] = boardIndex;
        }
        if((boardIndex = piece + 15) < 64 && col > 0 && nsign(tmp = board[boardIndex]%7) != sign) {
            *info |= tmp == 0 ? MOVE : (abs(tmp)%7 == KING ? CHECK : CAPTURE);
            moves[moveIndex++] = boardIndex;
        }
        if((boardIndex = piece + 17) < 64 && col < 7 && nsign(tmp = board[boardIndex]%7) != sign) {
            *info |= tmp == 0 ? MOVE : (abs(tmp)%7 == KING ? CHECK : CAPTURE);
            moves[moveIndex++] = boardIndex;
        }

        if((boardIndex = piece - 6) >= 0 && col < 6 && nsign(tmp = board[boardIndex]%7) != sign) {
            *info |= tmp == 0 ? MOVE : (abs(tmp)%7 == KING ? CHECK : CAPTURE);
            moves[moveIndex++] = boardIndex;
        }
        if((boardIndex = piece - 10) >= 0 && col > 1 && nsign(tmp = board[boardIndex]%7) != sign) {
            *info |= tmp == 0 ? MOVE : (abs(tmp)%7 == KING ? CHECK : CAPTURE);
            moves[moveIndex++] = boardIndex;
        }
        if((boardIndex = piece - 15) >= 0 && col < 7 && nsign(tmp = board[boardIndex]%7) != sign) {
            *info |= tmp == 0 ? MOVE : (abs(tmp)%7 == KING ? CHECK : CAPTURE);
            moves[moveIndex++] = boardIndex;
        }
        if((boardIndex = piece - 17) >= 0 && col > 0 && nsign(tmp = board[boardIndex]%7) != sign) {
            *info |= tmp == 0 ? MOVE : (abs(tmp)%7 == KING ? CHECK : CAPTURE);
            moves[moveIndex++] = boardIndex;
        }
    }
    if(type == BISHOP || type == QUEEN) {
        int direction = 9;
        int repeat;
        int psign;
        for(int d = 0; d < 4; ++d) {
            repeat = (direction == -9 || direction == 7) ? col : 7-col;
            for(boardIndex = piece + direction; repeat > 0 && boardIndex >= 0 && boardIndex < 64; --repeat) {
                if((psign = nsign(tmp = board[boardIndex]%7)) != sign) {
                    *info |= tmp == 0 ? MOVE : (abs(tmp)%7 == KING ? CHECK : CAPTURE);
                    moves[moveIndex++] = boardIndex;
                }
                if(psign)
                    break;
                boardIndex += direction;
            }
            direction = (direction < 0) ? 7 : -direction;
        }
        // return moveIndex;

    }
    if(type == KING || type == UNMOVED_KING) {
        boardIndex = piece + 1;
        int8_t psign;
        if(col < 7 && (psign = nsign(tmp = board[boardIndex]%7)) != sign) {
            *info |= tmp == 0 ? MOVE : (abs(tmp)%7 == KING ? CHECK : CAPTURE);
            moves[moveIndex++] = boardIndex;
        }
        boardIndex = piece - 1;
        if(col > 0 && (psign = nsign(tmp = board[boardIndex]%7)) != sign) {
            *info |= tmp == 0 ? MOVE : (abs(tmp)%7 == KING ? CHECK : CAPTURE);
            moves[moveIndex++] = boardIndex;
        }
        boardIndex = piece + 8;
        if(row < 7 && (psign = nsign(tmp = board[boardIndex]%7)) != sign) {
            *info |= tmp == 0 ? MOVE : (abs(tmp)%7 == KING ? CHECK : CAPTURE);
            moves[moveIndex++] = boardIndex;
        }
        boardIndex = piece - 8;
        if(row > 0 && (psign = nsign(tmp = board[boardIndex]%7)) != sign) {
            *info |= tmp == 0 ? MOVE : (abs(tmp)%7 == KING ? CHECK : CAPTURE);
            moves[moveIndex++] = boardIndex;
        }

        boardIndex = piece + 9;
        if(row < 7 && col < 7 && (psign = nsign(tmp = board[boardIndex]%7)) != sign) {
            *info |= tmp == 0 ? MOVE : (abs(tmp)%7 == KING ? CHECK : CAPTURE);
            moves[moveIndex++] = boardIndex;
        }
        boardIndex = piece + 7;
        if(row < 7 && col > 0 && (psign = nsign(tmp = board[boardIndex]%7)) != sign) {
            *info |= tmp == 0 ? MOVE : (abs(tmp)%7 == KING ? CHECK : CAPTURE);
            moves[moveIndex++] = boardIndex;
        }
        boardIndex = piece - 7;
        if(row > 0 && col < 7 && (psign = nsign(tmp = board[boardIndex]%7)) != sign) {
            *info |= tmp == 0 ? MOVE : (abs(tmp)%7 == KING ? CHECK : CAPTURE);
            moves[moveIndex++] = boardIndex;
        }
        boardIndex = piece - 9;
        if(row > 0 && col > 0 && (psign = nsign(tmp = board[boardIndex]%7)) != sign) {
            *info |= tmp == 0 ? MOVE : (abs(tmp)%7 == KING ? CHECK : CAPTURE);
            moves[moveIndex++] = boardIndex;
        }

    }
    return moveIndex;
}

int check(int8_t board[64], int8_t team) { /* if team can put -team into check next move */
    int8_t moves[30];
    int8_t info = 0;
    for(int i = 0; i < 64; ++i) {
        if(nsign(board[i]) == team)
            validMoves(board, i, moves, &info);
    }
    return info;
}
int underAttack(int8_t board[64], int8_t team, int8_t at) {
    int8_t moves[30];
    int8_t info = 0;
    int8_t count;
    int8_t type;
    if(board[at] == 0)
        board[at] = -team * 127;
    for(int i = 0; i < 64; ++i) {
        if(nsign(board[i]) == team) {
            count = validMoves(board, i, moves, &info);
            for(--count; count >= 0; --count) {
                if(moves[count] == at)
                    return 1;
            }
        }
    }
    if(board[at] == -team * 127)
        board[at] = 0;
    return 0;
}

int8_t move(int8_t board[64], int8_t piece, int8_t to, int8_t state) {
    int8_t rowI = piece / 8;
    int8_t rowF = to / 8;
    int8_t type = board[piece];
    int8_t totype = board[to];
    int8_t sign = nsign(type);
    int8_t tosign = nsign(totype);
    if(abs(type) == PAWN && abs(rowI - rowF) == 2) { /* double forward (create ghost pawn) */
        board[piece + 8 * sign] = GHOST_PAWN * sign;
        board[to] = type;
        board[piece] = 0;
    }
    if(abs(type) == PAWN && abs(totype) == GHOST_PAWN) { /* en passant capture */
        board[to] = type;
        board[piece] = 0;
        board[to - 8 * sign] = 0;
    }
    else if(abs(type) == UNMOVED_ROOK && abs(totype) == UNMOVED_KING) { /* castle */
        if((state & CHECK) == 0) {
            if(piece > to && !underAttack(board, -sign, to + 1)) { /* king moves right, rook moves left */
                board[to] = 0;
                board[to + 2] = KING * sign;
                board[piece] = 0;
                board[to + 1] = ROOK * sign;
            }
            else if(piece < to && !underAttack(board, -sign, to - 1)) { /* king moves left, rook moves right */
                board[to] = 0;
                board[to - 2] = KING * sign;
                board[piece] = 0;
                board[to - 1] = ROOK * sign;
            }
            else
                return 0;
        }
        else
            return 0;
    }

    else {
        board[to] = type%7; /* mod 7 so that UNMOVED pieces become normal pieces */
        board[piece] = 0;
    }
    if(abs(type) == PAWN && rowF%7 == 0)
        board[to] = QUEEN * sign;

    /* ghost pawns only used for captures en passant, which only works immediately after the first pawn moves. Once another move is made, en passant does not work */
    /* this removes ghost pawns of the opposite team */
    for(int i = 0; i < 64; ++i) 
        if(board[i] == -GHOST_PAWN * sign)
            board[i] = 0;

    return 1;
}

int8_t checkLegality(int8_t board[64], int8_t team, int8_t piece, int8_t to[30]) {
    int8_t tmp[64];
    int8_t moveCount;
    int8_t legalMoves;
    int8_t info;
    int8_t state;
    if(nsign(board[piece]) != team)
        return 0;
    legalMoves = moveCount = validMoves(board, piece, to, &info);
    for(int i = moveCount - 1; i >= 0; --i) {
        memcpy(tmp, board, 64);
        state = check(board, -team);
        if(!move(tmp, piece, to[i], state) || (check(tmp, -team) & CHECK) == CHECK) {
            --legalMoves;
            for(int a = i; a < legalMoves; a++)
                to[a] = to[a+1];
        }
    }
    return legalMoves;
}

int makeMove(int8_t board[64], int8_t team, int8_t piece, int8_t to) {
    int8_t tmp[64];
    int8_t moves[30];
    int8_t moveCount;
    int8_t info;
    int8_t state;
    memcpy(tmp, board, 64);
    moveCount = checkLegality(board, team, piece, moves);
    for(moveCount -= 1; moveCount >= 0 && to != moves[moveCount]; --moveCount);
    state = check(board, -team);
    if(moveCount >= 0) {
        move(board, piece, to, state);
        return 1;
    }
    return 0;
}

void printBoard(int8_t board[64], int8_t *select, int count, int cursor, int8_t turn) {
    char c, pcolor, bcolor;
    int8_t index;
    int8_t piece;
    int8_t aR, aC;
    int8_t adder = turn < 0 ? 0 : 7;
    char string[9] = "     a   ";
    write(1, "\033[0m", 4);
    for(int l = 0; l < 8; ++l) {
        string[5] = 'a' + 7 - (adder - turn * l);
        write(1, string, 9);
    }
    printf("\n");
    for(int row = -1; row < 8; ++row) {
        for(int l = 0; row >= 0 && l < 4; ++l) {
            if(l == 2)
                write(1, "\033[0m", 4);
            c = l == 2 ? ('1' + (adder - turn * row)) : ' ';
            write(1, &c, 1);
            for(int col = -1; col < 8; ++col) {
                aC = (adder - turn * col);
                aR = (adder - turn * row);
                index = 8 * aR + aC;
                bcolor = (aR+aC+1)%2;
                
                if(col >= 0) {
                    piece = board[index]%7;
                    pcolor = piece < 0; /* positive is white */
                    if(bcolor == 0) /* black */
                        write(1,"\033[40m", 5);//write(1, "\033[48;5;52m", 10);
                    else if(bcolor != 0)
                        write(1,"\033[47m", 5);//write(1, "\033[48;5;216m", 11); /* white */
                    if(pcolor) /* dark pieces */
                        write(1, "\033[1;34m", 7);
                    else
                        write(1, "\033[1;31m", 7);
                    if(piece == 0)
                        write(1, "        ", 8);
                    else
                        write(1, pieces[abs(piece)][l], 8);
                }
                write(1, "\033[0m", 4);
                write(1, "\033[1;32m", 7);
                c = ' ';
                if((index == cursor && col >= 0) || (index - turn == cursor && col != 7))
                    c = '@';
                else
                    for(int s = 0; s < count; ++s) {
                        if((index == select[s] && col >= 0) || (index - turn == select[s] && col != 7)) {
                            c = '*';
                            break;
                        }
                    }
                write(1, &c, 1);
            }
            if(l == 2) {
                write(1, "\033[0m", 4);
                c = ('1' + (adder - turn * row));
                write(1, &c, 1);
            }
            printf("\n");
        }
        c = ' ';
        write(1, &c, 1);
        for(int col = 0; col < 8; ++col) {
            write(1, "\033[0m", 4);
            write(1, "\033[1;32m", 7);
            aC = (adder - turn * col);
            aR = (adder - turn * row);
            index = 8 * aR + aC;
            c = 0;
            if(index == cursor || cursor == index - 8 * turn)
                c = 2;
            else
                for(int s = 0; s < count; ++s) {
                    if(select[s] == index || select[s] == index - 8 * turn)
                        c = 1;
                }
            write(1, c ? (c == 1 ? " ********" : " @@@@@@@@") : "         ", 9);
        }
        printf("\n");
    }
    write(1, "\033[0m", 4);
    for(int l = 0; l < 8; ++l) {
        string[5] = 'a' + 7 - (adder - turn * l);
        write(1, string, 9);
    }
    printf("\033[0m\n");
}

void disableRaw() {
    struct termios t;
    tcgetattr(0,&t);
    t.c_lflag |= (ECHO|ICANON);
    tcsetattr(0,0,&t);
}

void enableRaw(int min) {
    struct termios t;
    tcgetattr(0,&t);
    t.c_cc[VTIME] = 0;
    t.c_cc[VMIN] = min;
    t.c_lflag &= ~(ECHO|ICANON);
    tcsetattr(0,0,&t);
}

void waitTime(clock_t d) {
    clock_t time = clock();
    while(clock() - time < d);
    return;
}

void printAll(int8_t board[64], int8_t *select, int cursor, int8_t turn, int mode, int allowedMoves) {
    printf("\033c\n");
    printf("Controls:\n");
    printf("w, s, a, d: Move cursor\n");
    printf("q: un-select piece\n");
    printf("<Space>: Select piece\n");
    printf("<Enter>: Move selected piece to cursor\n\n");
    printf("Allowed moves will only be highlighted for pieces that are currently in turn.\n\n");
    printf("Note on Castling: Castling is a Rook's move in this player. Select the rook, and then move the rook to the King to castle.");
    printf("\n___________________\n");
    printf("| Check \033[1;34mBlue\033[0m %s |\n", check(board, 1)&CHECK ? "YES" : "NO ");
    printf("| Check \033[1;31mRed\033[0m  %s |\n", check(board, -1)&CHECK ? "YES" : "NO ");
    printf("-------------------\n");

    printf("Turn: %s\n", turn < 0 ? "\033[1;34mBlue\033[0m" : "\033[1;31mRed\033[0m");

    printBoard(board, select, mode ? 1 : allowedMoves, cursor, turn);
}

int main() {
    /* board is started with normal pieces, but king and rook is started with UNMOVED king and rook */
    int8_t board[64] = {9,3,4,13,5,4,3,9, 1,1,1,1,1,1,1,1, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, -1,-1,-1,-1,-1,-1,-1,-1, -9,-3,-4,-13,-5,-4,-3,-9};
    // {9,0,0,0,13,0,0,9, 0,-1,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0};
    int8_t select[30];
    int count;
    char c;
    int cursor = 0;
    int piece = -1;
    int allowedMoves;
    int mode = MODE_SELECTPIECE;
    int8_t info;
    int8_t turn = 1;
    enableRaw(1);
    while(c != 'Q') {
        if(mode == MODE_SELECTPIECE) {
            allowedMoves = nsign(board[cursor]) == turn ? checkLegality(board, turn, cursor, select) : 0;
        }
        printAll(board, select, cursor, turn, mode, allowedMoves);
        read(0,&c,1);
        switch(c) {
            case 'w':
                cursor = (cursor + 56 + (turn + 1) * 8)%64;
                break;
            case 's':
                cursor = (cursor + 8 + (turn + 1) * 24)%64;
                break;
            case 'd':
                cursor = (cursor + 64 - turn)%64;
                break;
            case 'a':
                cursor = (cursor + 64 + turn)%64;
                break;
            case ' ':
                if(nsign(board[cursor]) == turn && mode == MODE_SELECTPIECE) {
                    piece = cursor;
                    mode = MODE_SELECTMOVE;
                    select[0] = cursor;
                }
                break;
            case 10: /* enter key */
                if(mode == MODE_SELECTMOVE && makeMove(board, turn, piece, cursor)) {
                    printAll(board, select, cursor, turn, mode, allowedMoves);
                    mode = MODE_SELECTPIECE;
                    waitTime(600000);
                    turn = -turn;
                }
                break;
            case 'q':
                if(mode == MODE_SELECTMOVE)
                    mode = MODE_SELECTPIECE;
                break;
        }
    }
    disableRaw();
}
