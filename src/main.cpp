#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <Wire.h>

#define TFT_CS   10
#define TFT_DC   9
#define TFT_RST  8
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST);

#define BUTTON_UP 2
#define BUTTON_DOWN 3
#define BUTTON_LEFT 4
#define BUTTON_RIGHT 5
#define BUTTON_SELECT 6

#define BOARD_SIZE 8
#define SQUARE_SIZE 30  
#define HIGHLIGHT_COLOR ILI9341_YELLOW  
#define SCREEN_WIDTH 240
#define SCREEN_HEIGHT 320

// Variables for timer and score
#define TOTAL_TIME 60 // Total game time in seconds (60 seconds)
unsigned long startTime;
int elapsedSeconds = 0;
int remainingSeconds = TOTAL_TIME; // Global variable to track remaining seconds
int scorePlayer1 = 0;
int scorePlayer2 = 0;


int lastX = -1, lastY = -1;  
int cursorX = 0;
int cursorY = 0;
int selectedX = -1;
int selectedY = -1;
bool pieceSelected = false;
bool isPlayer1Turn = true; // Player 1 starts the game


enum Piece { EMPTY = 0, PLAYER1 = 1, PLAYER2 = 2, KING1 = -1, KING2 = -2 };


// Initialize the board
Piece board[BOARD_SIZE][BOARD_SIZE] = {
  {PLAYER1, EMPTY, PLAYER1, EMPTY, PLAYER1, EMPTY, PLAYER1, EMPTY},
  {EMPTY, PLAYER1, EMPTY, EMPTY, EMPTY, PLAYER1, EMPTY, PLAYER1},
  {PLAYER1, EMPTY, PLAYER1, EMPTY, PLAYER1, EMPTY, PLAYER1, EMPTY},
  {EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY},
  {EMPTY, EMPTY, PLAYER1, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY},
  {EMPTY, PLAYER2, EMPTY, PLAYER2, EMPTY, PLAYER2, EMPTY, PLAYER2},
  {PLAYER2, EMPTY, PLAYER1, EMPTY, PLAYER2, EMPTY, PLAYER2, EMPTY},
  {EMPTY, PLAYER2, EMPTY, EMPTY, EMPTY, PLAYER2, EMPTY, PLAYER2}
};



// Function prototypes
void drawChessboard();
void drawPieces(Piece board[BOARD_SIZE][BOARD_SIZE]);
void drawCoin(int row, int col, uint16_t color);
void movePiece(int fromRow, int fromCol, int toRow, int toCol);
void highlightSquare(int row, int col, uint16_t color);
void drawSquare(int row, int col);
bool hasMoreJump(int row, int col);
bool isValidMove(Piece board[BOARD_SIZE][BOARD_SIZE], int fromRow, int fromCol, int toRow, int toCol);
void handleButtons();
void updateTimer();
void updateScore();
void displayStatus();


// Function pointers for modular actions
void (*drawSquarePtr)(int, int) = drawSquare;
void (*highlightSquarePtr)(int, int, uint16_t) = highlightSquare;
void (*movePiecePtr)(int, int, int, int) = movePiece;
bool (*isValidMovePtr)(Piece [BOARD_SIZE][BOARD_SIZE], int, int, int, int) = isValidMove;

void setup() {
  tft.begin();
  tft.fillScreen(ILI9341_BLACK);
  drawChessboard();
  drawPieces(board);

  // Set button pins
  pinMode(BUTTON_UP, INPUT_PULLUP);
  pinMode(BUTTON_DOWN, INPUT_PULLUP);
  pinMode(BUTTON_LEFT, INPUT_PULLUP);
  pinMode(BUTTON_RIGHT, INPUT_PULLUP);
  pinMode(BUTTON_SELECT, INPUT_PULLUP);

  // Initialize timer
  startTime = millis();

  // Display initial timer and score
  displayStatus();
}

void drawChessboard() {
  for (int row = 0; row < BOARD_SIZE; row++) {
    for (int col = 0; col < BOARD_SIZE; col++) {
      uint16_t color = (row + col) % 2 == 0 ? ILI9341_BLACK : ILI9341_WHITE;
      tft.fillRect(col * SQUARE_SIZE, row * SQUARE_SIZE, SQUARE_SIZE, SQUARE_SIZE, color);
    }
  }
}

void drawPieces(Piece board[BOARD_SIZE][BOARD_SIZE]) {
  for (int row = 0; row < BOARD_SIZE; row++) {
    for (int col = 0; col < BOARD_SIZE; col++) {
      int value = board[row][col];

      if (value == 1) {
        drawCoin(row, col, ILI9341_BLUE);  // Blue coin for player 1
      } else if (value == 2) {
        drawCoin(row, col, ILI9341_RED);   // Red coin for player 2
      } else if (value == -1) {
        drawCoin(row, col, ILI9341_CYAN);  // Cyan coin for player 1 king
      } else if (value == -2) {
        drawCoin(row, col, ILI9341_ORANGE); // Orange coin for player 2 king
      }
    }
  }
}

void drawCoin(int row, int col, uint16_t color) {
  int x = col * SQUARE_SIZE;
  int y = row * SQUARE_SIZE;
  int radius = SQUARE_SIZE / 2 - 4;
  tft.fillCircle(x + SQUARE_SIZE / 2, y + SQUARE_SIZE / 2, radius, color);
}

void highlightSquare(int row, int col, uint16_t color) {
  int x = col * SQUARE_SIZE;
  int y = row * SQUARE_SIZE;
  int thickness = 3;  

  for (int i = 0; i < thickness; i++) {
    tft.drawFastHLine(x, y + i, SQUARE_SIZE, color);                 
    tft.drawFastHLine(x, y + SQUARE_SIZE - i - 1, SQUARE_SIZE, color); 
  }

  for (int i = 0; i < thickness; i++) {
    tft.drawFastVLine(x + i, y, SQUARE_SIZE, color);                  
    tft.drawFastVLine(x + SQUARE_SIZE - i - 1, y, SQUARE_SIZE, color); 
  }
}


void drawSquare(int row, int col) {
  // Draw the square itself 
  uint16_t color = (row + col) % 2 == 0 ? ILI9341_BLACK : ILI9341_WHITE;
  int x = col * SQUARE_SIZE;
  int y = row * SQUARE_SIZE;
  tft.fillRect(x, y, SQUARE_SIZE, SQUARE_SIZE, color);

  // If there is a piece on this square, redraw it
  if (board[row][col] != 0) {
    int piece = board[row][col];
    if (piece == 1) {
      drawCoin(row, col, ILI9341_BLUE);  // Player 1
    } else if (piece == 2) {
      drawCoin(row, col, ILI9341_RED);   // Player 2
    } else if (piece == -1) {
      drawCoin(row, col, ILI9341_CYAN);  // Player 1 King
    } else if (piece == -2) {
      drawCoin(row, col, ILI9341_ORANGE); // Player 2 King
    }
  }
}

bool hasMoreJump(int row, int col) {
    int piece = board[row][col];

    // Check for Player 1 regular piece
    if (piece == PLAYER1) {
        if (row + 2 < BOARD_SIZE && col + 2 < BOARD_SIZE && board[row + 1][col + 1] == PLAYER2 && board[row + 2][col + 2] == EMPTY) return true;
        if (row + 2 < BOARD_SIZE && col - 2 >= 0 && board[row + 1][col - 1] == PLAYER2 && board[row + 2][col - 2] == EMPTY) return true;
    }

    // Check for Player 2 regular piece
    else if (piece == PLAYER2) {
        if (row - 2 >= 0 && col + 2 < BOARD_SIZE && board[row - 1][col + 1] == PLAYER1 && board[row - 2][col + 2] == EMPTY) return true;
        if (row - 2 >= 0 && col - 2 >= 0 && board[row - 1][col - 1] == PLAYER1 && board[row - 2][col - 2] == EMPTY) return true;
    }

    // Check for kings
    if (piece == KING1 || piece == KING2) {
        if (row + 2 < BOARD_SIZE && col + 2 < BOARD_SIZE && board[row + 1][col + 1] != EMPTY && board[row + 2][col + 2] == EMPTY) return true;
        if (row + 2 < BOARD_SIZE && col - 2 >= 0 && board[row + 1][col - 1] != EMPTY && board[row + 2][col - 2] == EMPTY) return true;
        if (row - 2 >= 0 && col + 2 < BOARD_SIZE && board[row - 1][col + 1] != EMPTY && board[row - 2][col + 2] == EMPTY) return true;
        if (row - 2 >= 0 && col - 2 >= 0 && board[row - 1][col - 1] != EMPTY && board[row - 2][col - 2] == EMPTY) return true;
    }

    return false;
}


void movePiece(int fromRow, int fromCol, int toRow, int toCol) {
    // Check if the destination square is empty
    if (board[toRow][toCol] == EMPTY) {
        // Check for a jump (moving over an opponent's piece)
        if (abs(toRow - fromRow) == 2 && abs(toCol - fromCol) == 2) {
            int jumpedRow = (fromRow + toRow) / 2;
            int jumpedCol = (fromCol + toCol) / 2;

            board[jumpedRow][jumpedCol] = EMPTY; 
            (*drawSquarePtr)(jumpedRow, jumpedCol); 

            if (board[fromRow][fromCol] == PLAYER1 || board[fromRow][fromCol] == KING1) {
                scorePlayer1 += 1;
            } else if (board[fromRow][fromCol] == PLAYER2 || board[fromRow][fromCol] == KING2) {
                scorePlayer2 += 1;
            }
            updateScore();  
        }

        // Move the piece to the new square
        board[toRow][toCol] = board[fromRow][fromCol];  
        board[fromRow][fromCol] = EMPTY;  

        // Check for promotion to king
        if ((board[toRow][toCol] == PLAYER1 && toRow == 7) || (board[toRow][toCol] == KING1 && toRow == 7)) { 
            board[toRow][toCol] = KING1; // Promote to king
            scorePlayer1 += 2; // Add bonus points
            updateScore(); 
        } else if ((board[toRow][toCol] == PLAYER2 && toRow == 0) || (board[toRow][toCol] == KING2 && toRow == 0)) { 
            board[toRow][toCol] = KING2; // Promote to king
            scorePlayer2 += 2; // Add bonus points
            updateScore(); 
        }

        // Redraw the moved squares
        (*drawSquarePtr)(fromRow, fromCol);
        (*drawSquarePtr)(toRow, toCol);
        drawPieces(board); 

        // Check for additional jumps after a capture
        if (abs(toRow - fromRow) == 2 && hasMoreJump(toRow, toCol)) {
            // Highlight the current piece to indicate the player can continue jumping
            selectedX = toCol;
            selectedY = toRow;
            highlightSquare(toRow, toCol, ILI9341_YELLOW);
            pieceSelected = true;  // Keep piece selected for another jump
        } else {
            pieceSelected = false; // End player's turn if no more jumps are possible
            isPlayer1Turn = !isPlayer1Turn; // Toggle turn
        }
    }
}





bool isValidMove(Piece board[BOARD_SIZE][BOARD_SIZE], int fromRow, int fromCol, int toRow, int toCol) {
  if (board[toRow][toCol] != EMPTY) {
    return false;  // Destination square must be empty
  }

  Piece piece = board[fromRow][fromCol];

  // Player 1 regular piece
  if (piece == PLAYER1) {
    if (toRow == fromRow + 1 && abs(toCol - fromCol) == 1) {
      return true;  
    } else if (toRow == fromRow + 2 && abs(toCol - fromCol) == 2 && board[(fromRow + toRow) / 2][(fromCol + toCol) / 2] == PLAYER2) {
      return true;  
    }
  }

  // Player 2 regular piece
  else if (piece == PLAYER2) {
    if (toRow == fromRow - 1 && abs(toCol - fromCol) == 1) {
      return true;  
    } else if (toRow == fromRow - 2 && abs(toCol - fromCol) == 2 && board[(fromRow + toRow) / 2][(fromCol + toCol) / 2] == PLAYER1) {
      return true;  
    }
  }

  // Player 1 king (can move both forward and backward)
  else if (piece == KING1) {
    if (abs(toRow - fromRow) == 1 && abs(toCol - fromCol) == 1) {
      return true;  
    } else if (abs(toRow - fromRow) == 2 && abs(toCol - fromCol) == 2 && (board[(fromRow + toRow) / 2][(fromCol + toCol) / 2] == PLAYER2 || board[(fromRow + toRow) / 2][(fromCol + toCol) / 2] == KING2)) {
      return true;  
    }
  }

  // Player 2 king (can move both forward and backward)
  else if (piece == KING2) {
    if (abs(toRow - fromRow) == 1 && abs(toCol - fromCol) == 1) {
      return true;  
    } else if (abs(toRow - fromRow) == 2 && abs(toCol - fromCol) == 2 && (board[(fromRow + toRow) / 2][(fromCol + toCol) / 2] == PLAYER1 || board[(fromRow + toRow) / 2][(fromCol + toCol) / 2] == KING1)) {
      return true;  
    }
  }

  return false;
}



void handleButtons() {
    // Check if the cursor position has changed
    if (cursorX != lastX || cursorY != lastY) {
        if (lastX != -1 && lastY != -1) {
            (*highlightSquarePtr)(lastY, lastX, ILI9341_BLACK); // Unhighlight previous square
            (*drawSquarePtr)(lastY, lastX);                    // Redraw square under the highlight
        }
        (*highlightSquarePtr)(cursorY, cursorX, HIGHLIGHT_COLOR); // Highlight current square
        lastX = cursorX; // Update lastX to current cursorX
        lastY = cursorY; // Update lastY to current cursorY
    }

    if (digitalRead(BUTTON_UP) == LOW) {
        cursorY = (cursorY - 1 + BOARD_SIZE) % BOARD_SIZE; // Move up
        delay(200); 
    }
    else if (digitalRead(BUTTON_DOWN) == LOW) {
        cursorY = (cursorY + 1) % BOARD_SIZE; // Move down
        delay(200);
    }
    else if (digitalRead(BUTTON_LEFT) == LOW) {
        cursorX = (cursorX - 1 + BOARD_SIZE) % BOARD_SIZE; // Move left
        delay(200);  
    }
    else if (digitalRead(BUTTON_RIGHT) == LOW) {
        cursorX = (cursorX + 1) % BOARD_SIZE; // Move right
        delay(200);
    }

    // Handle coin selection and movement
    if (digitalRead(BUTTON_SELECT) == LOW) {
        if (!pieceSelected) {
            // Check if the selected piece belongs to the current player
            int selectedPiece = board[cursorY][cursorX];
            if ((isPlayer1Turn && (selectedPiece == PLAYER1 || selectedPiece == KING1)) || 
                (!isPlayer1Turn && (selectedPiece == PLAYER2 || selectedPiece == KING2))) {
                selectedX = cursorX;
                selectedY = cursorY;
                pieceSelected = true;
                highlightSquare(cursorY, cursorX, ILI9341_YELLOW); // Highlight selected piece
            }
        } else {
            // Check if the move is valid
            if (isValidMove(board, selectedY, selectedX, cursorY, cursorX)) {
                movePiece(selectedY, selectedX, cursorY, cursorX);
    
                // Un-highlight the previous square
                highlightSquare(selectedY, selectedX, ILI9341_BLACK); 
            }
        }
    }
}





void displayWinnerOrDraw() {
  tft.fillScreen(ILI9341_BLACK);
  tft.setCursor(10, SCREEN_HEIGHT / 2);
  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(2);
  if (scorePlayer1 > scorePlayer2) {
    tft.println("Player 1 Wins!");
  } else if (scorePlayer2 > scorePlayer1) {
    tft.println("Player 2 Wins!");
  } else {
    tft.println("It's a Draw!");
  }
  while (true); // Stop the program
}

void updateTimer() {
  unsigned long currentTime = millis();
  elapsedSeconds = (currentTime - startTime) / 1000; // Calculate elapsed time
  remainingSeconds = TOTAL_TIME - elapsedSeconds; // Update remaining time

  if (remainingSeconds <= 0) {
    displayWinnerOrDraw();
  }
}

void updateScore() {
  tft.fillRect(0, BOARD_SIZE * SQUARE_SIZE + 30, SCREEN_WIDTH, 60, ILI9341_BLACK);  // Clear previous score area
  displayStatus();  // Call displayStatus to redraw the updated scores
}

void displayStatus() {
  updateTimer();  // Update and check timer

  // Display timer
  tft.setCursor(10, BOARD_SIZE * SQUARE_SIZE + 10);
  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(2);
  tft.print("Time: ");
  tft.print(TOTAL_TIME);  // Print the remaining seconds
  tft.print("s");  
  // Display Player 1 score with color indication
  tft.setCursor(10, BOARD_SIZE * SQUARE_SIZE + 30);
  tft.setTextColor(isPlayer1Turn ? ILI9341_GREEN : ILI9341_BLUE);  // Green if Player 1's turn
  tft.print("Player 1: ");
  tft.print(scorePlayer1);

  // Display Player 2 score with color indication
  tft.setCursor(10, BOARD_SIZE * SQUARE_SIZE + 50);
  tft.setTextColor(!isPlayer1Turn ? ILI9341_GREEN : ILI9341_BLUE);  // Green if Player 2's turn
  tft.print("Player 2: ");
  tft.print(scorePlayer2);
}





void loop() {
  handleButtons();
  displayStatus();
}
