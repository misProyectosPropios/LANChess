#include <stdio.h>
#include <string.h>

#include "app_info.h"
#include "board.h"

static int expect_piece(Board *board, int row, int col, PieceType type, PieceColor color)
{
    Piece piece = board->squares[row][col];

    if (piece.type != type || piece.color != color) {
        fprintf(stderr, "unexpected piece at row %d col %d\n", row, col);
        return 1;
    }

    return 0;
}

int main(void)
{
    Board board;
    Movement movement = {
        { 6, 4 },
        { 4, 4 }
    };

    if (strcmp(lanchess_name(), "LANChess") != 0) {
        fprintf(stderr, "unexpected app name\n");
        return 1;
    }

    if (strcmp(lanchess_version(), "0.1.0") != 0) {
        fprintf(stderr, "unexpected app version\n");
        return 1;
    }

    initializeClassicGame(&board);

    if (expect_piece(&board, 0, 0, PIECE_ROOK, COLOR_BLACK) != 0) {
        return 1;
    }

    if (expect_piece(&board, 0, 4, PIECE_KING, COLOR_BLACK) != 0) {
        return 1;
    }

    if (expect_piece(&board, 1, 3, PIECE_PAWN, COLOR_BLACK) != 0) {
        return 1;
    }

    if (expect_piece(&board, 4, 4, PIECE_NONE, COLOR_NONE) != 0) {
        return 1;
    }

    if (expect_piece(&board, 6, 3, PIECE_PAWN, COLOR_WHITE) != 0) {
        return 1;
    }

    if (expect_piece(&board, 7, 3, PIECE_QUEEN, COLOR_WHITE) != 0) {
        return 1;
    }

    if (expect_piece(&board, 7, 4, PIECE_KING, COLOR_WHITE) != 0) {
        return 1;
    }

    movePiece(&board, movement);

    puts("smoke test passed");
    return 0;
}
