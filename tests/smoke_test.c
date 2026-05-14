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

static int expect_backrank(Board *board, int row, PieceColor color)
{
    ASSERT_TEST_SUCCESS(expect_piece(board, row, 0, PIECE_ROOK, color));
    ASSERT_TEST_SUCCESS(expect_piece(board, row, 1, PIECE_KNIGHT, color));
    ASSERT_TEST_SUCCESS(expect_piece(board, row, 2, PIECE_BISHOP, color));
    ASSERT_TEST_SUCCESS(expect_piece(board, row, 3, PIECE_QUEEN, color));
    ASSERT_TEST_SUCCESS(expect_piece(board, row, 4, PIECE_KING, color));
    ASSERT_TEST_SUCCESS(expect_piece(board, row, 5, PIECE_BISHOP, color));
    ASSERT_TEST_SUCCESS(expect_piece(board, row, 6, PIECE_KNIGHT, color));
    ASSERT_TEST_SUCCESS(expect_piece(board, row, 7, PIECE_ROOK, color));
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

    ASSERT_TEST_SUCCESS(expect_backrank(&board, 0, COLOR_BLACK));
    ASSERT_TEST_SUCCESS(expect_backrank(&board, 7, COLOR_WHITE));

    for(int i = 0; i < 8; i++) {
        ASSERT_TEST_SUCCESS(expect_piece(&board, 1, i, PIECE_PAWN, COLOR_BLACK));
        ASSERT_TEST_SUCCESS(expect_piece(&board, 6, i, PIECE_PAWN, COLOR_WHITE));
    }
    
    for(int i = 2; i < 6; i++) {
        for(int j = 0; j < 8; j++) {
            ASSERT_TEST_SUCCESS(expect_piece(&board, i, j, PIECE_NONE, COLOR_NONE));
        }
    }
    return TEST_SUCCESS;
}

static int test_pawn(void)
{
    return TEST_SUCCESS;
}

static int test_doubleMovePawn(void)
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
        //{ "move piece", test_move_piece },
        { "pawn", test_pawn },
        { "doublePawn", test_doubleMovePawn },
        { "rook", test_rook },
        { "knight", test_knight },
        { "bishop", test_bishop },
        { "queen", test_queen },
        { "king", test_king }
    };

    return run_tests(tests, TEST_COUNT(tests));
}
