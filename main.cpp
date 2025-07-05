// Test rebuild
#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>

// Tetromino definitions: 4 rotation states per piece
const std::vector<std::vector<std::string>> tetrominoes = {
    {
        // I piece
        "...."
        "####"
        "...."
        "....",

        "..#."
        "..#."
        "..#."
        "..#.",

        "...."
        "...."
        "####"
        "....",

        ".#.."
        ".#.."
        ".#.."
        ".#.."
    },
    {
        // O piece
        ".##."
        ".##."
        "...."
        "....",

        ".##."
        ".##."
        "...."
        "....",

        ".##."
        ".##."
        "...."
        "....",

        ".##."
        ".##."
        "...."
        "...."
    },
};


// Board dimensions
const int WIDTH = 10;
const int HEIGHT = 20;

// Board grid
std::vector<std::vector<char>> board(HEIGHT, std::vector<char>(WIDTH, ' '));

// Piece position
int pieceX = WIDTH / 2 - 1;
int pieceY = 0;

int currentPiece = 0; // 0 = I, 1 = O
int currentRotation = 0;

// Terminal input handling
void enableRawMode() {
    termios term;
    tcgetattr(STDIN_FILENO, &term);
    term.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &term);
}

void disableRawMode() {
    termios term;
    tcgetattr(STDIN_FILENO, &term);
    term.c_lflag |= ICANON | ECHO;
    tcsetattr(STDIN_FILENO, TCSANOW, &term);
}

bool kbhit() {
    termios oldt, newt;
    int ch;
    int oldf;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);
    ch = getchar();
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    fcntl(STDIN_FILENO, F_SETFL, oldf);
    if (ch != EOF) {
        ungetc(ch, stdin);
        return true;
    }
    return false;
}

void render() {
    std::cout << "\033[2J\033[1;1H";
    // Temporary board copy
    std::vector<std::vector<char>> temp = board;

    // Get the current tetromino shape
    std::string shape = tetrominoes[currentPiece][currentRotation];

    // Overlay the tetromino
    for (int py = 0; py < 4; py++) {
        for (int px = 0; px < 4; px++) {
            if (shape[py * 4 + px] == '#') {
                int bx = pieceX + px;
                int by = pieceY + py;
                if (by >= 0 && by < HEIGHT && bx >= 0 && bx < WIDTH) {
                    temp[by][bx] = 'O';
                }
            }
        }
    }

    // Draw the board
    for (int y = 0; y < HEIGHT; y++) {
        std::cout << "|";
        for (int x = 0; x < WIDTH; x++) {
            std::cout << temp[y][x];
        }
        std::cout << "|" << std::endl;
    }
    for (int i = 0; i < WIDTH + 2; i++) std::cout << "-";
    std::cout << std::endl;
}

bool canMove(int dx, int dy) {
    int newX = pieceX + dx;
    int newY = pieceY + dy;
    if (newX < 0 || newX >= WIDTH || newY >= HEIGHT)
        return false;
    if (board[newY][newX] != ' ')
        return false;
    return true;
}

void lockPiece() {
    std::string shape = tetrominoes[currentPiece][currentRotation];
    for (int py = 0; py < 4; py++) {
        for (int px = 0; px < 4; px++) {
            if (shape[py * 4 + px] == '#') {
                int bx = pieceX + px;
                int by = pieceY + py;
                if (by >= 0 && by < HEIGHT && bx >= 0 && bx < WIDTH) {
                    board[by][bx] = 'O';
                }
            }
        }
    }
}

bool doesPieceFit(int tetromino, int rotation, int posX, int posY) {
    std::string shape = tetrominoes[tetromino][rotation];
    for (int py = 0; py < 4; py++) {
        for (int px = 0; px < 4; px++) {
            if (shape[py * 4 + px] == '#') {
                int bx = posX + px;
                int by = posY + py;

                // Check bounds
                if (bx < 0 || bx >= WIDTH || by >= HEIGHT) {
                    return false;
                }
                // Check collision with existing blocks
                if (by >= 0 && board[by][bx] != ' ') {
                    return false;
                }
            }
        }
    }
    return true;
}

int main() {
    enableRawMode();
    bool running = true;
    auto lastDrop = std::chrono::steady_clock::now();
    while (running) {
        render();
        if (kbhit()) {
            char c = getchar();
 
    	    switch (c) {
	        case 'a':
	            if (doesPieceFit(currentPiece, currentRotation, pieceX - 1, pieceY))
	                pieceX--;
	            break;
	        case 'd':
	            if (doesPieceFit(currentPiece, currentRotation, pieceX + 1, pieceY))
	                pieceX++;
	            break;
	        case 's':
	            if (doesPieceFit(currentPiece, currentRotation, pieceX, pieceY + 1))
	                pieceY++;
	            break;
	        case 'w':
	        {
	            int nextRotation = (currentRotation + 1) % 4;
	            if (doesPieceFit(currentPiece, nextRotation, pieceX, pieceY))
	                currentRotation = nextRotation;
	        }
	        break;
	        case 'q':
	            running = false;
	            break;
	    }
        }
        auto now = std::chrono::steady_clock::now();
        if (std::chrono::duration_cast<std::chrono::milliseconds>(now - lastDrop).count() > 500) {
	    if (doesPieceFit(currentPiece, currentRotation, pieceX, pieceY + 1)) {
	        pieceY++;
	    } else {
	        lockPiece();
	        // Next random piece
	        currentPiece = rand() % tetrominoes.size();     
	        currentRotation = 0;
	        pieceX = WIDTH / 2 - 2;
	        pieceY = 0;
	    }
            lastDrop = now;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
    disableRawMode();
    std::cout << "Thanks for playing!" << std::endl;
    return 0;
}

