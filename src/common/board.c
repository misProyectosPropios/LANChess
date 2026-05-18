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
    BoardPosition* rel;
    if (!is_on_board(position.row, position.col)) return result;

    Piece piece = board->squares[position.row][position.col];
    if (piece.type == PIECE_NONE) return result;

    // Allocate a maximum possible moves for a piece (simple upper bound)
    result.possiblePositions = malloc(28 * sizeof(BoardPosition)); 
    if (!result.possiblePositions) return result;
    
    switch (piece.type) {
        case PIECE_PAWN:
            rel =  malloc(8 * sizeof(BoardPosition));
            if (!rel) return result; 
            
            rel[0] = (BoardPosition){.row = 1, .col = 0};
            rel[1] = (BoardPosition){.row = 2, .col = 0};
            rel[2] = (BoardPosition){.row = -1, .col = 0};
            rel[3] = (BoardPosition){.row = -2, .col = 0};
            rel[4] = (BoardPosition){.row = 1, .col = 1};
            rel[5] = (BoardPosition){.row = 1, .col = -1};
            rel[6] = (BoardPosition){.row = -1, .col = 1};
            rel[7] = (BoardPosition){.row = -1, .col = -1};
            relativePositions = (RelativePosition){
                .length = 2,
                .relative = rel
            };
            possiblePosition(&result, &relativePositions, position);
            break;
        case PIECE_ROOK:
            {
                rel = malloc(28 * sizeof(BoardPosition));
                if (!rel) return result;
                for (int i = 0; i < 7; i++) {
                    rel[i]      = (BoardPosition){.row =  (i + 1), .col = 0};
                    rel[i + 7]  = (BoardPosition){.row = -(i + 1), .col = 0};
                    rel[i + 14] = (BoardPosition){.row = 0,        .col =  (i + 1)};
                    rel[i + 21] = (BoardPosition){.row = 0,        .col = -(i + 1)};
                }
                relativePositions = (RelativePosition){
                    .length = 28,
                    .relative = rel
                };
                possiblePosition(&result, &relativePositions, position);
            }
            break;
        case PIECE_KNIGHT:
            
            rel =  malloc(8 * sizeof(BoardPosition));
            if (!rel) return result; 
            
            rel[0] = (BoardPosition){.row = 1, .col = 2};
            rel[1] = (BoardPosition){.row = 1, .col = -2};
            rel[2] = (BoardPosition){.row = -1, .col = 2};
            rel[3] = (BoardPosition){.row = -1, .col = -2};
            rel[4] = (BoardPosition){.row = 2, .col = 1};
            rel[5] = (BoardPosition){.row = 2, .col = -1};
            rel[6] = (BoardPosition){.row = -2, .col = 1};
            rel[7] = (BoardPosition){.row = -2, .col = -1};


            relativePositions = (RelativePosition){
                .length = 8,
                .relative = rel
            };
            possiblePosition(&result, &relativePositions, position);
            break;
        case PIECE_BISHOP:

            rel =  malloc(28 * sizeof(BoardPosition));
            if (!rel) return result; 

            for(int i = 0; i < 7; i++) {
                rel[i]      = (BoardPosition){.row = i, .col = i};
                rel[i + 7]  = (BoardPosition){.row = i, .col = -i};
                rel[i + 14] = (BoardPosition){.row = -i, .col = i};
                rel[i + 21] = (BoardPosition){.row = -i, .col = -i};
            }

            relativePositions = (RelativePosition){
                .length = 28,
                .relative = rel
            };
            possiblePosition(&result, &relativePositions, position);
            break;
        case PIECE_QUEEN:
            rel =  malloc(56 * sizeof(BoardPosition));
            if (!rel) return result; 

            for(int i = 0; i < 7; i++) {
                rel[i]      = (BoardPosition){.row = i,        .col = i};
                rel[i + 7]  = (BoardPosition){.row = i,        .col = -i};
                rel[i + 14] = (BoardPosition){.row = -i,       .col = i};
                rel[i + 21] = (BoardPosition){.row = -i,       .col = -i};
                rel[i + 28] = (BoardPosition){.row =  (i + 1), .col = 0};
                rel[i + 35] = (BoardPosition){.row = -(i + 1), .col = 0};
                rel[i + 42] = (BoardPosition){.row = 0,        .col =  (i + 1)};
                rel[i + 49] = (BoardPosition){.row = 0,        .col = -(i + 1)};
            }


            relativePositions = (RelativePosition){
                .length = 56,
                .relative = rel
            };
            possiblePosition(&result, &relativePositions, position);
            break;
        case PIECE_KING:
            rel =  malloc(8 * sizeof(BoardPosition));
            if (!rel) return result; 

            rel[0] = (BoardPosition){.row = 1,  .col = 0};
            rel[1] = (BoardPosition){.row = 1,  .col = 1};
            rel[2] = (BoardPosition){.row = 1,  .col = -1};
            rel[3] = (BoardPosition){.row = -1, .col = 1};
            rel[4] = (BoardPosition){.row = -1, .col = -1};
            rel[5] = (BoardPosition){.row = -1, .col = 0};
            rel[6] = (BoardPosition){.row = 0,  .col = 1};
            rel[7] = (BoardPosition){.row = 0,  .col = -1};

            relativePositions = (RelativePosition){
                .length = 8,
                .relative = rel
            };
            possiblePosition(&result, &relativePositions, position);
            break;
        default:
            break;
    }
    return result;
}

void pawnCannotMoveBackwards(BoardPosition* piecePosition, Piece* pawn, PossiblePositions* movements) {
    //#TODO
    if (pawn->color == COLOR_WHITE) {
       // Remove any moves that go backwards
      
       for (int i = 0; i < movements->length; i++) {
           if (movements->possiblePositions[i].row > piecePosition->row) {
               // Shift remaining moves left
               for (int j = i; j < movements->length - 1; j++) {
                   movements->possiblePositions[j] = movements->possiblePositions[j + 1];
               }
               movements->length--;
               i--; // Check the new move at this index
           }
       }
    }
    return;    
}


void moveCannotGoBeyondEnemies(Board* board, PossiblePositions* movements) {
   //#TODO

   return;
 }

void moveCannotGoBeyondAllies(Board* board, PossiblePositions* movements) {
   //#TODO
    return;
}

int isPinned(Board* board, Movement movement) {
  //#TODO
   return 1;
}

int cannotMovePinnedPiece(Board* board, PossiblePositions* positions, Movement movement) {
   //#TODO
   return 1;
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
    movements->length = index; // Set the actual number of valid positions found

    if (relative->relative){
        free(relative->relative);
    }
}

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

    if (movingPiece.type != PIECE_NONE) {
       movePiece(board, movement);
   }
}

void movePiece(Board *board, Movement movement)
{
    // Separated printing logic with a debug flag
    printMovement(movement, true);
    
    board->squares[movement.to.row][movement.to.col] = board->squares[movement.from.row][movement.from.col];
    board->squares[movement.from.row][movement.from.col] = empty_piece();
}
