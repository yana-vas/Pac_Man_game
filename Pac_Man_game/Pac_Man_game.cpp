#include <iostream>
#include <fstream>

void loadMap(const char* filePath, char**& matrix, int& rows, int& cols) {
    std::ifstream file(filePath);

    if (!file.is_open()) {
        std::cerr << "Error: Unable to open file" << filePath << std::endl;
        rows = cols = 0;
        matrix = nullptr;
        return;
    }

    // Read dimensions from the first line
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

int main() {
    const char* filePath = "C:\\Users\\PC1\\source\\repos\\Pac_Man_game\\map.txt";
    //C:\Users\PC1\source\repos\Pac_Man_game
    char** matrix = nullptr;
    int rows = 0, cols = 0;

    loadMap(filePath, matrix, rows, cols);

    if (matrix) {
        // Output the matrix
        for (int i = 0; i < rows; ++i) {
            std::cout << matrix[i] << std::endl;
        }

        // Deallocate the matrix
        cleanupMatrix(matrix, rows);
    }

    return 0;
}
