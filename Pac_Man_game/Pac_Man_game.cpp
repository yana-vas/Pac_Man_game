#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cmath>
#include <ctime>
#include <windows.h>

// -----------------------------------------------------
// GLOBALS
// -----------------------------------------------------
static const char PACMAN_CH = 'Y';
static const char BLINKY_CH = 'B';
static const char PINKY_CH  = 'P';
static const char INKY_CH   = 'I';
static const char CLYDE_CH  = 'C';

char** matrix   = nullptr;
int rows        = 0;
int cols        = 0;

bool gameOver         = false;
bool frightenedMode   = false;
bool switchMode       = false;    // To reverse directions once
int  frightenedCounter= 0;
int  currentScore     = 0;

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
    file.ignore();

    matrix = new char*[rows];
    for (int i = 0; i < rows; ++i) {
        matrix[i] = new char[cols + 1];
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

void printMatrix() {
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            char ch = matrix[i][j];
            switch (ch) {
                case PACMAN_CH: // 'Y'
                    setConsoleColor(14); // Yellow
                    std::cout << ch;
                    break;
                case BLINKY_CH: // 'B'
                    setConsoleColor(12); // Light Red
                    std::cout << ch;
                    break;
                case PINKY_CH:  // 'P'
                    setConsoleColor(13); // Magenta
                    std::cout << ch;
                    break;
                case INKY_CH:   // 'I'
                    setConsoleColor(9);  // Blue
                    std::cout << ch;
                    break;
                case CLYDE_CH:  // 'C'
                    setConsoleColor(10); // Green
                    std::cout << ch;
                    break;
                default:
                    // Everything else: walls, pellets, spaces => default color
                    setConsoleColor(7);
                    std::cout << ch;
                    break;
            }
        }
        // Reset to default
        setConsoleColor(7);
        std::cout << "\n";
    }
    // Ensure color is reset
    setConsoleColor(7);
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
        matrix[r][c] == PINKY_CH  ||
        matrix[r][c] == INKY_CH   ||
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
void sendGhostToCorner(char ghostCh,
    int& gRow, int& gCol,
    char& prevTile,
    int& prevDr, int& prevDc)
{
    // Restore whatever tile was there
    matrix[gRow][gCol] = prevTile;

    // Choose corners for each ghost:
    if (ghostCh == BLINKY_CH) {
        // top-right corner
        gRow = 1;
        gCol = cols - 2;
    }
    else if (ghostCh == PINKY_CH) {
        // top-left corner
        gRow = 1;
        gCol = 1;
    }
    else if (ghostCh == INKY_CH) {
        // bottom-right corner
        gRow = rows - 2;
        gCol = cols - 2;
    }
    else if (ghostCh == CLYDE_CH) {
        // bottom-left corner
        gRow = rows - 2;
        gCol = 1;
    }

    // Put ghost in corner, store old tile
    prevTile = matrix[gRow][gCol];
    matrix[gRow][gCol] = ghostCh;

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

void movePacMan(int& pacRow, int& pacCol,
                char inputDir,
                int& score)
{
    // Pac-Man moves 2 squares if frightened, else 1
    int steps = (frightenedMode ? 2 : 1);

    while (steps-- > 0) {
        int nr = pacRow;
        int nc = pacCol;
        calculateNewPosition(nr, nc, inputDir, 1);

        if (!isValidMovePac(nr, nc)) {
            // can't move into wall or out-of-bounds
            break;
        }

        char tile = matrix[nr][nc];
        if (tile == BLINKY_CH || tile == PINKY_CH ||
            tile == INKY_CH   || tile == CLYDE_CH)
        {
            // If not frightened => game over
            if (!frightenedMode) {
                gameOver = true;
                std::cout << "Game Over! Caught by a ghost!\n";
                return;
            }
            else {
                // If frightened => that ghost goes to corner
                if (tile == BLINKY_CH) {
                    sendGhostToCorner(BLINKY_CH, nr, nc, BprevTile, BprevDr, BprevDc);
                }
                else if (tile == PINKY_CH) {
                    sendGhostToCorner(PINKY_CH, nr, nc, PprevTile, PprevDr, PprevDc);
                }
                else if (tile == INKY_CH) {
                    sendGhostToCorner(INKY_CH, nr, nc, IprevTile, IprevDr, IprevDc);
                }
                else if (tile == CLYDE_CH) {
                    sendGhostToCorner(CLYDE_CH, nr, nc, CprevTile, CprevDr, CprevDc);
                }
                // Continue placing Pac-Man in that cell
            }
        }

        // If power pellet, turn on frightened
        if (tile == '@') {
            frightenedMode = true;
            frightenedCounter = 0;
            switchMode = true;
        }

        matrix[pacRow][pacCol] = ' ';
        matrix[nr][nc] = PACMAN_CH;

        if (tile == '-' || tile == '@') {
            score++;
        }

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
void moveGhostChase(
    int& gRow, int& gCol,
    int& prevDr, int& prevDc,
    char& prevTile,
    char ghostCh,
    int targetRow, int targetCol
)
{
    // Directions in priority order: Up, Left, Down, Right
    struct Dir { int dr, dc; };
    Dir prio[4] = {
        { -1,  0 },  // Up
        {  0, -1 },  // Left
        {  1,  0 },  // Down
        {  0,  1 }   // Right
    };

    // Put back the tile
    matrix[gRow][gCol] = prevTile;

    bool foundValid = false;
    double bestDist = 1e9;
    int chosenDR = 0, chosenDC = 0;

    int oppDr = -prevDr, oppDc = -prevDc;

    // Evaluate directions
    for (int i = 0; i < 4; i++) {
        int dr = prio[i].dr;
        int dc = prio[i].dc;

        // skip immediate reverse
        if (dr == oppDr && dc == oppDc) {
            continue;
        }

        int nr = gRow + dr;
        int nc = gCol + dc;
        if (!isValidMoveGhost(nr, nc)) {
            continue;
        }

        double dist = sqrt((double)(targetRow - nr)*(targetRow - nr)
                         + (double)(targetCol - nc)*(targetCol - nc));
        if (dist < bestDist) {
            bestDist = dist;
            chosenDR = dr;
            chosenDC = dc;
            foundValid = true;
        }
    }

    // If no valid forward direction found, try reverse or stand still
    if (!foundValid) {
        int nr = gRow + oppDr;
        int nc = gCol + oppDc;
        if (isValidMoveGhost(nr, nc)) {
            chosenDR = oppDr;
            chosenDC = oppDc;
            foundValid = true;
        }
        else {
            chosenDR = 0;
            chosenDC = 0;
            foundValid = true;
        }
    }

    gRow += chosenDR;
    gCol += chosenDC;

    prevTile = matrix[gRow][gCol];

    matrix[gRow][gCol] = ghostCh;

    prevDr = chosenDR;
    prevDc = chosenDC;

    // If new cell had Pac-Man => collision
    if (prevTile == PACMAN_CH) {
        if (!frightenedMode) {
            gameOver = true;
            std::cout << "Game Over! A ghost caught Pac-Man!\n";
        } else {
            sendGhostToCorner(ghostCh, gRow, gCol, prevTile, prevDr, prevDc);
        }
    }
}

// -----------------------------------------------------
// GHOST MOVEMENT: FRIGHTENED (random)
// -----------------------------------------------------
void moveGhostFrightened(
    int& gRow, int& gCol,
    int& prevDr, int& prevDc,
    char& prevTile,
    char ghostCh
)
{
    matrix[gRow][gCol] = prevTile;

    int directions[4][2] = {
        {-1, 0}, // Up
        { 0,-1}, // Left
        { 1, 0}, // Down
        { 0, 1}  // Right
    };

    int oppDr = -prevDr, oppDc = -prevDc;
    bool moved = false;
    int tries = 10;

    while (!moved && tries-- > 0) {
        int i = rand() % 4;
        int dr = directions[i][0];
        int dc = directions[i][1];

        // skip immediate reverse
        if (dr == oppDr && dc == oppDc) {
            continue;
        }
        int nr = gRow + dr;
        int nc = gCol + dc;

        if (!isValidMoveGhost(nr, nc)) {
            continue;
        }

        // Move
        gRow = nr;
        gCol = nc;
        prevTile = matrix[gRow][gCol];
        matrix[gRow][gCol] = ghostCh;
        prevDr = dr;
        prevDc = dc;
        moved = true;

        // Collide with Pac-Man => corner
        if (prevTile == PACMAN_CH) {
            sendGhostToCorner(ghostCh, gRow, gCol, prevTile, prevDr, prevDc);
        }
    }
    // If we fail all attempts, try reversing or stand still
    if (!moved) {
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
            // stand still
            matrix[gRow][gCol] = ghostCh;
        }
    }
}

// -----------------------------------------------------
// BLINKY/PINKY/INKY/CLYDE chase logic
// -----------------------------------------------------
void activateP(int& pRow, int& pCol,
    int& pDr, int& pDc,
    char& pPrevTile,
    int pacRow, int pacCol,
    char orientation)
{
    int targetR = pacRow, targetC = pacCol;
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

    // Clamp
    if (targetR < 0) targetR = 0;
    if (targetR >= rows) targetR = rows-1;
    if (targetC < 0) targetC = 0;
    if (targetC >= cols) targetC = cols-1;

    moveGhostChase(pRow, pCol, pDr, pDc, pPrevTile, PINKY_CH, targetR, targetC);
}

void activateI(int& iRow, int& iCol,
    int& iDr, int& iDc,
    char& iPrevTile,
    int pacRow, int pacCol,
    int bRow, int bCol,
    char orientation)
{
    int refR = pacRow, refC = pacCol;
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
    if (targetR < 0) targetR = 0;
    if (targetR >= rows) targetR = rows-1;
    if (targetC < 0) targetC = 0;
    if (targetC >= cols) targetC = cols-1;

    moveGhostChase(iRow, iCol, iDr, iDc, iPrevTile, INKY_CH, targetR, targetC);
}

void activateC(int& cRow, int& cCol,
    int& cDr, int& cDc,
    char& cPrevTile,
    int pacRow, int pacCol)
{
    double dist = sqrt((double)(cCol - pacCol)*(cCol - pacCol)
                     + (double)(cRow - pacRow)*(cRow - pacRow));
    if (dist >= 8.0) {
        moveGhostChase(cRow, cCol, cDr, cDc, cPrevTile, CLYDE_CH, pacRow, pacCol);
    } else {
        // bottom-left corner
        moveGhostChase(cRow, cCol, cDr, cDc, cPrevTile, CLYDE_CH, rows-1, 0);
    }
}


void runGame(int& pacRow, int& pacCol, char& pacOrientation,
    int& Brow, int& Bcol,
    int& Prow, int& Pcol,
    int& Irow, int& Icol,
    int& Crow, int& Ccol,
    int totalPellets)
{
    while (!gameOver) {
        clearScreen();
        printMatrix();
        std::cout << "Score: " << currentScore << " / " << totalPellets << "\n";

        // Win check
        if (currentScore >= totalPellets) {
            std::cout << "You Win! All pellets collected!\n";
            break;
        }

        // Input
        std::cout << "\nMove (w/a/s/d) or q to quit: ";
        char moveKey;
        std::cin >> moveKey;
        if (moveKey == 'q') break;
        if (moveKey!='w' && moveKey!='a' && moveKey!='s' && moveKey!='d') {
            continue; // ignore invalid
        }
        pacOrientation = moveKey;

        // Move Pac-Man
        movePacMan(pacRow, pacCol, moveKey, currentScore);
        if (gameOver) break;

        // If frightened mode => random movement
        if (frightenedMode) {
            if (switchMode) {
                // Reverse directions once
                BprevDr *= -1; BprevDc *= -1;
                PprevDr *= -1; PprevDc *= -1;
                IprevDr *= -1; IprevDc *= -1;
                CprevDr *= -1; CprevDc *= -1;
                switchMode = false;
            }
            moveGhostFrightened(Brow, Bcol, BprevDr, BprevDc, BprevTile, BLINKY_CH);
            if (gameOver) break;
            moveGhostFrightened(Prow, Pcol, PprevDr, PprevDc, PprevTile, PINKY_CH);
            if (gameOver) break;
            moveGhostFrightened(Irow, Icol, IprevDr, IprevDc, IprevTile, INKY_CH);
            if (gameOver) break;
            moveGhostFrightened(Crow, Ccol, CprevDr, CprevDc, CprevTile, CLYDE_CH);
            if (gameOver) break;

            continue; // do not run normal chase logic
        }

        // Normal mode chase
        moveGhostChase(Brow, Bcol, BprevDr, BprevDc, BprevTile, BLINKY_CH, pacRow, pacCol);
        if (gameOver) break;

        if (currentScore >= 20) {
            activateP(Prow, Pcol, PprevDr, PprevDc, PprevTile, pacRow, pacCol, pacOrientation);
            if (gameOver) break;
        }

        if (currentScore >= 40) {
            activateI(Irow, Icol, IprevDr, IprevDc, IprevTile, pacRow, pacCol, Brow, Bcol, pacOrientation);
            if (gameOver) break;
        }

        if (currentScore >= 60) {
            activateC(Crow, Ccol, CprevDr, CprevDc, CprevTile, pacRow, pacCol);
            if (gameOver) break;
        }
    }
}

int main() {
    srand((unsigned)time(NULL));

    const char* mapPath = "C:\\Users\\PC1\\source\\repos\\Pac_Man_game\\map.txt";
    loadMap(mapPath);
    if (!matrix) {
        std::cerr << "Error: Could not load map.\n";
        return 1;
    }

    int totalPellets = maxScorePellets();

    int pacRow = 0, pacCol = 0;
    if (!findCharacter(PACMAN_CH, pacRow, pacCol)) {
        std::cerr << "Error: Pac-Man not found in map!\n";
        cleanupMatrix();
        return 1;
    }

    int Brow = 0, Bcol = 0;
    int Prow = 0, Pcol = 0;
    int Irow = 0, Icol = 0;
    int Crow= 0, Ccol = 0;
    findCharacter(BLINKY_CH, Brow, Bcol);
    findCharacter(PINKY_CH,  Prow, Pcol);
    findCharacter(INKY_CH,   Irow, Icol);
    findCharacter(CLYDE_CH,  Crow, Ccol);

    // Initialize
    BprevTile = ' ';
    PprevTile = ' ';
    IprevTile = ' ';
    CprevTile = ' ';

    char pacOrientation = 'd'; // Default facing right

    runGame(pacRow, pacCol, pacOrientation,
            Brow, Bcol,
            Prow, Pcol,
            Irow, Icol,
            Crow, Ccol,
            totalPellets);

    cleanupMatrix();
    return 0;
}
