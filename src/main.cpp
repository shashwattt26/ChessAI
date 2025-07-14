#include <iostream>
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <string>

const int WINDOW_WIDTH = 600;
const int WINDOW_HEIGHT = 600;

bool whiteKingMoved = false, blackKingMoved = false;
bool whiteKingsideRookMoved = false, whiteQueensideRookMoved = false;
bool blackKingsideRookMoved = false, blackQueensideRookMoved = false;

bool isValidMove(int board[8][8], int fromRow, int fromCol, int toRow, int toCol, bool checkCheck);

void createChessBoard(SDL_Renderer *renderer) {
    int squareSize = WINDOW_WIDTH / 8;
    for (int row = 0; row < 8; ++row) {
        for (int col = 0; col < 8; ++col) {
            SDL_FRect square;
            square.x = col * squareSize;
            square.y = row * squareSize;
            square.w = squareSize;
            square.h = squareSize;

            if ((row + col) % 2 == 0) {
                SDL_SetRenderDrawColor(renderer, 166, 116, 30, 255); // White
            } else {
                SDL_SetRenderDrawColor(renderer, 255, 253, 152, 255); // Black
            }
            SDL_RenderFillRect(renderer, &square);
        }
    }
}

SDL_Texture* loadPiece(SDL_Renderer* renderer, const std::string& filename) {
    SDL_Texture* tex = IMG_LoadTexture(renderer, filename.c_str());
    if (!tex) {
        std::cerr << "IMG_LoadTexture Error: " << SDL_GetError() << std::endl;
    }
    return tex;
}

void drawPieces(SDL_Renderer* renderer, SDL_Texture* pieces[], int board[8][8]) {
    int squareSize = WINDOW_WIDTH / 8;
    for (int row = 0; row < 8; ++row) {
        for (int col = 0; col < 8; ++col) {
            if (board[row][col] != -1) {
                SDL_FRect dst;
                dst.x = col * squareSize;
                dst.y = row * squareSize;
                dst.w = squareSize;
                dst.h = squareSize;
                SDL_RenderTexture(renderer, pieces[board[row][col]], nullptr, &dst);
            }
        }
    }
}

bool checkForCheck(int board[8][8], bool isWhite) {
    // Find king position
    int kingRow = -1, kingCol = -1;
    int kingPiece = isWhite ? 11 : 5;
    for (int r = 0; r < 8; ++r) {
        for (int c = 0; c < 8; ++c) {
            if (board[r][c] == kingPiece) {
                kingRow = r; kingCol = c;
            }
        }
    }
    if (kingRow == -1) return false; // King not found

    // Check if any enemy piece can move to king's square
    for (int r = 0; r < 8; ++r) {
        for (int c = 0; c < 8; ++c) {
            if (board[r][c] != -1 && ((board[r][c] >= 6) != isWhite)) {
                if (isValidMove(board, r, c, kingRow, kingCol, false))
                    return true;
            }
        }
    }
    return false;
}

bool isValidMove(int board[8][8], int fromRow, int fromCol, int toRow, int toCol, bool checkCheck = true) {
    int piece = board[fromRow][fromCol];
    if(piece == -1) {
        return false; // No piece to move
    }
    bool isWhite = (piece >= 6); //pieces 0-5 are black, 6-11 are white
    if (toRow < 0 || toRow >= 8 || toCol < 0 || toCol >= 8) {
        return false; // Move out of bounds
    }
    int targetPiece = board[toRow][toCol];
    if (targetPiece != -1 && ((isWhite && targetPiece >= 6) || (!isWhite && targetPiece < 6))) {
        return false; // Can't capture own piece
    }
    if (checkCheck) {
        int temp = board[toRow][toCol];
        board[toRow][toCol] = board[fromRow][fromCol];
        board[fromRow][fromCol] = -1;

        bool causesCheck = checkForCheck(board, isWhite);

        // Undo the move
        board[fromRow][fromCol] = board[toRow][toCol];
        board[toRow][toCol] = temp;

        if (causesCheck) return false;
    }

    int dr = toRow - fromRow;
    int dc = toCol - fromCol;
    switch (piece % 6)
    {
    case 0: // Pawn
        if (isWhite) {
            if (dc == 0 && dr == -1 && targetPiece == -1) return true; // Move forward
            if (fromRow == 6 && dc == 0 && dr == -2 && board[fromRow-1][fromCol] == -1 && targetPiece == -1) return true; // Double move
            if (abs(dc) == 1 && dr == -1 && targetPiece != -1 && targetPiece < 6) return true; // Capture
        } else {
            if (dc == 0 && dr == 1 && targetPiece == -1) return true; // Move forward
            if (fromRow == 1 && dc == 0 && dr == 2 && board[fromRow+1][fromCol] == -1 && targetPiece == -1) return true; // Double move
            if (abs(dc) == 1 && dr == 1 && targetPiece != -1 && targetPiece >= 6) return true; // Capture
        }
        break;
    case 1: // Rook
        if (dc == 0 || dr == 0) { // Horizontal or vertical move
            int stepR = (dr != 0) ? (dr > 0 ? 1 : -1) : 0;
            int stepC = (dc != 0) ? (dc > 0 ? 1 : -1) : 0;
            for (int r = fromRow + stepR, c = fromCol + stepC; r != toRow || c != toCol; r += stepR, c += stepC) {
                if (board[r][c] != -1) return false; // Path is blocked
            }
            return true;
        }
        break;
    case 2: // Knight
        if ((abs(dr) == 2 && abs(dc) == 1) || (abs(dr) == 1 && abs(dc) == 2)) {
            return true; // L-shaped move
        }
        break;
    case 3: // Bishop
        if (abs(dr) == abs(dc)) { // Diagonal move
            int stepR = (dr > 0) ? 1 : -1;
            int stepC = (dc > 0) ? 1 : -1;
            for (int r = fromRow + stepR, c = fromCol + stepC; r != toRow || c != toCol; r += stepR, c += stepC) {
                if (board[r][c] != -1) return false; // Path is blocked
            }
            return true;
        }
        break;
    case 4: // Queen
        if (dc == 0 || dr == 0 || abs(dr) == abs(dc)) { // Horizontal, vertical, or diagonal move
            int stepR = (dr != 0) ? (dr > 0 ? 1 : -1) : 0;
            int stepC = (dc != 0) ? (dc > 0 ? 1 : -1) : 0;
            for (int r = fromRow + stepR, c = fromCol + stepC; r != toRow || c != toCol; r += stepR, c += stepC) {
                if (board[r][c] != -1) return false; // Path is blocked
            }
            return true;
        }
        break;
    case 5: // King
        if (abs(dr) <= 1 && abs(dc) <= 1) return true;
        // Castling
        if (isWhite && fromRow == 7 && fromCol == 4 && toRow == 7) {
            // Kingside castling
            if (toCol == 6 && !whiteKingMoved && !whiteKingsideRookMoved &&
                board[7][5] == -1 && board[7][6] == -1) {
                return true;
            }
            // Queenside castling
            if (toCol == 2 && !whiteKingMoved && !whiteQueensideRookMoved &&
                board[7][1] == -1 && board[7][2] == -1 && board[7][3] == -1) {
                return true;
            }
        }
        if (!isWhite && fromRow == 0 && fromCol == 4 && toRow == 0) {
            // Kingside castling
            if (toCol == 6 && !blackKingMoved && !blackKingsideRookMoved &&
                board[0][5] == -1 && board[0][6] == -1 && !checkForCheck(board, false)) {
                return true;
            }
            // Queenside castling
            if (toCol == 2 && !blackKingMoved && !blackQueensideRookMoved &&
                board[0][1] == -1 && board[0][2] == -1 && board[0][3] == -1 && !checkForCheck(board, false)) {
                return true;
            }
        }
        break;
    }
    return false;
}

void movePiece(int board[8][8], int fromRow, int fromCol, int toRow, int toCol) {
    int piece = board[fromRow][fromCol];
    if (isValidMove(board, fromRow, fromCol, toRow, toCol)) {
        // Update castling flags
        if (piece == 11) whiteKingMoved = true;
        if (piece == 5) blackKingMoved = true;
        if (piece == 7 && fromRow == 7 && fromCol == 7) whiteKingsideRookMoved = true;
        if (piece == 7 && fromRow == 7 && fromCol == 0) whiteQueensideRookMoved = true;
        if (piece == 1 && fromRow == 0 && fromCol == 7) blackKingsideRookMoved = true;
        if (piece == 1 && fromRow == 0 && fromCol == 0) blackQueensideRookMoved = true;

        // Handle castling rook move
        if (piece == 11 && fromRow == 7 && fromCol == 4 && toRow == 7 && toCol == 6) {
            // White kingside castling
            board[7][5] = board[7][7];
            board[7][7] = -1;
        }
        if (piece == 11 && fromRow == 7 && fromCol == 4 && toRow == 7 && toCol == 2) {
            // White queenside castling
            board[7][3] = board[7][0];
            board[7][0] = -1;
        }
        if (piece == 5 && fromRow == 0 && fromCol == 4 && toRow == 0 && toCol == 6) {
            // Black kingside castling
            board[0][5] = board[0][7];
            board[0][7] = -1;
        }
        if (piece == 5 && fromRow == 0 && fromCol == 4 && toRow == 0 && toCol == 2) {
            // Black queenside castling
            board[0][3] = board[0][0];
            board[0][0] = -1;
        }

        board[toRow][toCol] = piece;
        board[fromRow][fromCol] = -1; // Empty square
    } else {
        std::cerr << "Invalid move!" << std::endl;
    }
}

int main(int argc, char *argv[]) {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        std::cerr << "SDL_Init Error: " << SDL_GetError() << std::endl;
        SDL_Log("SDL_Init failed: %s", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    if (!TTF_Init()) {
        std::cerr << "TTF_Init Error: " << SDL_GetError() << std::endl;
        SDL_Log("TTF_Init failed: %s", SDL_GetError());
        TTF_Quit();
        SDL_Quit();
        return 1;
    }
    SDL_Window *window = SDL_CreateWindow("Chess", WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_RESIZABLE);
    if (!window) {
        std::cerr << "SDL_CreateWindow Error: " << SDL_GetError() << std::endl;
        SDL_Log("SDL_CreateWindow failed: %s", SDL_GetError());
        TTF_Quit();
        SDL_Quit();
        return 1;
    }
    SDL_Renderer *renderer = SDL_CreateRenderer(window, NULL);
    if (!renderer) {
        std::cerr << "SDL_CreateRenderer Error: " << SDL_GetError() << std::endl;
        SDL_Log("SDL_CreateRenderer failed: %s", SDL_GetError());
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
        return 1;
    }

    //init pieces
    SDL_Texture * pieces[12];
    pieces[0] = loadPiece(renderer, "res/pieces-png/black-pawn.png");    // 0: black pawn
    pieces[1] = loadPiece(renderer, "res/pieces-png/black-rook.png");    // 1: black rook
    pieces[2] = loadPiece(renderer, "res/pieces-png/black-knight.png");  // 2: black knight
    pieces[3] = loadPiece(renderer, "res/pieces-png/black-bishop.png");  // 3: black bishop
    pieces[4] = loadPiece(renderer, "res/pieces-png/black-queen.png");   // 4: black queen
    pieces[5] = loadPiece(renderer, "res/pieces-png/black-king.png");    // 5: black king
    pieces[6] = loadPiece(renderer, "res/pieces-png/white-pawn.png");    // 6: white pawn
    pieces[7] = loadPiece(renderer, "res/pieces-png/white-rook.png");    // 7: white rook
    pieces[8] = loadPiece(renderer, "res/pieces-png/white-knight.png");  // 8: white knight
    pieces[9] = loadPiece(renderer, "res/pieces-png/white-bishop.png");  // 9: white bishop
    pieces[10] = loadPiece(renderer, "res/pieces-png/white-queen.png");  // 10: white queen
    pieces[11] = loadPiece(renderer, "res/pieces-png/white-king.png");   // 11: white king

    // Piece indices:
    // 0: black pawn, 1: black rook, 2: black knight, 3: black bishop, 4: black queen, 5: black king
    // 6: white pawn, 7: white rook, 8: white knight, 9: white bishop, 10: white queen, 11: white king
    int board[8][8] = {
        {1, 2, 3, 4, 5, 3, 2, 1},      // Black: rook, knight, bishop, queen, king, bishop, knight, rook
        {0, 0, 0, 0, 0, 0, 0, 0},      // Black pawns
        { -1, -1, -1, -1, -1, -1, -1, -1 }, // Empty rows
        { -1, -1, -1, -1, -1, -1, -1, -1 },
        { -1, -1, -1, -1, -1, -1, -1, -1 },
        { -1, -1, -1, -1, -1, -1, -1, -1 },
        {6, 6, 6, 6, 6, 6, 6, 6},      // White pawns
        {7, 8, 9, 10, 11, 9, 8, 7}     // White: rook, knight, bishop, queen, king, bishop, knight, rook
    };

    //game loop
    bool running = true;
    SDL_Event event;

    
    int selectedRow = -1, selectedCol = -1; // Track selected piece
    while (running) {
        float mouseX, mouseY;
        SDL_GetMouseState(&mouseX, &mouseY);
        int hoverY = mouseY / (WINDOW_HEIGHT / 8);
        int hoverX = mouseX / (WINDOW_WIDTH / 8);
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT || event.type == SDL_EVENT_KEY_DOWN && event.key.key == SDLK_ESCAPE) {
                running = false;
            }
            // Handle mouse click for moving pieces
            if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
                SDL_GetMouseState(&mouseX, &mouseY);
                int row = mouseY / (WINDOW_HEIGHT / 8);
                int col = mouseX / (WINDOW_WIDTH / 8);
                if (selectedRow == -1 && selectedCol == -1) {
                    // Select piece
                    if (board[row][col] != -1) { // If there's a piece at the clicked position
                        selectedRow = row;
                        selectedCol = col;
                    }
                } else {
                    // Move piece
                    movePiece(board, selectedRow, selectedCol, row, col);
                    selectedRow = -1; // Deselect after moving
                    selectedCol = -1;
                }
            }
        }
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        //CREATE CHESS BOARD
        createChessBoard(renderer);
        //Draw pieces
        SDL_FRect hoverRect{hoverX * (WINDOW_WIDTH / 8), hoverY * (WINDOW_HEIGHT / 8), (WINDOW_WIDTH / 8), (WINDOW_HEIGHT / 8)};
        SDL_SetRenderDrawColor(renderer, 231, 255, 0, 0.2*255); // Highlight color
        SDL_RenderFillRect(renderer, &hoverRect);
        // Highlight selected piece
        if (selectedRow != -1 && selectedCol != -1) {
            SDL_FRect selectedRect{selectedCol * (WINDOW_WIDTH / 8), selectedRow * (WINDOW_HEIGHT / 8), (WINDOW_WIDTH / 8), (WINDOW_HEIGHT / 8)};
            SDL_SetRenderDrawColor(renderer, 255, 0, 0, 0.5*255); // Highlight color
            SDL_RenderFillRect(renderer, &selectedRect);
        }
        drawPieces(renderer, pieces, board);
        // Present the renderer
        SDL_RenderPresent(renderer);
        SDL_Delay(16); // Approx 60 FPS
    }
    for (int i = 0; i < 12; i++)
    {
        SDL_DestroyTexture(pieces[i]);  
    }
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();
    return 0;
}