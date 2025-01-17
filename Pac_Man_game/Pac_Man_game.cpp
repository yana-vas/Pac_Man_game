#include <iostream>
#include <fstream>
#include <windows.h>
#include <cmath>


bool gameOver = false;

const char pacManCh = 'Y';
const char BlinkyCh = 'B';
const char PinkyCh = 'P';
const char InkyCh = 'I';
const char ClydeCh = 'C';

// Cage exit coordinates
const int CAGE_EXIT_ROW = 7;
const int CAGE_EXIT_COL = 9;


char** matrix = nullptr;
int rows = 0, cols = 0;
int currScore = 0;


int BprevCol = 0;
int BprevRow = 1;
char BprevTile = ' ';

int PprevCol = -1;
int PprevRow = 0;
char PprevTile = ' ';

int IprevCol = -1;
int IprevRow = 0;
char IprevTile = ' ';

int CprevCol = -1;
int CprevRow = 0;
char CprevTile = ' ';


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

void findCharacter(int& chRow, int& chCol, char ch) {
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

void movePacMan(int& pacRow, int& pacCol, char direction, int& score, bool& frightenedMode) {
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

    
    if (newRow >= 0 && newRow < rows && newCol >= 0 && newCol < cols) {
        char target = matrix[newRow][newCol];
        if ((target == 'B') || (target == 'P') || (target == 'C') || (target == 'I')) {
            gameOver = true;
        }
        if (target == '-' || target == '@' || target == ' ') {
        
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

void exitCageUP(int& ghostRow, int& ghostCol, bool& exitedCage, char GhChar, char& GhPrevTile, int& GhPrevCol, int& GhPrevRow) {
    //up    
    int addRow = -1;


    matrix[ghostRow][ghostCol] = GhPrevTile;

    GhPrevTile = matrix[ghostRow + addRow][ghostCol];
    ghostRow += addRow;
    matrix[ghostRow][ghostCol] = GhChar;

    GhPrevRow = addRow;

    if (ghostRow == CAGE_EXIT_ROW && ghostCol == CAGE_EXIT_COL) {
        exitedCage = true;
    }
}

void exitCageLeft(int& ghostRow, int& ghostCol, bool& exitedCage, char GhChar, char& GhPrevTile, int& GhPrevCol, int& GhPrevRow) {
    //left    
    int addCol = -1;


    matrix[ghostRow][ghostCol] = GhPrevTile;

    GhPrevTile = matrix[ghostRow][ghostCol + addCol];
    ghostCol += addCol;
    matrix[ghostRow][ghostCol] = GhChar;

    GhPrevCol = addCol;

    if (ghostRow == CAGE_EXIT_ROW && ghostCol == CAGE_EXIT_COL) {
        exitedCage = true;
    }
}

void exitCageRight(int& ghostRow, int& ghostCol, bool& exitedCage, char GhChar, char& GhPrevTile, int& GhPrevCol, int& GhPrevRow) {
    //left    
    int addCol = 1;


    matrix[ghostRow][ghostCol] = GhPrevTile;

    GhPrevTile = matrix[ghostRow][ghostCol + addCol];
    ghostCol += addCol;
    matrix[ghostRow][ghostCol] = GhChar;

    GhPrevCol = addCol;

    if (ghostRow == CAGE_EXIT_ROW && ghostCol == CAGE_EXIT_COL) {
        exitedCage = true;
    }
}


void moveGhost(int& GhRow, int& GhCol, int targetCol, int targetRow, int& GhPrevCol, int& GhPrevRow, char& GhPrevTile, char ghChar) {

    int directions[4][2] = { {-1, 0}, {1, 0}, {0, -1}, {0, 1} }; // Up, Down, Left, Right ({r, c})
    int opp_col = GhCol - GhPrevCol;
    int opp_row = GhRow - GhPrevRow;

    int best_dcol = 0, best_drow = 0;
    double min_distance = 1e9;
    bool valid_move_found = false;

    // Loop through all directions
    for (int i = 0; i < 4; i++) {
        int drow = directions[i][0];
        int dcol = directions[i][1];

        // New position after the move
        int ncol = GhCol + dcol;
        int nrow = GhRow + drow;

        // Skip the opposite direction
        if (ncol == opp_col && nrow == opp_row) continue;

        // Skip if the move leads to a wall or another ghost
        if (matrix[nrow][ncol] == '#' || matrix[nrow][ncol] == 'C' || matrix[nrow][ncol] == 'I' || matrix[nrow][ncol] == 'P') continue;

        valid_move_found = true;

        
        double distance = sqrt((targetCol - ncol) * (targetCol - ncol) + (targetRow - nrow) * (targetRow - nrow));

        
        if (distance < min_distance) {
            min_distance = distance;
            best_dcol = dcol;
            best_drow = drow;
        }
    }

    if (!valid_move_found) {
        int ncol = GhCol + opp_col;
        int nrow = GhRow + opp_row;
        if (ncol >= 0 && ncol < cols && nrow >= 0 && nrow < rows &&
            matrix[nrow][ncol] != '#' && matrix[nrow][ncol] != 'C' &&
            matrix[nrow][ncol] != 'I' && matrix[nrow][ncol] != 'P' && matrix[nrow][ncol] != 'B') {
            best_dcol = opp_col;
            best_drow = opp_row;
        }
        else {
            // TO-DO (if needed) handle the case where no valid move is found
        }
    }

    GhPrevCol = GhCol;
    GhPrevRow = GhRow;

    GhCol += best_dcol;
    GhRow += best_drow;

    
    matrix[GhPrevRow][GhPrevCol] = GhPrevTile;
    GhPrevTile = matrix[GhRow][GhCol];
    matrix[GhRow][GhCol] = ghChar;

    
    if (GhCol == targetCol && GhRow == targetRow) {
        gameOver = true;
    }
}


void activateP(int& PRow, int& PCol, int pacRow, int pacCol, char pacOrientation) {
    // Pinky targets 4 tiles ahead of Pac-Man in the direction Pac-Man is facing
    int targetRow = pacRow, targetCol = pacCol;
    if (pacOrientation == 'w' || pacOrientation == 'W') { // Up
        targetRow -= 4;
        targetCol -= 4;
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

    moveGhost(PRow, PCol, targetCol, targetRow, PprevCol, PprevRow, PprevTile, PinkyCh);

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
    bool BhasExitedCage = false;
    bool PhasExitedCage = false;
    bool IhasExitedCage = false;
    bool ChasExitedCage = false;

    bool Ileft = false;
    bool Cright = false;


    while (!gameOver) {
        clearScreen();
        printMatrix(matrix, rows, cols);

        // Display
        std::cout << "Score: " << score << std::endl;
        std::cout << "\nEnter direction (w/a/s/d) or q to quit: ";
        char direction;
        std::cin >> direction;

        if (direction == 'q') {
            break;
        }

        pacOrientation = direction;
        movePacMan(pacRow, pacCol, direction, score, frightenedMode);
        
        if (gameOver) {
            std::cout << "Game Over! A ghost caught Pac-Man!" << std::endl;
            break;
        }

        // Activate ghosts
        if (!BhasExitedCage) {
            exitCageUP(BRow, BCol, BhasExitedCage, BlinkyCh, BprevTile, BprevCol, BprevRow);
        }
        else {
            moveGhost(BRow, BCol, pacRow, pacCol, BprevCol, BprevRow, BprevTile, BlinkyCh);
        }
        if (score >= 20) {
            if (!PhasExitedCage) {
                exitCageUP(PRow, PCol, PhasExitedCage, PinkyCh, PprevTile, PprevCol, PprevRow);
            }
            else {
                activateP(PRow, PCol, pacRow, pacCol, pacOrientation);
            }
        }
        if (score >= 40) {
            if (!IhasExitedCage) {
                if (Ileft) {
                    exitCageUP(IRow, ICol, IhasExitedCage, InkyCh, IprevTile, IprevCol, IprevRow);
                }
                else {
                    exitCageLeft(IRow, ICol, IhasExitedCage, InkyCh, IprevTile, IprevCol, IprevRow);
                    Ileft = true;
                }
            }
            else {
                //
            }
        }
        if (score >= 60) {
            if (!ChasExitedCage) {
                if (Cright) {
                    exitCageUP(CRow, CCol, ChasExitedCage, ClydeCh, CprevTile, CprevCol, CprevRow);
                }
                else {
                    exitCageRight(CRow, CCol, ChasExitedCage, ClydeCh, CprevTile, CprevCol, CprevRow);
                    Cright = true;
                }
            }
            else {
                // TO-DO
            }
        }

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
        findCharacter(pacRow, pacCol, 'Y');
        char pacOrientation = 'd'; // Default value

        int BRow = 0, BCol = 0;
        findCharacter(BRow, BCol, 'B');

        int PRow = 0, PCol = 0;
        findCharacter(PRow, PCol, 'P');

        int IRow = 0, ICol = 0;
        findCharacter(IRow, ICol, 'I');


        int CRow = 0, CCol = 0;
        findCharacter(CRow, CCol, 'C');


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