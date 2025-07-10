#include <iostream>
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <string>

const int WINDOW_WIDTH = 600;
const int WINDOW_HEIGHT = 600;

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

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT || event.type == SDL_EVENT_KEY_DOWN && event.key.key == SDLK_ESCAPE) {
                running = false;
            }
        }
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        //CREATE CHESS BOARD
        createChessBoard(renderer);

        //Draw pieces
        drawPieces(renderer, pieces, board); // Draw all pieces

        SDL_RenderPresent(renderer);
        SDL_Delay(16); // Approx 60 FPS
    }
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();
    return 0;
}