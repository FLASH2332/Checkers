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
#define TOTAL_TIME 45 // Total game time in seconds (20 seconds)
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

int board[BOARD_SIZE][BOARD_SIZE] = {
  {1, 0, 1, 0, 1, 0, 1, 0},
  {0, 1, 0, 1, 0, 1, 0, 1},
  {1, 0, 1, 0, 1, 0, 1, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 2, 0, 2, 0, 2, 0, 2},
  {2, 0, 1, 0, 2, 0, 2, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},

};

// Function prototypes
void drawChessboard();
void drawPieces(int board[BOARD_SIZE][BOARD_SIZE]);
void drawCoin(int row, int col, uint16_t color);
void movePiece(int fromRow, int fromCol, int toRow, int toCol);
void highlightSquare(int row, int col, uint16_t color);
void drawSquare(int row, int col);
void handleButtons();
void updateTimer();
void updateScore();
void displayStatus();

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

void drawPieces(int board[BOARD_SIZE][BOARD_SIZE]) {
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
  tft.drawRect(x, y, SQUARE_SIZE, SQUARE_SIZE, color);
}

void drawSquare(int row, int col) {
  // Draw the square itself (alternating colors)
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


void movePiece(int fromRow, int fromCol, int toRow, int toCol) {
  if (board[toRow][toCol] == 0) {
    if (abs(toRow - fromRow) == 2 && abs(toCol - fromCol) == 2) {
      int jumpedRow = (fromRow + toRow) / 2;
      int jumpedCol = (fromCol + toCol) / 2;
      board[jumpedRow][jumpedCol] = 0;
      drawSquare(jumpedRow, jumpedCol);

      // Update score when capturing a piece
      if (board[fromRow][fromCol] == 1) {
        scorePlayer1+=1;
      } else if (board[fromRow][fromCol] == 2) {
        scorePlayer2+=1;
      }
      updateScore();  // Update score after capturing
    }

    // Move the piece to the new square
    board[toRow][toCol] = board[fromRow][fromCol];
    board[fromRow][fromCol] = 0;

    // Check for promotion to king
    if (board[toRow][toCol] == 1 && toRow == 7) { // Player 1 promotes
      board[toRow][toCol] = -1; // Promote to king
      scorePlayer1 += 2; // Add bonus points
      updateScore(); // Update score for promotion
    } else if (board[toRow][toCol] == 2 && toRow == 0) { // Player 2 promotes
      board[toRow][toCol] = -2; // Promote to king
      scorePlayer2 += 2; // Add bonus points
      updateScore(); // Update score for promotion
    }

    // Redraw the moved squares
    drawSquare(fromRow, fromCol);
    drawSquare(toRow, toCol);
    drawPieces(board); // Redraw pieces on the board
  }
}


bool isValidMove(int board[BOARD_SIZE][BOARD_SIZE], int fromRow, int fromCol, int toRow, int toCol) {
  if (board[toRow][toCol] != 0) {
    return false;  // Destination square must be empty
  }

  int piece = board[fromRow][fromCol];

  // Player 1 regular piece
  if (piece == 1) {
    if (toRow == fromRow + 1 && abs(toCol - fromCol) == 1) {
      return true;  // Normal move
    } else if (toRow == fromRow + 2 && abs(toCol - fromCol) == 2 && board[(fromRow + toRow) / 2][(fromCol + toCol) / 2] == 2) {
      return true;  // Capture move
    }
  }

  // Player 2 regular piece
  else if (piece == 2) {
    if (toRow == fromRow - 1 && abs(toCol - fromCol) == 1) {
      return true;  // Normal move
    } else if (toRow == fromRow - 2 && abs(toCol - fromCol) == 2 && board[(fromRow + toRow) / 2][(fromCol + toCol) / 2] == 1) {
      return true;  // Capture move
    }
  }

  // Player 1 king (can move both forward and backward)
  else if (piece == -1) {
    if (abs(toRow - fromRow) == 1 && abs(toCol - fromCol) == 1) {
      return true;  // Normal move in any direction
    } else if (abs(toRow - fromRow) == 2 && abs(toCol - fromCol) == 2 && (board[(fromRow + toRow) / 2][(fromCol + toCol) / 2] == 2 || board[(fromRow + toRow) / 2][(fromCol + toCol) / 2] == -2)) {
      return true;  // Capture move in any direction
    }
  }

  // Player 2 king (can move both forward and backward)
  else if (piece == -2) {
    if (abs(toRow - fromRow) == 1 && abs(toCol - fromCol) == 1) {
      return true;  // Normal move in any direction
    } else if (abs(toRow - fromRow) == 2 && abs(toCol - fromCol) == 2 && (board[(fromRow + toRow) / 2][(fromCol + toCol) / 2] == 1 || board[(fromRow + toRow) / 2][(fromCol + toCol) / 2] == -1)) {
      return true;  // Capture move in any direction
    }
  }

  return false;
}

void handleButtons() {
  // Highlight only when the cursor moves
  if (cursorX != lastX || cursorY != lastY) {
    if (lastX != -1 && lastY != -1) {
      highlightSquare(lastY, lastX, ILI9341_BLACK);  // Unhighlight previous square
      drawSquare(lastY, lastX);                      // Redraw square under the highlight
    }

    highlightSquare(cursorY, cursorX, ILI9341_YELLOW); // Highlight current square
    lastX = cursorX;
    lastY = cursorY;
  }

  // Handle cursor movement
  if (digitalRead(BUTTON_UP) == LOW) {
    cursorY = (cursorY - 1 + BOARD_SIZE) % BOARD_SIZE;  
    delay(200); 
  }
  else if (digitalRead(BUTTON_DOWN) == LOW) {
    cursorY = (cursorY + 1) % BOARD_SIZE;
    delay(200);
  }
  else if (digitalRead(BUTTON_LEFT) == LOW) {
    cursorX = (cursorX - 1 + BOARD_SIZE) % BOARD_SIZE;   
    delay(200);  
  }
  else if (digitalRead(BUTTON_RIGHT) == LOW) {
    cursorX = (cursorX + 1) % BOARD_SIZE;  
    delay(200);
  }

  // Handle coin selection and movement
  if (digitalRead(BUTTON_SELECT) == LOW) {
    if (!pieceSelected) {
      selectedX = cursorX;
      selectedY = cursorY;
      pieceSelected = true;
      highlightSquare(cursorY, cursorX, ILI9341_YELLOW);
    } else {
      if (isValidMove(board, selectedY, selectedX, cursorY, cursorX)) {
        movePiece(selectedY, selectedX, cursorY, cursorX);
      }
      pieceSelected = false;
    }
    delay(200); 
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
  // Clear the area where scores are displayed
  tft.fillRect(0, BOARD_SIZE * SQUARE_SIZE + 30, SCREEN_WIDTH, 60, ILI9341_BLACK);  // Clear previous score area
  displayStatus();  // Call displayStatus to redraw the updated scores
}

void displayStatus() {
  updateTimer();  // Update and check timer
  tft.setCursor(10, BOARD_SIZE * SQUARE_SIZE + 10); // Position for time display
  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(2);
  tft.print("Time: ");
  tft.print(TOTAL_TIME);  // Print the remaining seconds
  tft.print("s");  // Include "s" for seconds

  // Set cursor for player 1 score
  tft.setCursor(10, BOARD_SIZE * SQUARE_SIZE + 30);
  tft.print("Player 1: ");
  tft.print(scorePlayer1);  // Print player 1 score

  // Set cursor for player 2 score
  tft.setCursor(10, BOARD_SIZE * SQUARE_SIZE + 50);
  tft.print("Player 2: ");
  tft.print(scorePlayer2);  // Print player 2 score
}




void loop() {
  handleButtons();
  displayStatus();
}

