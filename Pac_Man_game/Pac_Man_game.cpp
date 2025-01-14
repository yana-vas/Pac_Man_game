#include <iostream>
#include <fstream>
#include <windows.h>

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

void findPacMan(char** matrix, int rows, int cols, int& pacRow, int& pacCol) {
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            if (matrix[i][j] == 'Y') {
                pacRow = i;
                pacCol = j;
                return;
            }
        }
    }
    pacRow = -1;
    pacCol = -1;
}

void movePacMan(char** matrix, int rows, int cols, int& pacRow, int& pacCol, char direction) {
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
            matrix[newRow][newCol] = 'Y';

            pacRow = newRow;
            pacCol = newCol;
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

void runGame(char** matrix, int rows, int cols, int& pacRow, int& pacCol) {
    if (pacRow == -1 || pacCol == -1) {
        std::cerr << "Error: Pac-Man not found in the map." << std::endl;
        cleanupMatrix(matrix, rows);
        exit(1);
    }

    while (true) {
        clearScreen();
        printMatrix(matrix, rows, cols);

        std::cout << "\nEnter direction (w/a/s/d) or q to quit: ";
        char direction;
        std::cin >> direction;

        if (direction == 'q') {
            break;
        }

        movePacMan(matrix, rows, cols, pacRow, pacCol, direction);
    }
}

int main() {
    const char* mapPath = "C:\\Users\\PC1\\source\\repos\\Pac_Man_game\\map.txt";
    char** matrix = nullptr;
    int rows = 0, cols = 0;

    loadMap(mapPath, matrix, rows, cols);

    if (matrix) {
        int pacRow = 0, pacCol = 0;
        findPacMan(matrix, rows, cols, pacRow, pacCol);
        runGame(matrix, rows, cols, pacRow, pacCol);
        cleanupMatrix(matrix, rows);
    }

    return 0;
}
