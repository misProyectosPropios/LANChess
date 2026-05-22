#include "board.h"
#include "Set.h"
#include "ArrayList.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>

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

static bool is_on_board(int row, int col) {
    return row >= 0 && row < LANCHESS_BOARD_SIZE && col >= 0 && col < LANCHESS_BOARD_SIZE;
}

static void printMovement(Movement movement, bool is_debug)
{
    if (is_debug) {
        printf(
            "DEBUG: Moving piece from [%d, %d] to [%d, %d]\n",
            movement.from.row, movement.from.col,
            movement.to.row, movement.to.col
        );
    }
}

static bool is_empty_square(Board *board, BoardPosition position)
{
    return is_on_board(position.row, position.col) &&
           board->squares[position.row][position.col].type == PIECE_NONE;
}

static bool is_same_color(Board *board, BoardPosition position, PieceColor color)
{
    if (!is_on_board(position.row, position.col)) {
        return false;
    }
    Piece piece = board->squares[position.row][position.col];
    return piece.type != PIECE_NONE && piece.color == color;
}

static bool is_opponent_piece(Board *board, BoardPosition position, PieceColor color)
{
    if (!is_on_board(position.row, position.col)) {
        return false;
    }
    Piece piece = board->squares[position.row][position.col];
    return piece.type != PIECE_NONE && piece.color != color;
}

static BoardPosition find_king(Board *board, PieceColor color)
{
    for (int row = 0; row < LANCHESS_BOARD_SIZE; row++) {
        for (int col = 0; col < LANCHESS_BOARD_SIZE; col++) {
            Piece piece = board->squares[row][col];
            if (piece.type == PIECE_KING && piece.color == color) {
                return (BoardPosition){ .row = row, .col = col };
            }
        }
    }
    return (BoardPosition){ .row = -1, .col = -1 };
}

static bool is_attacked_by_sliding(Board *board, BoardPosition target, PieceColor attackerColor, int rowStep, int colStep)
{
    BoardPosition scan = target;
    while (true) {
        scan.row += rowStep;
        scan.col += colStep;

        if (!is_on_board(scan.row, scan.col)) {
            return false;
        }

        Piece piece = board->squares[scan.row][scan.col];
        if (piece.type == PIECE_NONE) {
            continue;
        }

        if (piece.color != attackerColor) {
            return false;
        }

        if ((rowStep == 0 || colStep == 0) &&
            (piece.type == PIECE_ROOK || piece.type == PIECE_QUEEN)) {
            return true;
        }

        if ((rowStep != 0 && colStep != 0) &&
            (piece.type == PIECE_BISHOP || piece.type == PIECE_QUEEN)) {
            return true;
        }

        return false;
    }
}

static bool is_square_attacked(Board *board, BoardPosition target, PieceColor attackerColor)
{
    int pawnRow = attackerColor == COLOR_WHITE ? target.row + 1 : target.row - 1;
    for (int colDiff = -1; colDiff <= 1; colDiff += 2) {
        BoardPosition pawnPos = { .row = pawnRow, .col = target.col + colDiff };
        if (is_on_board(pawnPos.row, pawnPos.col)) {
            Piece piece = board->squares[pawnPos.row][pawnPos.col];
            if (piece.type == PIECE_PAWN && piece.color == attackerColor) {
                return true;
            }
        }
    }

    int knightRows[] = { 2, 2, -2, -2, 1, 1, -1, -1 };
    int knightCols[] = { 1, -1, 1, -1, 2, -2, 2, -2 };

    for (int i = 0; i < 8; i++) {
        BoardPosition scan = { .row = target.row + knightRows[i], .col = target.col + knightCols[i] };
        if (is_on_board(scan.row, scan.col)) {
            Piece piece = board->squares[scan.row][scan.col];
            if (piece.type == PIECE_KNIGHT && piece.color == attackerColor) {
                return true;
            }
        }
    }

    int directions[][2] = {
        { 1, 0 }, { -1, 0 }, { 0, 1 }, { 0, -1 },
        { 1, 1 }, { 1, -1 }, { -1, 1 }, { -1, -1 }
    };

    for (int i = 0; i < 8; i++) {
        if (is_attacked_by_sliding(board, target, attackerColor, directions[i][0], directions[i][1])) {
            return true;
        }
    }

    for (int rowDiff = -1; rowDiff <= 1; rowDiff++) {
        for (int colDiff = -1; colDiff <= 1; colDiff++) {
            if (rowDiff == 0 && colDiff == 0) {
                continue;
            }
            BoardPosition scan = { .row = target.row + rowDiff, .col = target.col + colDiff };
            if (is_on_board(scan.row, scan.col)) {
                Piece piece = board->squares[scan.row][scan.col];
                if (piece.type == PIECE_KING && piece.color == attackerColor) {
                    return true;
                }
            }
        }
    }

    return false;
}

static bool king_is_in_check(Board *board, PieceColor kingColor)
{
    BoardPosition kingPos = find_king(board, kingColor);
    if (!is_on_board(kingPos.row, kingPos.col)) {
        return false;
    }

    PieceColor opponentColor = kingColor == COLOR_WHITE ? COLOR_BLACK : COLOR_WHITE;
    return is_square_attacked(board, kingPos, opponentColor);
}

bool is_in_check(Board *board, PieceColor color)
{
    if (board == NULL) {
        return false;
    }
    return king_is_in_check(board, color);
}

bool is_checkmate(Board *board, PieceColor color)
{
    if (board == NULL || !king_is_in_check(board, color)) {
        return false;
    }

    for (int row = 0; row < LANCHESS_BOARD_SIZE; row++) {
        for (int col = 0; col < LANCHESS_BOARD_SIZE; col++) {
            Piece piece = board->squares[row][col];
            if (piece.color != color) {
                continue;
            }

            PossiblePositions targets = movements(board, (BoardPosition){ .row = row, .col = col });
            if (targets.possiblePositions != NULL && size_BoardPosition(targets.possiblePositions) > 0) {
                destroy_BoardPosition(targets.possiblePositions);
                return false;
            }
            destroy_BoardPosition(targets.possiblePositions);
        }
    }

    return true;
}

void printBoard(Board *board)
{
    if (board == NULL) {
        return;
    }

    printf("  0 1 2 3 4 5 6 7\n");
    for (int row = 0; row < LANCHESS_BOARD_SIZE; row++) {
        printf("%d ", row);
        for (int col = 0; col < LANCHESS_BOARD_SIZE; col++) {
            Piece piece = board->squares[row][col];
            char symbol = '.';
            if (piece.type != PIECE_NONE) {
                switch (piece.type) {
                    case PIECE_PAWN: symbol = 'p'; break;
                    case PIECE_ROOK: symbol = 'r'; break;
                    case PIECE_KNIGHT: symbol = 'n'; break;
                    case PIECE_BISHOP: symbol = 'b'; break;
                    case PIECE_QUEEN: symbol = 'q'; break;
                    case PIECE_KING: symbol = 'k'; break;
                    default: symbol = '?'; break;
                }
                if (piece.color == COLOR_WHITE) {
                    symbol = (char)toupper(symbol);
                }
            }
            printf("%c ", symbol);
        }
        printf("\n");
    }
}

static bool is_en_passant_move(Board *board, Movement movement)
{
    Piece pawn = board->squares[movement.from.row][movement.from.col];
    if (pawn.type != PIECE_PAWN) {
        return false;
    }

    if (movement.from.col == movement.to.col) {
        return false;
    }

    if (!is_empty_square(board, movement.to)) {
        return false;
    }

    BoardPosition adjacent = { .row = movement.from.row, .col = movement.to.col };
    if (!is_opponent_piece(board, adjacent, pawn.color) ||
        board->squares[adjacent.row][adjacent.col].type != PIECE_PAWN) {
        return false;
    }

    if (pawn.color == COLOR_WHITE) {
        return movement.from.row == 3 && movement.to.row == 2;
    }

    return movement.from.row == 4 && movement.to.row == 5;
}

static bool move_leaves_king_in_check(Board *board, Movement movement)
{
    Board copy = *board;
    Piece movingPiece = copy.squares[movement.from.row][movement.from.col];
    if (movingPiece.type == PIECE_NONE) {
        return false;
    }

    if (is_en_passant_move(&copy, movement)) {
        BoardPosition captured = { .row = movement.from.row, .col = movement.to.col };
        copy.squares[captured.row][captured.col] = empty_piece();
    }

    copy.squares[movement.to.row][movement.to.col] = movingPiece;
    copy.squares[movement.from.row][movement.from.col] = empty_piece();
    return king_is_in_check(&copy, movingPiece.color);
}

static bool is_pawn_promotion(Piece pawn, Movement movement)
{
    if (pawn.type != PIECE_PAWN) {
        return false;
    }

    if (pawn.color == COLOR_WHITE) {
        return movement.to.row == 0;
    }

    if (pawn.color == COLOR_BLACK) {
        return movement.to.row == 7;
    }

    return false;
}

static void promote_pawn(Board *board, Movement movement)
{
    board->squares[movement.to.row][movement.to.col].type = PIECE_QUEEN;
}

static bool add_safe_move(Board *board, PossiblePositions *result, BoardPosition from, BoardPosition to, PieceColor color)
{
    if (!is_on_board(to.row, to.col)) {
        return false;
    }

    if (is_same_color(board, to, color)) {
        return false;
    }

    Movement candidate = { .from = from, .to = to };
    if (move_leaves_king_in_check(board, candidate)) {
        return false;
    }

    add_BoardPosition(result->possiblePositions, to);
    return true;
}

static void add_slide_moves(Board *board, PossiblePositions *result, BoardPosition origin, int rowStep, int colStep, PieceColor color)
{
    BoardPosition current = origin;
    while (true) {
        current.row += rowStep;
        current.col += colStep;

        if (!is_on_board(current.row, current.col)) {
            return;
        }

        if (is_same_color(board, current, color)) {
            return;
        }

        Movement candidate = { .from = origin, .to = current };
        if (!move_leaves_king_in_check(board, candidate)) {
            add_BoardPosition(result->possiblePositions, current);
        }

        if (!is_empty_square(board, current)) {
            return;
        }
    }
}

PossiblePositions movements(Board *board, BoardPosition position) {
    PossiblePositions result = {0};
    if (!is_on_board(position.row, position.col)) {
        return result;
    }

    Piece piece = board->squares[position.row][position.col];
    if (piece.type == PIECE_NONE) {
        return result;
    }

    result.possiblePositions = createArray_BoardPosition();
    int forward = piece.color == COLOR_WHITE ? -1 : 1;

    switch (piece.type) {
        case PIECE_PAWN: {
            BoardPosition oneStep = { .row = position.row + forward, .col = position.col };
            if (is_empty_square(board, oneStep)) {
                add_safe_move(board, &result, position, oneStep, piece.color);

                int startRow = piece.color == COLOR_WHITE ? 6 : 1;
                BoardPosition twoSteps = { .row = position.row + forward * 2, .col = position.col };
                if (position.row == startRow && is_empty_square(board, twoSteps) && is_empty_square(board, oneStep)) {
                    add_safe_move(board, &result, position, twoSteps, piece.color);
                }
            }

            for (int colDiff = -1; colDiff <= 1; colDiff += 2) {
                BoardPosition diagonal = { .row = position.row + forward, .col = position.col + colDiff };
                if (!is_on_board(diagonal.row, diagonal.col)) {
                    continue;
                }

                if (is_opponent_piece(board, diagonal, piece.color)) {
                    add_safe_move(board, &result, position, diagonal, piece.color);
                    continue;
                }

                Movement candidate = { .from = position, .to = diagonal };
                if (is_en_passant_move(board, candidate)) {
                    add_safe_move(board, &result, position, diagonal, piece.color);
                }
            }
            break;
        }
        case PIECE_ROOK:
            add_slide_moves(board, &result, position, 1, 0, piece.color);
            add_slide_moves(board, &result, position, -1, 0, piece.color);
            add_slide_moves(board, &result, position, 0, 1, piece.color);
            add_slide_moves(board, &result, position, 0, -1, piece.color);
            break;
        case PIECE_KNIGHT: {
            int rowOffsets[] = { 1, 1, -1, -1, 2, 2, -2, -2 };
            int colOffsets[] = { 2, -2, 2, -2, 1, -1, 1, -1 };
            for (int i = 0; i < 8; i++) {
                BoardPosition target = { .row = position.row + rowOffsets[i], .col = position.col + colOffsets[i] };
                add_safe_move(board, &result, position, target, piece.color);
            }
            break;
        }
        case PIECE_BISHOP:
            add_slide_moves(board, &result, position, 1, 1, piece.color);
            add_slide_moves(board, &result, position, 1, -1, piece.color);
            add_slide_moves(board, &result, position, -1, 1, piece.color);
            add_slide_moves(board, &result, position, -1, -1, piece.color);
            break;
        case PIECE_QUEEN:
            add_slide_moves(board, &result, position, 1, 0, piece.color);
            add_slide_moves(board, &result, position, -1, 0, piece.color);
            add_slide_moves(board, &result, position, 0, 1, piece.color);
            add_slide_moves(board, &result, position, 0, -1, piece.color);
            add_slide_moves(board, &result, position, 1, 1, piece.color);
            add_slide_moves(board, &result, position, 1, -1, piece.color);
            add_slide_moves(board, &result, position, -1, 1, piece.color);
            add_slide_moves(board, &result, position, -1, -1, piece.color);
            break;
        case PIECE_KING: {
            for (int rowDiff = -1; rowDiff <= 1; rowDiff++) {
                for (int colDiff = -1; colDiff <= 1; colDiff++) {
                    if (rowDiff == 0 && colDiff == 0) {
                        continue;
                    }
                    BoardPosition target = { .row = position.row + rowDiff, .col = position.col + colDiff };
                    add_safe_move(board, &result, position, target, piece.color);
                }
            }
            break;
        }
        default:
            break;
    }

    return result;
}

void move(Board *board, Movement movement) {
    Piece movingPiece = board->squares[movement.from.row][movement.from.col];

    if (movingPiece.type == PIECE_NONE) {
        fprintf(stderr, "Error: No piece at source position.\n");
        return;
    }

    PossiblePositions validMoves = movements(board, movement.from);
    bool is_valid = false;
    for (int i = 0; i < (int)size_BoardPosition(validMoves.possiblePositions); i++) {
        BoardPosition candidate = get_BoardPosition(validMoves.possiblePositions, i);
        if (candidate.row == movement.to.row && candidate.col == movement.to.col) {
            is_valid = true;
            break;
        }
    }
    destroy_BoardPosition(validMoves.possiblePositions);

    if (!is_valid) {
        fprintf(stderr, "Error: Invalid move for this piece type.\n");
        return;
    }

    if (board->squares[movement.to.row][movement.to.col].color == movingPiece.color) {
        fprintf(stderr, "Error: Target square occupied by friendly piece.\n");
        return;
    }

    if (movingPiece.type != PIECE_NONE) {
        movePiece(board, movement);
    }
}

void movePiece(Board *board, Movement movement)
{
    printMovement(movement, false);
    Piece movingPiece = board->squares[movement.from.row][movement.from.col];

    if (is_en_passant_move(board, movement)) {
        BoardPosition captured = { .row = movement.from.row, .col = movement.to.col };
        board->squares[captured.row][captured.col] = empty_piece();
    }

    board->squares[movement.to.row][movement.to.col] = movingPiece;
    board->squares[movement.from.row][movement.from.col] = empty_piece();

    if (is_pawn_promotion(movingPiece, movement)) {
        promote_pawn(board, movement);
    }
}

void pawnCannotMoveBackwards(BoardPosition* piecePosition, Piece* pawn, PossiblePositions* movements) {
    if (pawn->type == PIECE_PAWN && pawn->color == COLOR_WHITE) {
       // Remove any moves that go backwards
       
       for (int i = 0; i < (int)size_BoardPosition(movements->possiblePositions); i++) {
           BoardPosition current = get_BoardPosition(movements->possiblePositions, i);
           if (current.row < piecePosition->row) {
               removeItem_BoardPosition(movements->possiblePositions, i);
               i--; // Check the new move at this index
           }
       }
    } else if (pawn->type == PIECE_PAWN && pawn->color == COLOR_BLACK) {
        for (int i = 0; i < (int)size_BoardPosition(movements->possiblePositions); i++) {
           BoardPosition current = get_BoardPosition(movements->possiblePositions, i);
           if (current.row > piecePosition->row) {
               removeItem_BoardPosition(movements->possiblePositions, i);
               i--; // Check the new move at this index
           }
       }
    }
}


void moveCannotGoBeyondEnemies(Board* board, PossiblePositions* movements) {
    (void)board;
    (void)movements;
    //#TODO
    return;
}

void moveCannotGoBeyondAllies(Board* board, PossiblePositions* movements) {
    (void)board;
    (void)movements;
    //#TODO
    return;
}

int isPinned(Board* board, Movement movement) {
    (void)board;
    (void)movement;
    //#TODO
    return 1;
}

int cannotMovePinnedPiece(Board* board, PossiblePositions* positions, Movement movement) {
    (void)board;
    (void)positions;
    (void)movement;
    //#TODO
    return 1;
}

void possiblePosition(PossiblePositions* movements, RelativePosition* relative, BoardPosition position) {
    for (size_t i = 0; i < size_BoardPosition(relative->relative); i++) {
        BoardPosition rel = get_BoardPosition(relative->relative, i);
        BoardPosition target = {
            .row = position.row + rel.row,
            .col = position.col + rel.col
        };
        if (is_on_board(target.row, target.col)) {
            add_BoardPosition(movements->possiblePositions, target);
        }
    }
}
