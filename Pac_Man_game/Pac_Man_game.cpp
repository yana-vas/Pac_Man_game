#include <iostream>
#include <fstream>
#include <windows.h>
#include <cmath>

const char pacManCh = 'Y';
const char BlinkyCh = 'B';
const char PinkyCh = 'P';
const char InkyCh = 'I';
const char ClydeCh = 'C';

// Cage exit coordinates
const int CAGE_EXIT_ROW = 8;
const int CAGE_EXIT_COL = 9;

// Directions: Up, Down, Left, Right
const int dx[] = { -1, 1, 0, 0 };
const int dy[] = { 0, 0, -1, 1 };
//const char dirChars[] = { 'U', 'D', 'L', 'R' };

char** matrix = nullptr;
int rows = 0, cols = 0;
int currScore = 0;


void loadMap(const char* filePath, char**& matrix, int& rows, int& cols) {
    std::ifstream file(filePath);

    if (!file.is_open()) {
        std::cerr << "Error: Unable to open file " << filePath << std::endl;
        rows = cols = 0;
        matrix = nullptr;
        return;
    }

    // Dimensions
    file >> rows >> cols;

    // Ignore newline character after dimensions
    file.ignore();

    matrix = new char* [rows];
    for (int i = 0; i < rows; ++i) {
        matrix[i] = new char[cols + 1]; // +1 for null terminator
    }

    for (int i = 0; i < rows; ++i) {
        file.getline(matrix[i], cols + 1);
    }

    file.close();
}

void cleanupMatrix(char** matrix, int rows) {
    for (int i = 0; i < rows; ++i) {
        delete[] matrix[i];
    }
    delete[] matrix;
}

bool isInsideCage(int x, int y) {
    const int CAGE_TOP_ROW = 7;    // Top row of the cage
    const int CAGE_BOTTOM_ROW = 9; // Bottom row of the cage
    const int CAGE_LEFT_COL = 7;   // Left column of the cage
    const int CAGE_RIGHT_COL = 11; // Right column of the cage

    return (x >= CAGE_TOP_ROW && x <= CAGE_BOTTOM_ROW && y >= CAGE_LEFT_COL && y <= CAGE_RIGHT_COL);
}

void findCharacter(char** matrix, int rows, int cols, int& chRow, int& chCol, char ch) {
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            if (matrix[i][j] == ch) {
                chRow = i;
                chCol = j;
                return;
            }
        }
    }
    chRow = -1;
    chCol = -1;
}

void movePacMan(char** matrix, int rows, int cols, int& pacRow, int& pacCol, char direction, int& score, bool& frightenedMode) {
    int newRow = pacRow;
    int newCol = pacCol;

    if (direction == 'w' || direction == 'W') { // Up
        newRow--;
    }
    else if (direction == 'a' || direction == 'A') { // Left
        newCol--;
    }
    else if (direction == 's' || direction == 'S') { // Down
        newRow++;
    }
    else if (direction == 'd' || direction == 'D') { // Right
        newCol++;
    }

    // Validate the new position
    if (newRow >= 0 && newRow < rows && newCol >= 0 && newCol < cols) {
        char target = matrix[newRow][newCol];
        if (target == '-' || target == '@' || target == ' ') {
            // Update positions
            matrix[pacRow][pacCol] = ' ';
            matrix[newRow][newCol] = pacManCh;

            pacRow = newRow;
            pacCol = newCol;

            if (target == '-' || target == '@') {
                score++;
            }
        }
    }
}

void printMatrix(char** matrix, int rows, int cols) {
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            std::cout << matrix[i][j];
        }
        std::cout << std::endl;
    }
}

void clearScreen() {
    system("cls");
}

int maxScore(char** matrix, int rows, int cols) {
    int counter = 0;
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            if (matrix[i][j] == '-' || matrix[i][j] == '@') {
                counter++;
            }
        }
    }
    return counter;
}

bool isGhostCh(char** matrix, int newX, int newY) {
    return matrix[newX][newY] == BlinkyCh && matrix[newX][newY] == PinkyCh && matrix[newX][newY] == InkyCh && matrix[newX][newY] == ClydeCh;
}

void moveGhost(char** matrix, int rows, int cols, int& ghostRow, int& ghostCol, int targetRow, int targetCol, char ghostChar, int& lastDirection, char& prevTile) {
    double minDistance = INFINITY;
    int bestMoveX = ghostRow, bestMoveY = ghostCol;
    int bestDirection = -1;


}

void exitCageB(char** matrix, int rows, int cols, int& ghostRow, int& ghostCol, bool& exitedCage) {
    //up    
    ghostRow -= 1;
    exitedCage = true;
}

void activateB(int& bx, int& by, int& prevX, int& prevY, int pacX, int pacY, bool& gameOver, int& lastDirectionB, char& prevTileB) {
    int bestCol = 0, bestRow = 0;

    // - Blinky's current position: (bx, by)
    // - Blinky's previous position: (prevX, prevY)
    // - Pac-Man's position: (px, py)
    // - Game map: a 2D array (0 = empty, 1 = wall, 'C', 'I', 'P' = other ghosts)

    int directions[4][2] = { {0, -1}, {0, 1}, {-1, 0}, {1, 0} }; // Up, Down, Left, Right
    int opp_x = bx - prevX;
    int opp_y = by - prevY;

    int best_dx = 0, best_dy = 0;
    double min_distance = 1e9;
    bool valid_move_found = false; 

    // Loop through all directions
    for (int i = 0; i < 4; i++) {
        int dx = directions[i][0];
        int dy = directions[i][1];

        // New position after the move
        int nx = bx + dx;
        int ny = by + dy;

        
        if (dx == opp_x && dy == opp_y) continue;

        
        if (matrix[nx][ny] == '#' || matrix[nx][ny] == 'C' || matrix[nx][ny] == 'I' || matrix[nx][ny] == 'P') continue;

        valid_move_found = true;
        
        double distance = sqrt(( - nx) * (pacX - nx) + (pacY - ny) * (pacY - ny));

        
        if (distance < min_distance) {
            min_distance = distance;
            best_dx = dx;
            best_dy = dy;
        }
    }

    if (!valid_move_found) {
        best_dx = opp_x;
        best_dy = opp_y;
    }

    prevX = bx;
    prevY = by;
    bx += best_dx;
    by += best_dy;

    moveGhost(matrix, rows, cols, bx, by, pacX, pacY, BlinkyCh, lastDirectionB, prevTileB);
}


void activateP(int& PRow, int& PCol, int pacRow, int pacCol, char pacOrientation, bool& gameOver, int& lastDirectionP, char& prevTileP) {
    // Pinky targets 4 tiles ahead of Pac-Man in the direction Pac-Man is facing
    int targetRow = pacRow, targetCol = pacCol;
    if (pacOrientation == 'w' || pacOrientation == 'W') { // Up
        targetRow -= 4;
    }
    else if (pacOrientation == 'a' || pacOrientation == 'A') { // Left
        targetCol -= 4;
    }
    else if (pacOrientation == 's' || pacOrientation == 'S') { // Down
        targetRow += 4;
    }
    else if (pacOrientation == 'd' || pacOrientation == 'D') { // Right
        targetCol += 4;
    }

    // Ensure target is within bounds
    if (targetRow < 0) targetRow = 0;
    else if (targetRow >= rows) targetRow = rows - 1;

    if (targetCol < 0) targetCol = 0;
    else if (targetCol >= cols) targetCol = cols - 1;

    moveGhost(matrix, rows, cols, PRow, PCol, targetRow, targetCol, PinkyCh, lastDirectionP, prevTileP);

    // Check if Pinky caught Pac-Man
    if (PRow == pacRow && PCol == pacCol) {
        gameOver = true;
    }
}

void runGame(char** matrix, int rows, int cols, int& score, char& pacOrientation,
    int& pacRow, int& pacCol, int& BRow, int& BCol, int& PRow, int& PCol,
    int& IRow, int& ICol, int& CRow, int& CCol) {
    if (pacRow == -1 || pacCol == -1) {
        std::cerr << "Error: Pac-Man not found in the map." << std::endl;
        cleanupMatrix(matrix, rows);
        exit(1);
    }

    bool frightenedMode = false;
    bool gameOver = false;
    bool BhasExitedCage = false; // Track if Blinky has exited the cage

    // Last move directions for ghosts
    int lastDirectionB = -1, lastDirectionP = -1, lastDirectionI = -1, lastDirectionC = -1;

    // Previous tiles for ghosts (by default ' ')
    char prevTileB = ' ', prevTileP = ' ', prevTileI = ' ', prevTileC = ' ';

    while (!gameOver) {
        clearScreen();
        printMatrix(matrix, rows, cols);

        // Display score
        std::cout << "Score: " << score << std::endl;
        std::cout << "\nEnter direction (w/a/s/d) or q to quit: ";
        char direction;
        std::cin >> direction;

        if (direction == 'q') {
            break;
        }

        pacOrientation = direction; // Update Pac-Man's orientation
        movePacMan(matrix, rows, cols, pacRow, pacCol, direction, score, frightenedMode);

        // Activate ghosts
        exitCageB(matrix, rows, cols, BRow, BCol, BhasExitedCage);
        activateB(matrix, rows, cols, BRow, BCol, pacRow, pacCol, gameOver, BhasExitedCage, lastDirectionB, prevTileB);
        /*if (score >= 20) {
            activateP(matrix, rows, cols, PRow, PCol, pacRow, pacCol, pacOrientation, gameOver, lastDirectionP, prevTileP);
        }*/
        // Add Inky and Clyde activation logic here

        if (gameOver) {
            std::cout << "Game Over! A ghost caught Pac-Man!" << std::endl;
            break;
        }
    }

    cleanupMatrix(matrix, rows);
}



int main() {
    const char* mapPath = "C:\\Users\\PC1\\source\\repos\\Pac_Man_game\\map.txt";

    loadMap(mapPath, matrix, rows, cols);

    if (matrix) {
        int goalScore = maxScore(matrix, rows, cols);

        int pacRow = 0, pacCol = 0;
        findCharacter(matrix, rows, cols, pacRow, pacCol, 'Y');
        char pacOrientation = 'd'; // Default value

        int BRow = 0, BCol = 0;
        findCharacter(matrix, rows, cols, BRow, BCol, 'B');

        int PRow = 0, PCol = 0;
        findCharacter(matrix, rows, cols, PRow, PCol, 'P');

        int IRow = 0, ICol = 0;
        findCharacter(matrix, rows, cols, IRow, ICol, 'I');

        int CRow = 0, CCol = 0;
        findCharacter(matrix, rows, cols, CRow, CCol, 'C');

        // Validate
        if (pacRow == -1 || pacCol == -1 ||
            BRow == -1 || BCol == -1 ||
            PRow == -1 || PCol == -1 ||
            IRow == -1 || ICol == -1 ||
            CRow == -1 || CCol == -1) {
            std::cerr << "Error: Characters not found on the map!" << std::endl;
        }
        else {
            runGame(matrix, rows, cols, currScore, pacOrientation, pacRow, pacCol, BRow, BCol, PRow, PCol, IRow, ICol, CRow, CCol);
        }

        cleanupMatrix(matrix, rows);
    }
    else {
        std::cerr << "Error: Failed to load the map!" << std::endl;
    }

    return 0;
}