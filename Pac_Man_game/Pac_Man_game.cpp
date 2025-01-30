/**
 *
 * Solution to course project # 5
 * Introduction to programming course
 * Faculty of Mathematics and Informatics of Sofia University
 * Winter semester 2024/2025
 *
 * @author Yana Vasileva
 * @idnumber 4MI0600494
 * @compiler VC
 *
 * This file implements a text-based Pac-Man-like game:
 * - Loads a map from file
 * - Handles Pac-Man and ghost movement, collisions, and scoring
 * - Manages frightened mode, win/lose conditions, and display updates
 *
 */

#include <cmath>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <iostream>
#include <windows.h>

 // -----------------------------------------------------
 // GLOBALS
 // -----------------------------------------------------
static const char PACMAN_CH = 'Y';
static const char BLINKY_CH = 'B';
static const char PINKY_CH = 'P';
static const char INKY_CH = 'I';
static const char CLYDE_CH = 'C';

char** matrix = nullptr;
int rows = 0;
int cols = 0;

bool gameOver = false;
bool frightenedMode = false;
bool switchMode = false;    // To reverse directions once
int  frightenedCounter = 0;
int  currentScore = 0;

// Track "previous tile" for each ghost so we can restore pellets/walls
char BprevTile = ' ';
char PprevTile = ' ';
char IprevTile = ' ';
char CprevTile = ' ';

// Track last movement direction (dr,dc) for each ghost to prevent reversing
int BprevDr = 0, BprevDc = 0;
int PprevDr = 0, PprevDc = 0;
int IprevDr = 0, IprevDc = 0;
int CprevDr = 0, CprevDc = 0;

// -----------------------------------------------------
// GLOBALS for positions/orientation/scores
// -----------------------------------------------------
int pacRow = 0, pacCol = 0;
int Brow = 0, Bcol = 0;
int Prow = 0, Pcol = 0;
int Irow = 0, Icol = 0;
int Crow = 0, Ccol = 0;
char pacOrientation = 'd';
int totalPellets = 0;


bool Bfound = false;
bool Pfound = false;
bool Ifound = false;
bool Cfound = false;

// -----------------------------------------------------
// UTILITY
// -----------------------------------------------------
void setConsoleColor(WORD color)
{
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
}

void clearScreen() {
    system("cls");
}

void loadMap(const char* filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        std::cerr << "Error: Unable to open file " << filePath << std::endl;
        return;
    }

    file >> rows >> cols;
    file.ignore(); // read past newline

    if (rows <= 0 || cols <= 0) {
        std::cerr << "Error: Invalid map dimensions read from file.\n";
        file.close();
        return;
    }

    matrix = new char* [rows];
    for (int i = 0; i < rows; ++i) {
        matrix[i] = new char[cols + 1];
        // Read exactly one line up to cols characters:
        file.getline(matrix[i], cols + 1);
    }
    file.close();
}

void cleanupMatrix() {
    if (!matrix) return;
    for (int i = 0; i < rows; i++) {
        delete[] matrix[i];
    }
    delete[] matrix;
    matrix = nullptr;
}

void setCharacterColor(char ch) {
    switch (ch) {
    case 'Y': // PACMAN_CH
        setConsoleColor(14); // Yellow
        break;
    case 'B': // BLINKY_CH
        setConsoleColor(12); // Light Red
        break;
    case 'P': // PINKY_CH
        setConsoleColor(13); // Magenta
        break;
    case 'I': // INKY_CH
        setConsoleColor(9);  // Blue
        break;
    case 'C': // CLYDE_CH
        setConsoleColor(10); // Green
        break;
    default:
        setConsoleColor(7);  // Default color
        break;
    }
}

void printMatrix() {
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            char ch = matrix[i][j];
            setCharacterColor(ch);
            std::cout << ch;
        }
        setConsoleColor(7);  // Reset to default after each row
        std::cout << "\n";
    }
    setConsoleColor(7);  // Ensure color is reset at the end
}

int maxScorePellets() {
    int total = 0;
    for (int r = 0; r < rows; r++) {
        for (int c = 0; c < cols; c++) {
            if (matrix[r][c] == '-' || matrix[r][c] == '@') {
                total++;
            }
        }
    }
    return total;
}

bool findCharacter(char ch, int& outRow, int& outCol) {
    for (int r = 0; r < rows; r++) {
        for (int c = 0; c < cols; c++) {
            if (matrix[r][c] == ch) {
                outRow = r;
                outCol = c;
                return true;
            }
        }
    }
    return false;
}

bool isValidPosition(int r, int c) {
    return (r >= 0 && r < rows && c >= 0 && c < cols);
}

// Pac-Man can move onto space ' ', pellet '-', or power '@'
bool isValidMovePac(int r, int c) {
    if (!isValidPosition(r, c)) return false;
    return (matrix[r][c] == ' ' || matrix[r][c] == '-' || matrix[r][c] == '@');
}

// Ghost can move onto anything except walls '#' and other ghosts
bool isValidMoveGhost(int r, int c)
{
    if (!isValidPosition(r, c)) return false;

    if (matrix[r][c] == '#') return false;

    if (matrix[r][c] == BLINKY_CH ||
        matrix[r][c] == PINKY_CH ||
        matrix[r][c] == INKY_CH ||
        matrix[r][c] == CLYDE_CH)
    {
        return false;
    }
    return true;
}

// -----------------------------------------------------
// FRIGHTENED MODE
// -----------------------------------------------------
void updateFrightenedMode() {
    if (!frightenedMode) return;
    frightenedCounter++;
    if (frightenedCounter >= 10) { // lasts 10 moves
        frightenedMode = false;
        frightenedCounter = 0;
    }
}

// -----------------------------------------------------
// CORNER TELEPORT for Ghosts in FRIGHTENED collision
// -----------------------------------------------------
void getGhostCornerPosition(char ghostCh, int& gRow, int& gCol) {
    switch (ghostCh) {
    case BLINKY_CH: // Top-right corner
        gRow = 1;
        gCol = cols - 2;
        break;
    case PINKY_CH: // Top-left corner
        gRow = 1;
        gCol = 1;
        break;
    case INKY_CH: // Bottom-right corner
        gRow = rows - 2;
        gCol = cols - 2;
        break;
    case CLYDE_CH: // Bottom-left corner
        gRow = rows - 2;
        gCol = 1;
        break;
    default:
        break;
    }
}

void sendGhostToCorner(char ghostCh, int& gRow, int& gCol, char& prevTile, int& prevDr, int& prevDc) {
    // Restore whatever tile was there
    matrix[gRow][gCol] = prevTile;

    getGhostCornerPosition(ghostCh, gRow, gCol);

    // If for some reason the corner is out-of-bounds or a wall, we ignore it
    if (isValidPosition(gRow, gCol)) {
        prevTile = matrix[gRow][gCol];
        matrix[gRow][gCol] = ghostCh;
    }

    // Reset direction deltas
    prevDr = 0;
    prevDc = 0;
}

// -----------------------------------------------------
// PAC-MAN
// -----------------------------------------------------
void calculateNewPosition(int& r, int& c, char dir, int step) {
    switch (dir) {
    case 'w': case 'W': r -= step; break;
    case 'a': case 'A': c -= step; break;
    case 's': case 'S': r += step; break;
    case 'd': case 'D': c += step; break;
    }
}

bool handleGhostCollision(char tile, int nr, int nc) {
    if (!frightenedMode) {
        gameOver = true;
        std::cout << "Game Over! Caught by a ghost!\n";
        return true;
    }
    else {
        // If frightened => that ghost goes to its corner
        switch (tile) {
        case BLINKY_CH:
            sendGhostToCorner(BLINKY_CH, nr, nc, BprevTile, BprevDr, BprevDc);
            break;
        case PINKY_CH:
            sendGhostToCorner(PINKY_CH, nr, nc, PprevTile, PprevDr, PprevDc);
            break;
        case INKY_CH:
            sendGhostToCorner(INKY_CH, nr, nc, IprevTile, IprevDr, IprevDc);
            break;
        case CLYDE_CH:
            sendGhostToCorner(CLYDE_CH, nr, nc, CprevTile, CprevDr, CprevDc);
            break;
        }
    }
    return false;
}

void handlePowerPellet(char tile) {
    if (tile == '@') {
        frightenedMode = true;
        frightenedCounter = 0;
        switchMode = true;
    }
}

void movePacMan(char inputDir) {
    int steps = (frightenedMode ? 2 : 1);

    while (steps-- > 0) {
        int nr = pacRow, nc = pacCol;
        calculateNewPosition(nr, nc, inputDir, 1);

        if (!isValidMovePac(nr, nc)) break; // Invalid move

        char tile = matrix[nr][nc];

        // Handle ghost collision
        if (tile == BLINKY_CH || tile == PINKY_CH ||
            tile == INKY_CH || tile == CLYDE_CH)
        {
            if (handleGhostCollision(tile, nr, nc)) return;
        }

        handlePowerPellet(tile);

        // Move Pac-Man
        matrix[pacRow][pacCol] = ' ';
        matrix[nr][nc] = PACMAN_CH;

        if (tile == '-' || tile == '@') currentScore++;

        pacRow = nr;
        pacCol = nc;

        updateFrightenedMode();
        if (gameOver) return;
    }
}

// -----------------------------------------------------
// GHOST MOVEMENT: CHASE
//   - No immediate reverse
//   - No passing walls
//   - Priority: Up -> Left -> Down -> Right
// -----------------------------------------------------
struct Dir { int dr, dc; };

void findBestDirection(
    int gRow, int gCol,
    int targetRow, int targetCol,
    int prevDr, int prevDc,
    int& chosenDR, int& chosenDC,
    bool& foundValid)
{
    Dir prio[4] = {
        { -1,  0 },  // Up
        {  0, -1 },  // Left
        {  1,  0 },  // Down
        {  0,  1 }   // Right
    };

    double bestDist = 1e9;
    int oppDr = -prevDr, oppDc = -prevDc;

    for (int i = 0; i < 4; i++) {
        int dr = prio[i].dr;
        int dc = prio[i].dc;

        // Skip immediate reverse direction
        if (dr == oppDr && dc == oppDc) continue;

        int nr = gRow + dr;
        int nc = gCol + dc;
        if (!isValidMoveGhost(nr, nc)) continue;

        double dist = std::sqrt(
            double(targetRow - nr) * (targetRow - nr) +
            double(targetCol - nc) * (targetCol - nc)
        );
        if (dist < bestDist) {
            bestDist = dist;
            chosenDR = dr;
            chosenDC = dc;
            foundValid = true;
        }
    }
}

void handleNoValidDirection(
    int gRow, int gCol,
    int prevDr, int prevDc,
    int& chosenDR, int& chosenDC,
    bool& foundValid)
{
    // Try going the opposite direction if everything else fails
    int oppDr = -prevDr, oppDc = -prevDc;
    int nr = gRow + oppDr;
    int nc = gCol + oppDc;

    if (isValidMoveGhost(nr, nc)) {
        chosenDR = oppDr;
        chosenDC = oppDc;
        foundValid = true;
    }
    else {
        // Stand still if no valid move
        chosenDR = 0;
        chosenDC = 0;
        foundValid = true;
    }
}

void checkPacmanCollision(
    char ghostCh,
    int gRow, int gCol,
    char& prevTile,
    int& prevDr, int& prevDc)
{
    if (prevTile == PACMAN_CH) {
        if (!frightenedMode) {
            gameOver = true;
            std::cout << "Game Over! A ghost caught Pac-Man!\n";
        }
        else {
            sendGhostToCorner(ghostCh, gRow, gCol, prevTile, prevDr, prevDc);
        }
    }
}

void moveGhostChase(
    int& gRow, int& gCol,
    int& prevDr, int& prevDc,
    char& prevTile,
    char ghostCh,
    int targetRow, int targetCol)
{
    // Restore previous tile
    matrix[gRow][gCol] = prevTile;

    int chosenDR = 0, chosenDC = 0;
    bool foundValid = false;

    findBestDirection(gRow, gCol, targetRow, targetCol, prevDr, prevDc, chosenDR, chosenDC, foundValid);

    if (!foundValid) {
        handleNoValidDirection(gRow, gCol, prevDr, prevDc, chosenDR, chosenDC, foundValid);
    }

    gRow += chosenDR;
    gCol += chosenDC;

    prevTile = matrix[gRow][gCol];
    matrix[gRow][gCol] = ghostCh;
    prevDr = chosenDR;
    prevDc = chosenDC;

    checkPacmanCollision(ghostCh, gRow, gCol, prevTile, prevDr, prevDc);
}

// -----------------------------------------------------
// GHOST MOVEMENT: FRIGHTENED (random)
// -----------------------------------------------------
bool chooseRandomDirection(
    int& gRow, int& gCol,
    int& prevDr, int& prevDc,
    char ghostCh,
    char& prevTile)
{
    int directions[4][2] = {
        {-1, 0}, // Up
        { 0,-1}, // Left
        { 1, 0}, // Down
        { 0, 1}  // Right
    };

    int oppDr = -prevDr, oppDc = -prevDc;
    int tries = 10;

    while (tries-- > 0) {
        int i = std::rand() % 4;
        int dr = directions[i][0];
        int dc = directions[i][1];

        // Skip immediate reverse direction
        if (dr == oppDr && dc == oppDc) continue;

        int nr = gRow + dr;
        int nc = gCol + dc;

        if (!isValidMoveGhost(nr, nc)) continue;

        // Move ghost
        gRow = nr;
        gCol = nc;
        prevTile = matrix[gRow][gCol];
        matrix[gRow][gCol] = ghostCh;
        prevDr = dr;
        prevDc = dc;

        // Check collision with Pac-Man
        if (prevTile == PACMAN_CH) {
            sendGhostToCorner(ghostCh, gRow, gCol, prevTile, prevDr, prevDc);
        }

        return true; // Successfully moved
    }

    return false; // No valid move found
}

void handleNoValidMove(
    int& gRow, int& gCol,
    int& prevDr, int& prevDc,
    char ghostCh,
    char& prevTile)
{
    int oppDr = -prevDr, oppDc = -prevDc;
    int nr = gRow + oppDr;
    int nc = gCol + oppDc;

    if (isValidMoveGhost(nr, nc)) {
        gRow = nr;
        gCol = nc;
        prevTile = matrix[gRow][gCol];
        matrix[gRow][gCol] = ghostCh;
        prevDr = oppDr;
        prevDc = oppDc;

        if (prevTile == PACMAN_CH) {
            sendGhostToCorner(ghostCh, gRow, gCol, prevTile, prevDr, prevDc);
        }
    }
    else {
        // Stand still
        matrix[gRow][gCol] = ghostCh;
    }
}

void moveGhostFrightened(
    int& gRow, int& gCol,
    int& prevDr, int& prevDc,
    char& prevTile,
    char ghostCh)
{
    matrix[gRow][gCol] = prevTile;

    if (!chooseRandomDirection(gRow, gCol, prevDr, prevDc, ghostCh, prevTile)) {
        handleNoValidMove(gRow, gCol, prevDr, prevDc, ghostCh, prevTile);
    }
}

// -----------------------------------------------------
// BLINKY/PINKY/INKY/CLYDE chase logic
// -----------------------------------------------------
void activateP(char orientation)
{
    int targetR = pacRow;
    int targetC = pacCol;
    // Pinky tries to get 4 steps "ahead" of Pac-Man
    if (orientation == 'w' || orientation == 'W') {
        targetR -= 4;
        targetC -= 4;
    }
    else if (orientation == 's' || orientation == 'S') {
        targetR += 4;
    }
    else if (orientation == 'a' || orientation == 'A') {
        targetC -= 4;
    }
    else if (orientation == 'd' || orientation == 'D') {
        targetC += 4;
    }

    // Clamp to boundaries
    if (targetR < 0)       targetR = 0;
    if (targetR >= rows)   targetR = rows - 1;
    if (targetC < 0)       targetC = 0;
    if (targetC >= cols)   targetC = cols - 1;

    moveGhostChase(Prow, Pcol, PprevDr, PprevDc, PprevTile, PINKY_CH, targetR, targetC);
}

void activateI(int bRow, int bCol, char orientation)
{
    int refR = pacRow;
    int refC = pacCol;
    // 2 tiles in front of Pac-Man
    if (orientation == 'w' || orientation == 'W') {
        refR -= 2;
        refC -= 2;
    }
    else if (orientation == 's' || orientation == 'S') {
        refR += 2;
    }
    else if (orientation == 'a' || orientation == 'A') {
        refC -= 2;
    }
    else if (orientation == 'd' || orientation == 'D') {
        refC += 2;
    }

    int vR = (refR - bRow) * 2;
    int vC = (refC - bCol) * 2;
    int targetR = bRow + vR;
    int targetC = bCol + vC;

    // Clamp to boundaries
    if (targetR < 0)       targetR = 0;
    if (targetR >= rows)   targetR = rows - 1;
    if (targetC < 0)       targetC = 0;
    if (targetC >= cols)   targetC = cols - 1;

    moveGhostChase(Irow, Icol, IprevDr, IprevDc, IprevTile, INKY_CH, targetR, targetC);
}

void activateC()
{
    double dist = std::sqrt(double(Ccol - pacCol) * (Ccol - pacCol)
        + double(Crow - pacRow) * (Crow - pacRow));
    if (dist >= 8.0) {
        moveGhostChase(Crow, Ccol, CprevDr, CprevDc, CprevTile, CLYDE_CH, pacRow, pacCol);
    }
    else {
        // Move toward bottom-left corner
        moveGhostChase(Crow, Ccol, CprevDr, CprevDc, CprevTile, CLYDE_CH, rows - 1, 0);
    }
}

// -----------------------------------------------------
// Display & Win Checks
// -----------------------------------------------------
void updateDisplay() {
    clearScreen();
    printMatrix();
    std::cout << "Score: " << currentScore << " / " << totalPellets << "\n";
}

bool checkWin() {
    if (currentScore >= totalPellets && totalPellets > 0) {
        std::cout << "You Win! All pellets collected!\n";
        return true;
    }
    return false;
}

char getPlayerMove() {
    std::cout << "\nMove (w/a/s/d) or q to quit: ";
    char moveKey;
    std::cin >> moveKey;
    return moveKey;
}

bool isValidMove(char moveKey) {
    return moveKey == 'w' || moveKey == 'a' ||
        moveKey == 's' || moveKey == 'd' ||
        moveKey == 'W' || moveKey == 'A' ||
        moveKey == 'S' || moveKey == 'D';
}

// -----------------------------------------------------
// GHOST DIRECTION REVERSAL (when frightened starts)
// -----------------------------------------------------
void reverseGhostDirections() {
    BprevDr *= -1; BprevDc *= -1;
    PprevDr *= -1; PprevDc *= -1;
    IprevDr *= -1; IprevDc *= -1;
    CprevDr *= -1; CprevDc *= -1;
}

// -----------------------------------------------------
// FRIGHTENED and NORMAL MODE HANDLERS
// -----------------------------------------------------
void handleFrightenedMode() {
    if (switchMode) {
        reverseGhostDirections();
        switchMode = false;
    }

    // Move each ghost only if found
    if (Bfound) moveGhostFrightened(Brow, Bcol, BprevDr, BprevDc, BprevTile, BLINKY_CH);
    if (gameOver) return;

    if (Pfound) moveGhostFrightened(Prow, Pcol, PprevDr, PprevDc, PprevTile, PINKY_CH);
    if (gameOver) return;

    if (Ifound) moveGhostFrightened(Irow, Icol, IprevDr, IprevDc, IprevTile, INKY_CH);
    if (gameOver) return;

    if (Cfound) moveGhostFrightened(Crow, Ccol, CprevDr, CprevDc, CprevTile, CLYDE_CH);
    if (gameOver) return;
}

void handleNormalMode() {
    // Blinky always moves if found
    if (Bfound) {
        moveGhostChase(Brow, Bcol, BprevDr, BprevDc, BprevTile, BLINKY_CH, pacRow, pacCol);
        if (gameOver) return;
    }

    // Pinky after 20 points
    if (Pfound && currentScore >= 20) {
        activateP(pacOrientation);
        if (gameOver) return;
    }

    // Inky after 40 points
    if (Ifound && currentScore >= 40) {
        activateI(Brow, Bcol, pacOrientation);
        if (gameOver) return;
    }

    // Clyde after 60 points
    if (Cfound && currentScore >= 60) {
        activateC();
        if (gameOver) return;
    }
}

void runGame() {
    while (!gameOver) {
        updateDisplay();

        if (checkWin()) {
            break;
        }

        char moveKey = getPlayerMove();
        if (moveKey == 'q' || moveKey == 'Q') {
            break;
        }

        if (!isValidMove(moveKey)) {
            // Ignore invalid input, just loop
            continue;
        }

        pacOrientation = moveKey;

        // Pac-Man moves first
        movePacMan(moveKey);
        if (gameOver) break;

        // Then ghosts move
        if (frightenedMode) {
            handleFrightenedMode();
        }
        else {
            handleNormalMode();
        }

        if (gameOver) break;
    }
}

// -----------------------------------------------------
// MAIN
// -----------------------------------------------------
int main() {
    std::srand((unsigned)time(NULL));

    const char* mapPath = "../map.txt";
    loadMap(mapPath);
    if (!matrix) {
        std::cerr << "Error: Could not load map or invalid map data.\n";
        return 1;
    }

    totalPellets = maxScorePellets();

    if (!findCharacter(PACMAN_CH, pacRow, pacCol)) {
        std::cerr << "Error: Pac-Man not found in map!\n";
        cleanupMatrix();
        return 1;
    }

    Bfound = findCharacter(BLINKY_CH, Brow, Bcol);
    Pfound = findCharacter(PINKY_CH, Prow, Pcol);
    Ifound = findCharacter(INKY_CH, Irow, Icol);
    Cfound = findCharacter(CLYDE_CH, Crow, Ccol);

    runGame();
    cleanupMatrix();
    return 0;
}
