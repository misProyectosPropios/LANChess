#ifndef LANCHESS_BOARD_H
#define LANCHESS_BOARD_H

#define LANCHESS_BOARD_SIZE 8

#include <stdbool.h>

typedef enum {
    PIECE_NONE = 0,
    PIECE_PAWN,
    PIECE_ROOK,
    PIECE_KNIGHT,
    PIECE_BISHOP,
    PIECE_QUEEN,
    PIECE_KING
} PieceType;

typedef enum {
    COLOR_NONE = 0,
    COLOR_WHITE,
    COLOR_BLACK
} PieceColor;

typedef struct {
    PieceType type;
    PieceColor color;
} Piece;

typedef struct {
    Piece squares[LANCHESS_BOARD_SIZE][LANCHESS_BOARD_SIZE];
} Board;

typedef struct {
    int row;
    int col;
} BoardPosition;

typedef struct {
    BoardPosition from;
    BoardPosition to;
} Movement;


#include "ArrayList.h"

DECLARE_ARRAYLIST(BoardPosition, BoardPosition)
DECLARE_ARRAYLIST(Movement, Movement)

typedef struct {
    ArrayList_BoardPosition* relative; // ArrayList of BoardPosition
} RelativePosition;

typedef struct {
    ArrayList_BoardPosition* possiblePositions; // ArrayList of BoardPosition
} PossiblePositions;

typedef struct {
    ArrayList_Movement* possibleMovements; // ArrayList of Movement
} PossibleMovements;

void initializeClassicGame(Board *board);
PossiblePositions movements(Board *board, BoardPosition position);
void movePiece(Board *board, Movement movement);
void move(Board *board, Movement movement);
void printBoard(Board *board);
bool is_in_check(Board *board, PieceColor color);
bool is_checkmate(Board *board, PieceColor color);
void possiblePosition(PossiblePositions* movements, RelativePosition* relative, BoardPosition position);

#endif
