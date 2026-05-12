#include <stdio.h>
#include <string.h>

#include "app_info.h"
#include "board.h"
#include "test_lib.h"

static int expect_piece(Board *board, int row, int col, PieceType type, PieceColor color)
{
    Piece piece = board->squares[row][col];
    ASSERT_INT_EQ(type, piece.type);
    ASSERT_INT_EQ(color, piece.color);
    return TEST_SUCCESS;
}

static int test_app_info(void)
{
    ASSERT_STR_EQ("LANChess", lanchess_name());
    ASSERT_STR_EQ("0.1.0", lanchess_version());
    return TEST_SUCCESS;
}

static int test_classic_board_setup(void)
{
    Board board;

    initializeClassicGame(&board);

    if (expect_piece(&board, 0, 0, PIECE_ROOK, COLOR_BLACK) != TEST_SUCCESS) {
        return TEST_FAILURE;
    }

    if (expect_piece(&board, 0, 4, PIECE_KING, COLOR_BLACK) != TEST_SUCCESS) {
        return TEST_FAILURE;
    }

    if (expect_piece(&board, 1, 3, PIECE_PAWN, COLOR_BLACK) != TEST_SUCCESS) {
        return TEST_FAILURE;
    }

    if (expect_piece(&board, 4, 4, PIECE_NONE, COLOR_NONE) != TEST_SUCCESS) {
        return TEST_FAILURE;
    }

    if (expect_piece(&board, 6, 3, PIECE_PAWN, COLOR_WHITE) != TEST_SUCCESS) {
        return TEST_FAILURE;
    }

    if (expect_piece(&board, 7, 3, PIECE_QUEEN, COLOR_WHITE) != TEST_SUCCESS) {
        return TEST_FAILURE;
    }

    if (expect_piece(&board, 7, 4, PIECE_KING, COLOR_WHITE) != TEST_SUCCESS) {
        return TEST_FAILURE;
    }

    return TEST_SUCCESS;
}

static int test_move_piece(void)
{
    Board board;
    Movement movement = {
        { 6, 4 },
        { 4, 4 }
    };

    initializeClassicGame(&board);
    movePiece(&board, movement);

    ASSERT_INT_EQ(PIECE_PAWN, board.squares[4][4].type);
    ASSERT_INT_EQ(COLOR_WHITE, board.squares[4][4].color);
    ASSERT_INT_EQ(PIECE_NONE, board.squares[6][4].type);

    return TEST_SUCCESS;
}

static int test_pawn(void)
{
    return TEST_SUCCESS;
}

static int test_pawnDoubleMove(void)
{
    return TEST_SUCCESS;
}

static int test_rook(void)
{
    return TEST_SUCCESS;
}

static int test_knight(void)
{
    return TEST_SUCCESS;
}

static int test_bishop(void)
{
    return TEST_SUCCESS;
}

static int test_queen(void)
{
    return TEST_SUCCESS;
}

static int test_king(void)
{
    return TEST_SUCCESS;
}

int main(void)
{
    TestCase tests[] = {
        { "app info", test_app_info },
        { "classic board setup", test_classic_board_setup },
        { "move piece", test_move_piece },
        { "pawn", test_pawn },
        { "rook", test_rook },
        { "knight", test_knight },
        { "bishop", test_bishop },
        { "queen", test_queen },
        { "king", test_king }
    };

    return run_tests(tests, TEST_COUNT(tests));
}
