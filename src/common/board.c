#include "board.h"

#include <stdio.h>

static Piece empty_piece(void)
{
    Piece piece = { PIECE_NONE, COLOR_NONE };
    return piece;
}

static Piece make_piece(PieceType type, PieceColor color)
{
    Piece piece = { type, color };
    return piece;
}

static void initialize_back_rank(Board *board, int row, PieceColor color)
{
    board->squares[row][0] = make_piece(PIECE_ROOK, color);
    board->squares[row][1] = make_piece(PIECE_KNIGHT, color);
    board->squares[row][2] = make_piece(PIECE_BISHOP, color);
    board->squares[row][3] = make_piece(PIECE_QUEEN, color);
    board->squares[row][4] = make_piece(PIECE_KING, color);
    board->squares[row][5] = make_piece(PIECE_BISHOP, color);
    board->squares[row][6] = make_piece(PIECE_KNIGHT, color);
    board->squares[row][7] = make_piece(PIECE_ROOK, color);
}

void initializeClassicGame(Board *board)
{
    if (board == 0) {
        return;
    }

    for (int row = 0; row < LANCHESS_BOARD_SIZE; row++) {
        for (int col = 0; col < LANCHESS_BOARD_SIZE; col++) {
            board->squares[row][col] = empty_piece();
        }
    }

    initialize_back_rank(board, 0, COLOR_BLACK);
    initialize_back_rank(board, 7, COLOR_WHITE);

    for (int col = 0; col < LANCHESS_BOARD_SIZE; col++) {
        board->squares[1][col] = make_piece(PIECE_PAWN, COLOR_BLACK);
        board->squares[6][col] = make_piece(PIECE_PAWN, COLOR_WHITE);
    }
}


//Function that returns a set of all the possible movmenets of a given piece
//It uses an scrupt PossibleMovements that it's: int length, Movements possibleMovements*
void movements(BoardPosition position) {
    // TODO: Implement logic to calculate possible movements for a piece at the given position
    // This will involve checking the piece type and color, then scanning valid board squares.
    
}

void move(Board *board, Movement movement) {
    //Add movement verificaiton that there exists an already piece in board


    //Add movement verification that the piece can move in the way correctly


    //Add movement verificaiton that there no any other piece could capture the 

    switch (board->squares[movement.from.row][movement.from.col].type) {
    case PIECE_PAWN:
    case PIECE_ROOK:
    case PIECE_KNIGHT:
    case PIECE_BISHOP:
    case PIECE_QUEEN:
    case PIECE_KING:
        movePiece(board, movement);
        break;
    case PIECE_NONE:
    default:
        break;
    }
}

void movePiece(Board *board, Movement movement)
{
    //Set to a function printMovement(movement, is_debug) separetad that with a flag is_debug print it or not
    printf(
        "movePiece called from row %d col %d to row %d col %d\n",
        movement.from.row,
        movement.from.col,
        movement.to.row,
        movement.to.col
    );

    
    board->squares[movement.to.row][movement.to.col] =
        board->squares[movement.from.row][movement.from.col];
    board->squares[movement.from.row][movement.from.col] = empty_piece();
}
