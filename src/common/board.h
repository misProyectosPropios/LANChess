#ifndef LANCHESS_BOARD_H
#define LANCHESS_BOARD_H

#define LANCHESS_BOARD_SIZE 8

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
    BoardPosition* relative;
    int length;
} RelativePosition;


typedef struct {
    BoardPosition from;
    BoardPosition to;
} Movement;

typedef struct {
    int length;
    BoardPosition *possiblePositions;
} PossiblePositions;

typedef struct {
    int length;
    Movement *possibleMovements;
} PossibleMovements;

void initializeClassicGame(Board *board);
PossiblePositions movements(Board *board, BoardPosition position);
void movePiece(Board *board, Movement movement);

void possiblePosition(PossiblePositions* movements, RelativePosition* relative, BoardPosition position);

#endif
