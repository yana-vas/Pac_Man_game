#include <iostream>
#include <fstream>
#include <windows.h>
#include <cmath>
#include <ctime>
#include <cstdlib>


bool gameOver = false;
bool frightenedMode = false;
bool switchMode = false;

int frightenedCounter = 0;

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

// { {-1, 0}, {0, -1}, {1, 0}, {0, 1} }; // Up, Left, Down, Right ({r, c})
int BprevCol = 0;
int BprevRow = 1;
char BprevTile = ' ';

int PprevCol = 0;
int PprevRow = 1;
char PprevTile = ' ';

int IprevCol = 1;
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

void cleanupMatrix() {
    if (!matrix) return;
    for (int i = 0; i < rows; ++i) {
        if (matrix[i]) {
            delete[] matrix[i];
        }
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

bool isValidMove(int newRow, int newCol) {
    return (newRow >= 0 && newRow < rows && newCol >= 0 && newCol < cols &&
        (matrix[newRow][newCol] == '-' || matrix[newRow][newCol] == '@' || matrix[newRow][newCol] == ' '));
}

void updateFrightenedMode(bool& frightenedMode, int& frightenedCounter) {
    if (frightenedMode) {
        frightenedCounter++;
        if (frightenedCounter == 10) {
            frightenedMode = false;
            frightenedCounter = 0;
        }
    }
}

void calculateNewPosition(int& newRow, int& newCol, char direction, int movementStep) {
    if (direction == 'w' || direction == 'W') { // Up
        newRow -= movementStep;
    }
    else if (direction == 'a' || direction == 'A') { // Left
        newCol -= movementStep;
    }
    else if (direction == 's' || direction == 'S') { // Down
        newRow += movementStep;
    }
    else if (direction == 'd' || direction == 'D') { // Right
        newCol += movementStep;
    }
}

bool isValidPosition(int row, int col) {
    return row >= 0 && row < rows && col >= 0 && col < cols;
}

void handleMove(int& pacRow, int& pacCol, int newRow, int newCol, int& score, char pacManCh) {
    if (!isValidPosition(newRow, newCol)) {
        std::cerr << "Error: Invalid move to (" << newRow << ", " << newCol << ")" << std::endl;
        return;
    }

    char target = matrix[newRow][newCol];
    matrix[pacRow][pacCol] = ' '; // Clear old position
    matrix[newRow][newCol] = pacManCh; // Update new position

    pacRow = newRow;
    pacCol = newCol;

    
    if (target == '-' || target == '@') {
        score++;
    }
}

void movePacMan(int& pacRow, int& pacCol, char direction, int& score, bool& frightenedMode) {
    int newRow = pacRow;
    int newCol = pacCol;

    // Calculate movement based on direction
    int movementStep = (frightenedMode) ? 2 : 1;
    int stepsCounter = 0;
    while (movementStep--) {
        //r, c, d
        int tempRow = newRow;
        int tempCol = newCol;
        calculateNewPosition(tempRow, tempCol, direction, 1);
        if (isValidMove(tempRow, tempCol)) {
            newRow = tempRow;
            newCol = tempCol;

            char target = matrix[newRow][newCol];

            // Check for collision with enemies
            if (target == 'B' || target == 'P' || target == 'C' || target == 'I') {
                gameOver = true;
                return;
            }

            switchMode = false;
            // Activate frightened mode
            if (target == '@') {
                frightenedMode = true;
                frightenedCounter = 0;
                switchMode = true;
            }

            // Update frightened mode
            updateFrightenedMode(frightenedMode, frightenedCounter);

            handleMove(pacRow, pacCol, newRow, newCol, score, pacManCh);
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

    int directions[4][2] = { {-1, 0}, {0, -1}, {1, 0}, {0, 1} }; // Up, Left, Down, Right ({r, c})
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
        if (dcol == -opp_col && drow == -opp_row) continue;

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
        for (int i = 0; i < 4; ++i) { // Iterate through all directions
            int drow = directions[i][0];
            int dcol = directions[i][1];
            int nrow = GhRow + drow;
            int ncol = GhCol + dcol;

            if (isValidPosition(nrow, ncol) && matrix[nrow][ncol] != '#') {
                best_drow = drow;
                best_dcol = dcol;
                break;
            }
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

void frightenedMoveGhost(int& GhRow, int& GhCol, int& GhPrevCol, int& GhPrevRow, char& GhPrevTile, char ghChar) {

    // Directions: 0 = Up, 1 = Left, 2 = Down, 3 = Right
    int directions[4][2] = { {-1, 0}, {0, -1}, {1, 0}, {0, 1} }; // {drow, dcol}
    int opp_row = GhRow - GhPrevRow; // Opposite movement row delta
    int opp_col = GhCol - GhPrevCol; // Opposite movement column delta

    bool valid_move_found = false;
    int chosen_direction = -1; // To store the chosen direction


    int retries = 10; // Limit attempts to 10
    while (!valid_move_found && retries-- > 0) {
        // Pick a random direction (0 to 3)
        chosen_direction = rand() % 4;
        int drow = directions[chosen_direction][0];
        int dcol = directions[chosen_direction][1];

        // Calculate the new position
        int nrow = GhRow + drow;
        int ncol = GhCol + dcol;

        // Check if the move is valid
        if (drow == -opp_row && dcol == -opp_col) continue; // Prevent reverse movement
        if (nrow < 0 || nrow >= rows || ncol < 0 || ncol >= cols) continue; // Out of bounds
        if (matrix[nrow][ncol] == '#') {
            valid_move_found = true;
            continue;
        }

        // Handle collisions with other ghosts or special tiles
        switch (matrix[nrow][ncol]) {
        case 'B': // Blinky
            nrow = 1; ncol = cols - 1; break;
        case 'P': // Pinky
            nrow = 1; ncol = 1; break;
        case 'I': // Inky
            nrow = rows - 1; ncol = cols - 1; break;
        case 'C': // Clyde
            nrow = rows - 1; ncol = 1; break;
        }

        // If all checks pass, the move is valid
        valid_move_found = true;

        // Update the ghost's position
        matrix[GhPrevRow][GhPrevCol] = GhPrevTile; // Restore the previous tile
        GhPrevTile = matrix[nrow][ncol]; // Save the new previous tile
        matrix[nrow][ncol] = ghChar; // Place the ghost in the new position

        // Update ghost's coordinates
        GhPrevRow = GhRow;
        GhPrevCol = GhCol;
        GhRow = nrow;
        GhCol = ncol;
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

}

void activateI(int& IRow, int& ICol, int pacRow, int pacCol, int BRow, int BCol, char pacOrientation) {
    int referenceRow = pacRow;
    int referenceCol = pacCol;

    if (pacOrientation == 'w' || pacOrientation == 'W') { // Up
        referenceRow -= 2;
        referenceCol -= 2;
    }
    else if (pacOrientation == 's' || pacOrientation == 'S') { // Down
        referenceRow += 2;
    }
    else if (pacOrientation == 'a' || pacOrientation == 'A') { // Left
        referenceCol -= 2;
    }
    else if (pacOrientation == 'd' || pacOrientation == 'D') { // Right
        referenceCol += 2;
    }

    int vectorRow = referenceRow - BRow;
    int vectorCol = referenceCol - BCol;

    vectorRow *= 2;
    vectorCol *= 2;

    
    int targetRow = BRow + vectorRow;
    int targetCol = BCol + vectorCol;

    if (targetRow < 0) targetRow = 0;
    else if (targetRow >= rows) targetRow = rows - 1;

    if (targetCol < 0) targetCol = 0;
    else if (targetCol >= cols) targetCol = cols - 1;

    moveGhost(IRow, ICol, targetCol, targetRow, PprevCol, PprevRow, PprevTile, PinkyCh);

}

void activateC(int& GhRow, int& GhCol, int pacRow, int pacCol, int& GhPrevRow, int& GhPrevCol, char& GhPrevTile, char ghChar) {
    double distance = std::sqrt((GhCol - pacCol) * (GhCol - pacCol) + (GhRow - pacRow) * (GhRow - pacRow));

    if (distance >= 8) {
        // Chase Pac-Man directly
        moveGhost(GhRow, GhCol, pacRow, pacCol, GhPrevRow, GhPrevCol, GhPrevTile, ghChar);
    }
    else {
        // Move toward lower-left corner
        moveGhost(GhRow, GhCol, rows - 1, 0, GhPrevRow, GhPrevCol, GhPrevTile, ghChar);
    }
}

void runGame(int& score, char& pacOrientation,
    int& pacRow, int& pacCol, int& BRow, int& BCol, int& PRow, int& PCol,
    int& IRow, int& ICol, int& CRow, int& CCol) {
    if (pacRow == -1 || pacCol == -1) {
        std::cerr << "Error: Pac-Man not found in the map." << std::endl;
        cleanupMatrix();
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

        if (direction != 'q' && direction != 'w' && direction != 'a' && direction != 's' && direction != 'd') {
            std::cout << "\nInvalid direction! \nEnter direction (w/a/s/d) or q to quit: ";
            std::cin >> direction;
        }

        if (direction == 'q') {
            break;
        }

        pacOrientation = direction;
        movePacMan(pacRow, pacCol, direction, score, frightenedMode);
        
        if (gameOver) {
            std::cout << "Game Over! A ghost caught Pac-Man!" << std::endl;
            break;
        }

        if (frightenedMode) {
            if (switchMode) {
                BprevRow *= -1;
                BprevCol *= -1;

                PprevRow *= -1;
                PprevCol *= -1;

                IprevRow *= -1;
                IprevCol *= -1;

                CprevRow *= -1;
                CprevCol *= -1;
                switchMode = false;
            }
            frightenedMoveGhost(BRow, BCol, BprevCol, BprevRow, BprevTile, BlinkyCh);
            frightenedMoveGhost(PRow, PCol, PprevCol, PprevRow, PprevTile, PinkyCh);
            frightenedMoveGhost(IRow, ICol, IprevCol, IprevRow, IprevTile, InkyCh);
            frightenedMoveGhost(CRow, CCol, CprevCol, CprevRow, CprevTile, ClydeCh);
            continue;
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
                activateI(IRow, ICol, pacRow, pacCol, BRow, BCol, pacOrientation);
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
                activateC(CRow, CCol, pacRow, pacCol, CprevRow, CprevCol, CprevTile, ClydeCh);
            }
        }

        if (gameOver) {
            std::cout << "Game Over! A ghost caught Pac-Man!" << std::endl;
            break;
        }
    }

    cleanupMatrix();
}



int main() {
    const char* mapPath = "C:\\Users\\PC1\\source\\repos\\Pac_Man_game\\map.txt";

    loadMap(mapPath, matrix, rows, cols);

    if (matrix) {
        srand(time(0));

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
            cleanupMatrix();
            return 1;
        }
        else {
            runGame(currScore, pacOrientation, pacRow, pacCol, BRow, BCol, PRow, PCol, IRow, ICol, CRow, CCol);
        }

        cleanupMatrix();
    }
    else {
        std::cerr << "Error: Failed to load the map!" << std::endl;
    }

    return 0;
}