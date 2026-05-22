#include <stdio.h>
#include <string.h>
#include <stdbool.h>

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

static void perform_move(int prevrow, int prevcol, int nextrow, int nextcol, Board *board)
{
    Movement movement = {
        .from = { .row = prevrow, .col = prevcol },
        .to = { .row = nextrow, .col = nextcol }
    };
    move(board, movement);
}

static void clear_board(Board *board)
{
    for (int row = 0; row < LANCHESS_BOARD_SIZE; row++) {
        for (int col = 0; col < LANCHESS_BOARD_SIZE; col++) {
            board->squares[row][col] = (Piece){ PIECE_NONE, COLOR_NONE };
        }
    }
}

static void set_piece(Board *board, int row, int col, PieceType type, PieceColor color)
{
    board->squares[row][col] = (Piece){ type, color };
}

static bool positions_equal(BoardPosition a, BoardPosition b)
{
    return a.row == b.row && a.col == b.col;
}

static bool king_is_in_check(Board *board, PieceColor kingColor)
{
    BoardPosition kingPos = { .row = -1, .col = -1 };
    for (int row = 0; row < LANCHESS_BOARD_SIZE; row++) {
        for (int col = 0; col < LANCHESS_BOARD_SIZE; col++) {
            Piece piece = board->squares[row][col];
            if (piece.type == PIECE_KING && piece.color == kingColor) {
                kingPos.row = row;
                kingPos.col = col;
            }
        }
    }

    if (kingPos.row < 0) {
        return false;
    }

    PieceColor opponentColor = kingColor == COLOR_WHITE ? COLOR_BLACK : COLOR_WHITE;
    for (int row = 0; row < LANCHESS_BOARD_SIZE; row++) {
        for (int col = 0; col < LANCHESS_BOARD_SIZE; col++) {
            Piece piece = board->squares[row][col];
            if (piece.color != opponentColor) {
                continue;
            }

            PossiblePositions targets = movements(board, (BoardPosition){ .row = row, .col = col });
            for (int i = 0; i < (int)size_BoardPosition(targets.possiblePositions); i++) {
                BoardPosition target = get_BoardPosition(targets.possiblePositions, i);
                if (positions_equal(target, kingPos)) {
                    destroy_BoardPosition(targets.possiblePositions);
                    return true;
                }
            }
            destroy_BoardPosition(targets.possiblePositions);
        }
    }

    return false;
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
    Board board;
    initializeClassicGame(&board);
    perform_move(1, 0, 2, 0, &board);
    ASSERT_TEST_SUCCESS(expect_piece(&board, 2, 0, PIECE_PAWN, COLOR_BLACK));
    ASSERT_TEST_SUCCESS(expect_piece(&board, 1, 0, PIECE_NONE, COLOR_NONE));
    return TEST_SUCCESS;
}

static int test_opponents_can_play(void)
{
    Board board;
    initializeClassicGame(&board);
    perform_move(1, 0, 3, 0, &board);
    perform_move(6, 0, 4, 0, &board);
    ASSERT_TEST_SUCCESS(expect_piece(&board, 3, 0, PIECE_PAWN, COLOR_BLACK));
    ASSERT_TEST_SUCCESS(expect_piece(&board, 4, 0, PIECE_PAWN, COLOR_WHITE));
    return TEST_SUCCESS;
}

static int test_en_passant(void)
{
    Board board;
    clear_board(&board);
    set_piece(&board, 4, 4, PIECE_PAWN, COLOR_BLACK);
    set_piece(&board, 6, 5, PIECE_PAWN, COLOR_WHITE);
    set_piece(&board, 0, 0, PIECE_KING, COLOR_BLACK);
    set_piece(&board, 7, 7, PIECE_KING, COLOR_WHITE);

    perform_move(6, 5, 4, 5, &board);
    perform_move(4, 4, 5, 5, &board);

    ASSERT_TEST_SUCCESS(expect_piece(&board, 5, 5, PIECE_PAWN, COLOR_BLACK));
    ASSERT_TEST_SUCCESS(expect_piece(&board, 4, 5, PIECE_NONE, COLOR_NONE));
    ASSERT_TEST_SUCCESS(expect_piece(&board, 4, 4, PIECE_NONE, COLOR_NONE));
    return TEST_SUCCESS;
}

static int test_pawn_promotion_to_queen(void)
{
    Board board;
    clear_board(&board);
    set_piece(&board, 1, 0, PIECE_PAWN, COLOR_WHITE);
    set_piece(&board, 0, 4, PIECE_KING, COLOR_BLACK);
    set_piece(&board, 7, 7, PIECE_KING, COLOR_WHITE);

    perform_move(1, 0, 0, 0, &board);

    ASSERT_TEST_SUCCESS(expect_piece(&board, 0, 0, PIECE_QUEEN, COLOR_WHITE));
    return TEST_SUCCESS;
}

static int test_check(void)
{
    Board board;
    clear_board(&board);
    set_piece(&board, 7, 4, PIECE_KING, COLOR_WHITE);
    set_piece(&board, 0, 4, PIECE_KING, COLOR_BLACK);
    set_piece(&board, 7, 0, PIECE_ROOK, COLOR_BLACK);

    ASSERT_TRUE(king_is_in_check(&board, COLOR_WHITE));
    return TEST_SUCCESS;
}

static int test_checkmate(void)
{
    Board board;
    clear_board(&board);
    set_piece(&board, 0, 0, PIECE_KING, COLOR_BLACK);
    set_piece(&board, 0, 1, PIECE_PAWN, COLOR_BLACK);
    set_piece(&board, 1, 0, PIECE_PAWN, COLOR_BLACK);
    set_piece(&board, 1, 1, PIECE_QUEEN, COLOR_WHITE);
    set_piece(&board, 2, 2, PIECE_BISHOP, COLOR_WHITE);
    set_piece(&board, 7, 7, PIECE_KING, COLOR_WHITE);

    ASSERT_TRUE(is_checkmate(&board, COLOR_BLACK));
    return TEST_SUCCESS;
}

static int test_pawnCannotMoveBackwards(void) {
    
    Board board;
    initializeClassicGame(&board);
    perform_move(1, 0, 2, 0, &board);
    perform_move(2, 0, 1, 0, &board);
    ASSERT_TEST_SUCCESS(expect_piece(&board, 1, 0, PIECE_NONE, COLOR_NONE));
    ASSERT_TEST_SUCCESS(expect_piece(&board, 2, 0, PIECE_PAWN, COLOR_BLACK));
    return TEST_SUCCESS;
}

static int test_doubleMovePawn(void)
{
  Board board;
    initializeClassicGame(&board);
    perform_move(1, 0, 3, 0, &board);
    ASSERT_TEST_SUCCESS(expect_piece(&board, 3, 0, PIECE_PAWN, COLOR_BLACK));
    return TEST_SUCCESS;
}

static int test_rook(void)
{
    Board board;
    initializeClassicGame(&board);
    perform_move(1, 0, 3, 0, &board);
    perform_move(0, 0, 1, 0, &board);

    ASSERT_TEST_SUCCESS(expect_piece(&board, 1, 0, PIECE_ROOK, COLOR_BLACK));
    return TEST_SUCCESS;
}

static int test_knight(void)
{
  Board board;
    initializeClassicGame(&board);
    perform_move(0, 1, 2, 0, &board);

    ASSERT_TEST_SUCCESS(expect_piece(&board, 2, 0, PIECE_KNIGHT, COLOR_BLACK));
    return TEST_SUCCESS;
}

static int test_bishop(void)
{
    Board board;
    initializeClassicGame(&board);
    perform_move(1, 1, 3, 1, &board);
    perform_move(0, 2, 1, 1, &board);

    ASSERT_TEST_SUCCESS(expect_piece(&board, 1, 1, PIECE_BISHOP, COLOR_BLACK));
    return TEST_SUCCESS;
}

static int test_queen(void)
{
    Board board;
    initializeClassicGame(&board);
    perform_move(1, 3, 2, 3, &board);
    perform_move(0, 3, 1, 3, &board);

    ASSERT_TEST_SUCCESS(expect_piece(&board, 1, 3, PIECE_QUEEN, COLOR_BLACK));
    return TEST_SUCCESS;
}

static int test_king(void)
{

    Board board;
    initializeClassicGame(&board);
    perform_move(1, 4, 2, 4, &board);
    perform_move(0, 4, 1, 4, &board);

    ASSERT_TEST_SUCCESS(expect_piece(&board, 1, 4, PIECE_KING, COLOR_BLACK));
    return TEST_SUCCESS;
}


static int test_movePieceOverSamePieceDoenstMoveIt() {
    Board board;
    initializeClassicGame(&board);
    perform_move(0, 0, 1, 0, &board);

    ASSERT_TEST_SUCCESS(expect_piece(&board, 0, 0, PIECE_ROOK, COLOR_BLACK));
    ASSERT_TEST_SUCCESS(expect_piece(&board, 1, 0, PIECE_PAWN, COLOR_BLACK));
    return TEST_SUCCESS;
}

static int test_cannotMovePinnedPiece() {
    Board board;
    initializeClassicGame(&board);
    perform_move(0, 0, 1, 0, &board);

    ASSERT_TEST_SUCCESS(expect_piece(&board, 0, 0, PIECE_ROOK, COLOR_BLACK));
    ASSERT_TEST_SUCCESS(expect_piece(&board, 1, 0, PIECE_PAWN, COLOR_BLACK));
    return TEST_SUCCESS;
}

void nullFunction() {
    
}

int main(void)
{
    TestCase tests[] = {
        { "app info", test_app_info },
        { "classic board setup", test_classic_board_setup },
        { "pawn", test_pawn },
        { "opponents can play", test_opponents_can_play },
        { "en passant", test_en_passant },
        { "pawn promotion to queen", test_pawn_promotion_to_queen },
        { "check", test_check },
        { "checkmate", test_checkmate },
        { "doubleMovePawn", test_doubleMovePawn },
        { "pawnCannotMoveBackwards", test_pawnCannotMoveBackwards },
        { "rook", test_rook },
        { "knight", test_knight },
        { "bishop", test_bishop },
        { "queen", test_queen },
        { "king", test_king },
        { "movePieceOverSamePieceDoenstMoveIt", test_movePieceOverSamePieceDoenstMoveIt },
        { "cannotMovePinnedPiece", test_cannotMovePinnedPiece }
    };

    return run_tests(tests, TEST_COUNT(tests), nullFunction, nullFunction);
}
