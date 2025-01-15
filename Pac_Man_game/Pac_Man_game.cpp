#include <iostream>
#include <fstream>
#include <windows.h>
#include <cmath>

const char pacManCh = 'Y';
const char BlinkyCh = 'B';
const char PinkyCh = 'P';
const char InkyCh = 'I';
const char ClydeCh = 'C';

void loadMap(const char* filePath, char**& matrix, int& rows, int& cols) {
    std::ifstream file(filePath);

    if (!file.is_open()) {
        std::cerr << "Error: Unable to open file" << filePath << std::endl;
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
        }
        if (target == '-' || target == '@') {
            score++;
        }
        /*if (target == '@') {
            frightenedMode = true;
        }*/
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


void moveGhost(char** matrix, int rows, int cols, int& ghostRow, int& ghostCol,
    int targetRow, int targetCol, int& lastDirection,
    bool& exitedCage, const int& startRow, const int& startCol,
    char ghostChar, int pacRow, int pacCol, bool& gameOver, int* exitPos) {

    // Movement priority: up, left, down, right
    int directionPriority[4][2] = {
        {-1, 0}, // Up
        {0, -1}, // Left
        {1, 0},  // Down
        {0, 1}   // Right
    };

    int bestRow = ghostRow, bestCol = ghostCol;
    double minDistance = 1e9; // Almost infinitely large value
    int chosenDirection = -1;


    //int exitIPos = 3; // col - 1, row - 2
    //int exitCPos = 3; // col+1, row - 2

    /*int exitRow = exitPos[0];
    int exitCol = exitPos[1];
    if (exitRow > 0) {
        while (exitRow > 0) {
            exitRow--;
            ghostRow++;
        }
    }
    if (exitRow < 0) {
        while (exitRow < 0) {
            exitRow++;
            ghostRow--;
        }
    }*/





    // Evaluate moves based on priority
    for (int i = 0; i < 4; ++i) {
        int newRow = ghostRow + directionPriority[i][0];
        int newCol = ghostCol + directionPriority[i][1];

        // Check if the move is valid
        if (newRow >= 0 && newRow < rows && newCol >= 0 && newCol < cols &&
            matrix[newRow][newCol] != '#' && matrix[newRow][newCol] != ghostChar &&
            matrix[newRow][newCol] != 'C' && // Avoid the cage area
            (lastDirection == -1 || i != (lastDirection + 2) % 4)) { // Prevent 180-degree reversal
            double distance = sqrt(pow(newRow - targetRow, 2) + pow(newCol - targetCol, 2));
            if (distance < minDistance || (distance == minDistance && i < chosenDirection)) {
                minDistance = distance;
                bestRow = newRow;
                bestCol = newCol;
                chosenDirection = i;
            }
        }
    }

    // Handle reversal exceptions
    if (chosenDirection == -1 && lastDirection != -1) { // Fallback if no valid direction is found
        int reverseDirection = (lastDirection + 2) % 4;
        int reverseRow = ghostRow + directionPriority[reverseDirection][0];
        int reverseCol = ghostCol + directionPriority[reverseDirection][1];

        if (reverseRow >= 0 && reverseRow < rows && reverseCol >= 0 && reverseCol < cols &&
            matrix[reverseRow][reverseCol] != '#' && matrix[reverseRow][reverseCol] != ghostChar) {
            bestRow = reverseRow;
            bestCol = reverseCol;
            chosenDirection = reverseDirection;
        }
    }

    // Update ghost's position and last direction
    if (bestRow != ghostRow || bestCol != ghostCol) {
        matrix[ghostRow][ghostCol] = ' '; // Clear old position
        ghostRow = bestRow;
        ghostCol = bestCol;
        matrix[ghostRow][ghostCol] = ghostChar; // Update new position
        lastDirection = chosenDirection;
    }
    else {
        std::cout << "Ghost did not move. Potential issue in logic.\n";
    }

    // Check if ghost meets Pac-Man
    if (ghostRow == pacRow && ghostCol == pacCol) {
        std::cout << "Collision detected! Game Over triggered.\n";
        gameOver = true;
    }

    // Debug output
    std::cout << "Ghost position: (" << ghostRow << ", " << ghostCol << ")\n";
    std::cout << "Pac-Man position: (" << pacRow << ", " << pacCol << ")\n";
    std::cout << "Game over: " << (gameOver ? "true" : "false") << "\n";
}






void activateB(char** matrix, int rows, int cols, int& BRow, int& BCol,
    int pacRow, int pacCol, int& lastDirection,
    bool frightenedMode, const int& startRow, const int& startCol,
    bool& gameOver) {

    int exitBPos[2] = { -1, 0 }; // row - 1
    moveGhost(matrix, rows, cols, BRow, BCol, pacRow, pacCol,
        lastDirection, frightenedMode, startRow, startCol, 'B', pacRow, pacCol, gameOver, exitBPos);
}

void activateP(char** matrix, int rows, int cols, int& PRow, int& PCol,
    int pacRow, int pacCol, char pacOrientation, int& lastDirection,
    bool frightenedMode, const int& startRow, const int& startCol,
    bool& gameOver) {

    int exitPPos[2] = { -2, 0 }; // row -2

    // Pinky's target position
    int targetRow = pacRow, targetCol = pacCol;
    if (pacOrientation == 'w' || pacOrientation == 'W') { // Up
        targetRow = pacRow - 4;
        targetCol = pacCol - 4; // Four tiles to the left
    }
    else if (pacOrientation == 'a' || pacOrientation == 'A') { // Left
        targetCol = pacCol - 4;
    }
    else if (pacOrientation == 's' || pacOrientation == 'S') { // Down
        targetRow = pacRow + 4;
    }
    else if (pacOrientation == 'd' || pacOrientation == 'D') { // Right
        targetCol = pacCol + 4;
    }


    if (targetRow < 0) {
        targetRow = 0;
    }
    else if (targetRow >= rows) {
        targetRow = rows - 1;
    }

    if (targetCol < 0) {
        targetCol = 0;
    }
    else if (targetCol >= cols) {
        targetCol = cols - 1;
    }

    moveGhost(matrix, rows, cols, PRow, PCol, targetRow, targetCol,
        lastDirection, frightenedMode, startRow, startCol, 'P', pacRow, pacCol, gameOver, exitPPos);
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

    // Starting positions and last directions
    int startRowB = BRow, startColB = BCol, lastDirectionB = -1;
    int startRowP = PRow, startColP = PCol, lastDirectionP = -1;
    int startRowI = IRow, startColI = ICol, lastDirectionI = -1;
    int startRowC = CRow, startColC = CCol, lastDirectionC = -1;

    while (!gameOver) {
        clearScreen();
        printMatrix(matrix, rows, cols);


        if (score >= 0) {
            activateB(matrix, rows, cols, BRow, BCol, pacRow, pacCol, lastDirectionB, frightenedMode, startRowB, startColB, gameOver);
        }
        if (score >= 20) {
            activateP(matrix, rows, cols, PRow, PCol, pacRow, pacCol, pacOrientation, lastDirectionP, frightenedMode, startRowP, startColP, gameOver);
        }
        if (score >= 40) {
            //activateInky(matrix, rows, cols, IRow, ICol, pacRow, pacCol, lastDirectionI, frightenedMode, startRowI, startColI, gameOver);
        }
        if (score >= 60) {
            //activateClyde(matrix, rows, cols, CRow, CCol, pacRow, pacCol, lastDirectionC, frightenedMode, startRowC, startColC, gameOver);
        }


        if (gameOver) {
            std::cout << "Game Over! A ghost caught Pac-Man!" << std::endl;
            break;
        }

        // Display
        std::cout << "Score: " << score << std::endl;
        std::cout << "\nEnter direction (w/a/s/d) or q to quit: ";
        char direction;
        std::cin >> direction;

        if (direction == 'q') {
            break;
        }

        pacOrientation = direction; // Update Pac-Man's orientation
        movePacMan(matrix, rows, cols, pacRow, pacCol, direction, score, frightenedMode);
    }

    cleanupMatrix(matrix, rows);
}


int main() {
    const char* mapPath = "C:\\Users\\PC1\\source\\repos\\Pac_Man_game\\map.txt";
    char** matrix = nullptr;

    int rows = 0, cols = 0;
    int currScore = 0;

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