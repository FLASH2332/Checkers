#include "SPI.h"
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"

// Pin configuration for the TFT display
#define TFT_DC 9
#define TFT_CS 10
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);

// Chessboard and piece setup
#define BOARD_SIZE 8        // 8x8 checkers board
#define SQUARE_SIZE 30      // Size of each square
#define PIECE_RADIUS 10     // Radius of checkers pieces

// Colors for board and pieces
#define WHITE ILI9341_WHITE
#define BLACK ILI9341_BLACK
#define DARK_BROWN 0x8410   // Custom color for dark squares
#define LIGHT_BEIGE 0xFFFF  // Custom color for light squares (beige)
#define PLAYER1_COLOR ILI9341_RED  // Color for Player 1 pieces
#define PLAYER2_COLOR ILI9341_BLUE // Color for Player 2 pieces

// 2D array to represent the checkers board state
// 0 = empty, 1 = player 1 piece, 2 = player 2 piece
int board[BOARD_SIZE][BOARD_SIZE] = {
  {2,0,2,0,0,0,1,0},
  {0,2,0,0,0,1,0,1},
  {2,0,2,0,0,0,1,0},
  {0,2,0,0,0,1,0,1},
  {2,0,2,0,0,0,1,0},
  {0,2,0,0,0,1,0,1},
  {2,0,2,0,0,0,1,0},
  {0,2,0,0,0,1,0,1}
};



// Function declarations (prototypes)
void drawChessboard();
void drawPieces();
void drawPiece(int row, int col, uint16_t color);

void setup() {
  tft.begin();
  tft.setRotation(1);  // Landscape mode
  tft.fillScreen(BLACK);

  // Draw the checkers board
  drawChessboard();

  // Draw the initial pieces
  drawPieces();
}

void loop() {
  // Nothing to do here
}

// Function to draw the checkers board

void drawChessboard() {
  for (int row = 0; row < BOARD_SIZE; row++) {
    for (int col = 0; col < BOARD_SIZE; col++) {
      // Calculate the position of the square
      int x = col * SQUARE_SIZE;
      int y = row * SQUARE_SIZE;

      // Alternate between dark and light squares
      uint16_t color = (row + col) % 2 == 0 ? DARK_BROWN : LIGHT_BEIGE;

      // Draw the square
      tft.fillRect(x, y, SQUARE_SIZE, SQUARE_SIZE, color);
    }
  }
}

// Function to draw the checkers pieces

void drawPieces() {
  for (int row = 0; row < BOARD_SIZE; row++) {
    for (int col = 0; col < BOARD_SIZE; col++) {
      // Get the value of the board at this position
      int piece = board[row][col];

      // If the square is not empty, draw the piece
      if (piece != 0) {
        uint16_t color = piece == 1 ? PLAYER1_COLOR : PLAYER2_COLOR;
        drawPiece(row, col, color);
      }
    }
  }
}

// Function to draw a single checkers piece

void drawPiece(int row, int col, uint16_t color) {
  // Calculate the center of the square
  int x = col * SQUARE_SIZE + SQUARE_SIZE / 2;
  int y = row * SQUARE_SIZE + SQUARE_SIZE / 2;

  // Draw the piece
  tft.fillCircle(x, y, PIECE_RADIUS, color);
}
