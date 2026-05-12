#include <stdio.h>
#include "board.h"

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

void movePiece(Board *board, Movement movement)
{
    (void)board;

    printf(
        "movePiece called from row %d col %d to row %d col %d\n",
        movement.from.row,
        movement.from.col,
        movement.to.row,
        movement.to.col
    );
}