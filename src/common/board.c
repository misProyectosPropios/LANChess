#include "board.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

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

PossiblePositions movements(Board *board, BoardPosition position) {
    PossiblePositions result = {0, NULL};
    RelativePosition relativePositions;
    if (!is_on_board(position.row, position.col)) return result;

    Piece piece = board->squares[position.row][position.col];
    if (piece.type == PIECE_NONE) return result;

    // Allocate a maximum possible moves for a piece (simple upper bound)
    result.possiblePositions = malloc(28 * sizeof(BoardPosition)); 
    if (!result.possiblePositions) return result;
    
    switch (piece.type) {
        case PIECE_PAWN:
            relativePositions = (RelativePosition){
                .length = 0,
                .relative = NULL
            };
            possiblePosition(&result, &relativePositions, position);
            break;
        case PIECE_ROOK:
            relativePositions = (RelativePosition){
                .length = 0,
                .relative = NULL
            };
            possiblePosition(&result, &relativePositions, position);
            break;
        case PIECE_KNIGHT:
            relativePositions = (RelativePosition){
                .length = 0,
                .relative = NULL
            };
            possiblePosition(&result, &relativePositions, position);
            break;
        case PIECE_BISHOP:
            relativePositions = (RelativePosition){
                .length = 0,
                .relative = NULL
            };
            possiblePosition(&result, &relativePositions, position);
            break;
        case PIECE_QUEEN:
            relativePositions = (RelativePosition){
                .length = 0,
                .relative = NULL
            };
            possiblePosition(&result, &relativePositions, position);
            break;
        case PIECE_KING:
            relativePositions = (RelativePosition){
                .length = 0,
                .relative = NULL
            };
            possiblePosition(&result, &relativePositions, position);
            break;
        default:
            break;
    }
    return result;
}

void possiblePosition(PossiblePositions* movements, RelativePosition* relative, BoardPosition position) {
    int index = 0;
    for(int i = 0; i < relative->length; i++) {
        BoardPosition target = {
            .row = position.row + relative->relative[i].row,
            .col = position.col + relative->relative[i].col
        };
        if (is_on_board(target.row, target.col)) {
            movements->possiblePositions[index] = target;
            index++;
        }
    }
}
    
    /*
    int rowOffsets[] = {-1, -1, -1,  0, 0,  1, 1, 1};
    int colOffsets[] = {-1,  0,  1, -1, 1, -1, 0, 1};

    for (int i = 0; i < 8; i++) {
        int targetRow = position.row + rowOffsets[i];
        int targetCol = position.col + colOffsets[i];
        movements->possibleMovements[movements->length++].from = position;
        movements->possibleMovements[movements->length++].to.row = targetRow;
        movements->possibleMovements[movements->length++].to.col = targetCol;
    }
    */


void move(Board *board, Movement movement) {
    Piece movingPiece = board->squares[movement.from.row][movement.from.col];

    // Verification: ensure a piece exists at the starting position
    if (movingPiece.type == PIECE_NONE) {
        fprintf(stderr, "Error: No piece at source position.\n");
        return;
    }

    // Verification: ensure the piece can move in the way correctly
    // We use the movements function to see if the target is in the list of valid moves
    PossiblePositions validMoves = movements(board, movement.from);
    bool is_valid = false;
    for (int i = 0; i < validMoves.length; i++) {
        if (validMoves.possiblePositions[i].row == movement.to.row &&
            validMoves.possiblePositions[i].col == movement.to.col) {
            is_valid = true;
            break;
        }
    }
    free(validMoves.possiblePositions);

    if (!is_valid) {
        fprintf(stderr, "Error: Invalid move for this piece type.\n");
        return;
    }

    // Verification: ensure target square isn't occupied by a friendly piece
    if (board->squares[movement.to.row][movement.to.col].color == movingPiece.color) {
        fprintf(stderr, "Error: Target square occupied by friendly piece.\n");
        return;
    }

    switch (movingPiece.type) {
    case PIECE_PAWN:
    case PIECE_ROOK:
        movePiece(board, movement);
        break;
    case PIECE_NONE:
    default:
        break;
    }
}

void movePiece(Board *board, Movement movement)
{
    // Separated printing logic with a debug flag
    printMovement(movement, true);
    
    board->squares[movement.to.row][movement.to.col] =
        board->squares[movement.from.row][movement.from.col];
    board->squares[movement.from.row][movement.from.col] = empty_piece();
}
