// Test rebuild
#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>

// Board dimensions
const int WIDTH = 10;
const int HEIGHT = 20;

// Board grid
std::vector<std::vector<char>> board(HEIGHT, std::vector<char>(WIDTH, ' '));

// Piece position
int pieceX = WIDTH / 2 - 1;
int pieceY = 0;

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
    system("clear");
    // Temporary board with piece
    std::vector<std::vector<char>> temp = board;
    temp[pieceY][pieceX] = 'O';
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
    board[pieceY][pieceX] = 'O';
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
                    if (canMove(-1,0)) pieceX--;
                    break;
                case 'd':
                    if (canMove(1,0)) pieceX++;
                    break;
                case 's':
                    if (canMove(0,1)) pieceY++;
                    break;
                case 'q':
                    running = false;
                    break;
            }
        }
        auto now = std::chrono::steady_clock::now();
        if (std::chrono::duration_cast<std::chrono::milliseconds>(now - lastDrop).count() > 500) {
            if (canMove(0,1)) {
                pieceY++;
            } else {
                lockPiece();
                pieceX = WIDTH / 2 - 1;
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

