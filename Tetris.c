#include <stdio.h>
#include <stdbool.h>
#include <ctype.h>
#include "Utils.h"
#include "DataHandler.h"

// Playfield Map indicator
#define FILL 1
#define SHADOW 2
#define EMPTY 0

// Colors
#define LIGHT_AQUA 11
#define BLUE 9
#define YELLOW 14
#define RED 4
#define LIGHT_RED 12

// Tetris Main Playfield Info
#define TETROMINO_SIZE 4
#define PLAYFIELD_WIDTH 10
#define PLAYFIELD_HEIGHT 20
#define PLAYFIELD_RIGHT_MARGIN 12
#define STARTING_X 3
#define STARTING_Y 0

// Tetris Playfield HUD Info
#define HOLD_HUD_HEIGHT_WIDTH 6
#define HOLD_HUD_RIGHT_MARGIN 3
#define NEXT_HUD_WIDTH 6
#define NEXT_HUD_HEIGHT 18
#define NEXT_HUD_RIGHT_MARGIN 25

// Scores
#define LINE_SCORE 100
#define BLOCK_SCORE 28

// Main Keys
#define KEY_ESC 27
#define KEY_HOLD 67
#define KEY_SPACE 32
#define KEY_UP 72
#define KEY_LEFT 75
#define KEY_RIGHT 77
#define KEY_DOWN 80

//== Tetrus Game Structures
typedef char Playfield;
typedef int Tetromino;
typedef struct PlayfieldLocationEntity {
	int X;
	int Y;
} Location;

// Bounds of a tetromino
typedef struct BoundsEntity {
	int boundaries[TETROMINO_SIZE];
} TetrominoBounds;

// Array copy method
void copyTetromino(
	Tetromino original[TETROMINO_SIZE][TETROMINO_SIZE],
	Tetromino duplicate[TETROMINO_SIZE][TETROMINO_SIZE]
) {
	for(int i = 0; i < TETROMINO_SIZE; i++) {
		for(int j = 0; j < TETROMINO_SIZE; j++) {
			duplicate[i][j] = original[i][j];
		}
	}
}

void clearCurrentTetromino(
	Tetromino currentPiece[TETROMINO_SIZE][TETROMINO_SIZE],
	Location *currentPosition
) {
	for(int i = 0; i < TETROMINO_SIZE; i++) {
		for(int j = 0; j < TETROMINO_SIZE; j++) {
			currentPiece[i][j] = EMPTY;
		}
	}
	
	// Reset user's current coordinates
	currentPosition->X = STARTING_X;
	currentPosition->Y = STARTING_Y;
}

void clearLastNextPiece(
	Tetromino nextPieces[3][TETROMINO_SIZE][TETROMINO_SIZE],
	Playfield nextMap[NEXT_HUD_HEIGHT][NEXT_HUD_WIDTH]
) {
	// Clear the old shape in the next HUD
	for(int idx = 0; idx < 3; idx++) {
		for(int i = 0; i < TETROMINO_SIZE; i++) {
			for(int j = 0; j < TETROMINO_SIZE; j++) {
				if(nextPieces[idx][i][j] == FILL) {
					nextMap[i + 1 + (6 * idx)][j + 1] = EMPTY;
				}
			}
		}
	}
	
	for(int idx = 0; idx < 3; idx++) {
		for(int i = 0; i < TETROMINO_SIZE; i++) {
			for(int j = 0; j < TETROMINO_SIZE; j++) {
				if(idx + 1 >= 3) {
					nextPieces[idx][i][j] = EMPTY;
					continue;
				}
				
				nextPieces[idx][i][j] = nextPieces[idx + 1][i][j];
			}
		}
	}
}

// Get the farthest block to the right
// of a tetromino which has a block in it
TetrominoBounds getOutermostRightBlocks(Tetromino currentPiece[TETROMINO_SIZE][TETROMINO_SIZE]) {
	TetrominoBounds bounds = {{-1, -1, -1, -1}};
	
	for(int row = 0, i = 0; row < TETROMINO_SIZE; row++) {
		int outermostBound = 0;
		for(int column = 0; column < TETROMINO_SIZE; column++) {
			if(currentPiece[row][column] == FILL && column > outermostBound) {
				outermostBound = column;
				bounds.boundaries[i] = column;
			}
		}
		i++;
	}
	
	return bounds;
}

// Get the farthest block to the left
// of a tetromino which has a block in it
TetrominoBounds getOutermostLeftBlocks(Tetromino currentPiece[TETROMINO_SIZE][TETROMINO_SIZE]) {
	TetrominoBounds bounds = {{-1, -1, -1, -1}};
	
	for(int row = 0, i = 0; row < TETROMINO_SIZE; row++) {
		int outermostBound = TETROMINO_SIZE - 1;
		for(int column = TETROMINO_SIZE - 1; column >= 0; column--) {
			if(currentPiece[row][column] == FILL && column < outermostBound) {
				outermostBound = column;
				bounds.boundaries[i] = column;
			}
		}
		i++;
	}
	
	return bounds;
}

// Get the farthest block below of a
// tetromino which has a block in it
TetrominoBounds getOutermostBottomBlocks(Tetromino currentPiece[TETROMINO_SIZE][TETROMINO_SIZE]) {
	TetrominoBounds bounds = {{-1, -1, -1, -1}};
	
	for(int column = 0, i = 0; column < TETROMINO_SIZE; column++) {
		int outermostBound = 0;
		for(int row = 0; row < TETROMINO_SIZE; row++) {
			if(currentPiece[row][column] == FILL && row > outermostBound) {
				outermostBound = row;
				bounds.boundaries[i] = row;
			}
		}
		i++;
	}
	
	return bounds;
}


//===============
void drawPlayfieldBorder() {
	HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(handle, LIGHT_AQUA);
    
    // Playfield is 10x20: https://tetris.fandom.com/wiki/Playfield
	for(int row = 0; row <= PLAYFIELD_HEIGHT + 1; row++) {
		for(int column = 0; column <= PLAYFIELD_WIDTH + 1; column++) {
			moveCursor(column + PLAYFIELD_RIGHT_MARGIN - 1, row + 1);
			// If its a border, print a box
			if(row == 0
				|| column == 0
				|| row == PLAYFIELD_HEIGHT + 1
				|| column == PLAYFIELD_WIDTH + 1)
				printf("■");
		}
		
		printf("\n");
	}
}

void drawHoldHudBorder() {
	HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(handle, LIGHT_AQUA);
	moveCursor(HOLD_HUD_RIGHT_MARGIN, 1);
	
	printf("[ H O L D ]\n");
	for(int row = 0; row <= HOLD_HUD_HEIGHT_WIDTH + 1; row++) {
		for(int column = 0; column <= HOLD_HUD_HEIGHT_WIDTH + 1; column++) {
			moveCursor(column + HOLD_HUD_RIGHT_MARGIN - 1, row + 2);
			if(row == 0
				|| column == 0
				|| row == HOLD_HUD_HEIGHT_WIDTH + 1
				|| column == HOLD_HUD_HEIGHT_WIDTH + 1)
				printf("■");
		}
	}
}

void drawNextHudBorder() {
	HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(handle, LIGHT_AQUA);
	moveCursor(NEXT_HUD_RIGHT_MARGIN, 1);
	
	printf("[ N E X T ]\n");
	for(int row = 0; row <= NEXT_HUD_HEIGHT + 1; row++) {
		for(int column = 0; column <= NEXT_HUD_WIDTH + 1; column++) {
			moveCursor(column + NEXT_HUD_RIGHT_MARGIN - 1, row + 2);
			if(row == 0
				|| column == 0
				|| row == NEXT_HUD_HEIGHT + 1
				|| column == NEXT_HUD_WIDTH + 1)
				printf("■");
		}
	}
}

void drawPlayfield(Playfield playfieldMap[PLAYFIELD_HEIGHT][PLAYFIELD_WIDTH]) {
	HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(handle, BLUE);
    
	for(int row = 0; row < PLAYFIELD_HEIGHT; row++) {
		for(int column = 0; column < PLAYFIELD_WIDTH; column++) {
			moveCursor(column + PLAYFIELD_RIGHT_MARGIN, row + 2);
			
			if(playfieldMap[row][column] == FILL) {
				// Print a yellow block
				SetConsoleTextAttribute(handle, YELLOW);
				printf("■");
				SetConsoleTextAttribute(handle, BLUE);
				continue;
			}
			
			// If its an empty cell then print our background
			if(playfieldMap[row][column] == EMPTY) {
				printf("░");
				continue;
			}
			
			if(playfieldMap[row][column] == SHADOW) {
				SetConsoleTextAttribute(handle, RED);
				printf("░");
				SetConsoleTextAttribute(handle, BLUE);
				continue;
			}
		}
	}
}

void drawHoldHud(Playfield holdMap[HOLD_HUD_HEIGHT_WIDTH][HOLD_HUD_HEIGHT_WIDTH]) {
	HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(handle, BLUE);
    
	for(int row = 0; row < HOLD_HUD_HEIGHT_WIDTH; row++) {
		for(int column = 0; column < HOLD_HUD_HEIGHT_WIDTH; column++) {
			moveCursor(column + HOLD_HUD_RIGHT_MARGIN, row + 3);
			
			// If its an empty cell then print our background
			if(holdMap[row][column] == EMPTY) {
				printf("░");
				continue;
			}
			
			// Print a yellow block
			SetConsoleTextAttribute(handle, YELLOW);
			printf("■");
			SetConsoleTextAttribute(handle, BLUE);
		}
	}
}

void drawNextHud(Playfield nextMap[NEXT_HUD_HEIGHT][NEXT_HUD_WIDTH]) {
	HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(handle, BLUE);
    
	for(int row = 0; row < NEXT_HUD_HEIGHT; row++) {
		for(int column = 0; column < NEXT_HUD_WIDTH; column++) {
			moveCursor(column + NEXT_HUD_RIGHT_MARGIN, row + 3);
			
			// If its an empty cell then print our background
			if(nextMap[row][column] == EMPTY) {
				printf("░");
				continue;
			}
			
			// Print a yellow block
			SetConsoleTextAttribute(handle, YELLOW);
			printf("■");
			SetConsoleTextAttribute(handle, BLUE);
		}
	}
}

void drawJumbotronHud(int score, int linesCleared, int level) {
	HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(handle, LIGHT_AQUA);
    
	moveCursor(HOLD_HUD_RIGHT_MARGIN - 1, HOLD_HUD_HEIGHT_WIDTH * 2);
	printf("[ S C O R E ]");
	
	moveCursor(HOLD_HUD_RIGHT_MARGIN - 1, HOLD_HUD_HEIGHT_WIDTH * 2 + 1);
	printf("%d", score);
	
	
	moveCursor(HOLD_HUD_RIGHT_MARGIN - 1, HOLD_HUD_HEIGHT_WIDTH * 2 + 4);
	printf("[ L I N E S ]");

	moveCursor(HOLD_HUD_RIGHT_MARGIN - 1, HOLD_HUD_HEIGHT_WIDTH * 2 + 5);
	printf("%d", linesCleared);
	
	
	moveCursor(HOLD_HUD_RIGHT_MARGIN - 1, HOLD_HUD_HEIGHT_WIDTH * 2 + 8);
	printf("[ L E V E L ]");

	moveCursor(HOLD_HUD_RIGHT_MARGIN - 1, HOLD_HUD_HEIGHT_WIDTH * 2 + 9);
	printf("%d", level);
}

void drawPiece(
	Playfield playfieldMap[PLAYFIELD_HEIGHT][PLAYFIELD_WIDTH],
	Tetromino currentPiece[TETROMINO_SIZE][TETROMINO_SIZE],
	int *rowOfShadow,
	Location *currentPosition
) {
	// Draw the shadow of the piece first
	*rowOfShadow = currentPosition->Y;
	TetrominoBounds bounds = getOutermostBottomBlocks(currentPiece);
	
	// Find the nearest boundary to the piece
	bool shouldStop = false;
	while(!shouldStop) {
		for(int column = 0; column < TETROMINO_SIZE; column++) {
			int columnBoundary = bounds.boundaries[column];
			if(columnBoundary == -1 || columnBoundary > 4)
				continue;
				
			if(*rowOfShadow + columnBoundary + 1 >= PLAYFIELD_HEIGHT ||
				playfieldMap[*rowOfShadow + columnBoundary + 1][currentPosition->X + column] == FILL) {
				shouldStop = true;
				break;
			}
		}
		(*rowOfShadow) += 1;
	}
	
	// Lastly, draw the main block
	for(int i = 0; i < TETROMINO_SIZE; i++) {
		for(int j = 0; j < TETROMINO_SIZE; j++) {
			if(currentPiece[i][j] == FILL) {
				playfieldMap[*rowOfShadow - 1 + i][currentPosition->X + j] = SHADOW;
				playfieldMap[currentPosition->Y + i][currentPosition->X + j] = currentPiece[i][j];
			}
		}
	}
}

void drawHeldPiece(
	Playfield holdMap[HOLD_HUD_HEIGHT_WIDTH][HOLD_HUD_HEIGHT_WIDTH],
	Tetromino currentPiece[TETROMINO_SIZE][TETROMINO_SIZE]
) {
	for(int i = 0; i < TETROMINO_SIZE; i++) {
		for(int j = 0; j < TETROMINO_SIZE; j++) {
			if(currentPiece[i][j] == FILL)
				holdMap[i + 1][j + 1] = currentPiece[i][j];
		}
	}
}

void drawNextPieces(
	Playfield nextMap[NEXT_HUD_HEIGHT][NEXT_HUD_WIDTH],
	Tetromino nextPiece[3][TETROMINO_SIZE][TETROMINO_SIZE]
) {
	for(int idx = 0; idx < 3; idx++) {
		for(int i = 0; i < TETROMINO_SIZE; i++) {
			for(int j = 0; j < TETROMINO_SIZE; j++) {
				if(nextPiece[idx][i][j] == FILL)
					nextMap[i + 1 + (6 * idx)][j + 1] = nextPiece[idx][i][j];
			}
		}
	}
}

void removePiece(
	Playfield playfieldMap[PLAYFIELD_HEIGHT][PLAYFIELD_WIDTH],
	Tetromino currentPiece[TETROMINO_SIZE][TETROMINO_SIZE],
	int *rowOfShadow,
	Location currentPosition
) {
	for(int i = 0; i < TETROMINO_SIZE; i++) {
		for(int j = 0; j < TETROMINO_SIZE; j++) {
			if(currentPiece[i][j] == FILL) {
				playfieldMap[currentPosition.Y + i][currentPosition.X + j] = EMPTY;
				playfieldMap[*rowOfShadow - 1 + i][currentPosition.X + j] = EMPTY;
			}
		}
	}
}

void generateMaps(
	Playfield playfieldMap[PLAYFIELD_HEIGHT][PLAYFIELD_WIDTH],
	Playfield holdMap[HOLD_HUD_HEIGHT_WIDTH][HOLD_HUD_HEIGHT_WIDTH],
	Playfield nextMap[NEXT_HUD_HEIGHT][NEXT_HUD_WIDTH]
) {
	for(int i = 0; i < PLAYFIELD_HEIGHT; i++) {
		for(int j = 0; j < PLAYFIELD_WIDTH; j++) {
			playfieldMap[i][j] = EMPTY;
		}
	}
	
	for(int i = 0; i < HOLD_HUD_HEIGHT_WIDTH; i++) {
		for(int j = 0; j < HOLD_HUD_HEIGHT_WIDTH; j++) {
			holdMap[i][j] = EMPTY;
		}
	}
	
	for(int i = 0; i < NEXT_HUD_HEIGHT; i++) {
		for(int j = 0; j < NEXT_HUD_WIDTH; j++) {
			nextMap[i][j] = EMPTY;
		}
	}
}

void generateTetromino(Tetromino currentPiece[TETROMINO_SIZE][TETROMINO_SIZE]) {
	// There are 7 tetromino/shapes and
	// we can represent them all
	// thru a 4x4 grid/array
	
	// https://tetris.fandom.com/wiki/Tetromino
	Tetromino listOfTetrominos[7][TETROMINO_SIZE][TETROMINO_SIZE] = {
		/* I Tetromino */ { {0, 1, 0, 0}, {0, 1, 0, 0}, {0, 1, 0, 0}, {0, 1, 0, 0} },
		/* J Tetromino */ { {0, 0, 0, 0}, {0, 1, 0, 0}, {0, 1, 1, 1}, {0, 0, 0, 0} },
		/* L Tetromino */ { {0, 0, 0, 0}, {0, 0, 1, 0}, {1, 1, 1, 0}, {0, 0, 0, 0} },
		/* O Tetromino */ { {0, 0, 0, 0}, {0, 1, 1, 0}, {0, 1, 1, 0}, {0, 0, 0, 0} },
		/* S Tetromino */ { {0, 0, 0, 0}, {0, 0, 1, 1}, {0, 1, 1, 0}, {0, 0, 0, 0} },
		/* T Tetromino */ { {0, 0, 0, 0}, {0, 1, 0, 0}, {1, 1, 1, 0}, {0, 0, 0, 0} },
		/* Z Tetromino */ { {0, 0, 0, 0}, {1, 1, 0, 0}, {0, 1, 1, 0}, {0, 0, 0, 0} }
	};
	
	// Generate a random tetromino
	copyTetromino(listOfTetrominos[rand() % 7], currentPiece);
}

//===================


// Check if it will hit a block
bool isInBoundary(
	Tetromino currentPiece[TETROMINO_SIZE][TETROMINO_SIZE],
	Playfield playfieldMap[PLAYFIELD_HEIGHT][PLAYFIELD_WIDTH],
	Location currentPosition,
	int movement
) {
	TetrominoBounds bounds;
	bool isInBoundaryDown, isInBoundaryLeft, isInBoundaryRight;
	
	switch(movement) {
		case 1: // Moving right
			bounds = getOutermostRightBlocks(currentPiece);
			
			for(int row = 0; row < TETROMINO_SIZE; row++) {
				int rowBoundary = bounds.boundaries[row];
				if(rowBoundary == -1)
					continue;
					
				if(currentPosition.X + rowBoundary + 1 >= PLAYFIELD_WIDTH ||
					playfieldMap[currentPosition.Y + row][currentPosition.X + rowBoundary + 1] == FILL)
					return true;
			}
			break;
		case 2: // Moving left
			bounds = getOutermostLeftBlocks(currentPiece);
			
			for(int row = 0; row < TETROMINO_SIZE; row++) {
				int rowBoundary = bounds.boundaries[row];
				if(rowBoundary == -1)
					continue;
					
				if(currentPosition.X + rowBoundary - 1 <= -1 ||
					playfieldMap[currentPosition.Y + row][currentPosition.X + rowBoundary - 1] == FILL)
					return true;
			}
			break;
		case 3: // Moving down
			bounds = getOutermostBottomBlocks(currentPiece);
			
			for(int column = 0; column < TETROMINO_SIZE; column++) {
				int columnBoundary = bounds.boundaries[column];
				if(columnBoundary == -1 || columnBoundary > 4)
					continue;
					
				if(currentPosition.Y + columnBoundary + 1 >= PLAYFIELD_HEIGHT ||
					playfieldMap[currentPosition.Y + columnBoundary + 1][currentPosition.X + column] == FILL)
					return true;
			}
			break;
	}
	
	return false;
}

// Will rotate a tetromino 90 degrees
void rotateTetromino(
	Tetromino currentPiece[TETROMINO_SIZE][TETROMINO_SIZE],
	Playfield playfieldMap[PLAYFIELD_HEIGHT][PLAYFIELD_WIDTH],
	int *rowOfShadow,
	Location *currentPosition
) {
	Tetromino rotatedTetromino[TETROMINO_SIZE][TETROMINO_SIZE];
	
	if(isInBoundary(currentPiece, playfieldMap, *currentPosition, 1)
		|| isInBoundary(currentPiece, playfieldMap, *currentPosition, 2)
		|| isInBoundary(currentPiece, playfieldMap, *currentPosition, 3))
		return;
	
	for(int i = 0, k = 0; i < TETROMINO_SIZE; i++) {
		for(int j = TETROMINO_SIZE - 1, l = 0; j >= 0; j--) {	
			rotatedTetromino[k][l++] = currentPiece[j][i];
		}
		k++;
	}
	
	if(isInBoundary(currentPiece, playfieldMap, *currentPosition, 1)
		|| isInBoundary(currentPiece, playfieldMap, *currentPosition, 2)
		|| isInBoundary(currentPiece, playfieldMap, *currentPosition, 3))
		return;
		
	removePiece(playfieldMap, currentPiece, rowOfShadow, *currentPosition);
	
	copyTetromino(rotatedTetromino, currentPiece);
	
	drawPiece(playfieldMap, currentPiece, rowOfShadow, currentPosition);
}

void moveLeft(
	Tetromino currentPiece[TETROMINO_SIZE][TETROMINO_SIZE],
	Playfield playfieldMap[PLAYFIELD_HEIGHT][PLAYFIELD_WIDTH],
	int *rowOfShadow,
	Location *currentPosition
) {
	if(isInBoundary(currentPiece, playfieldMap, *currentPosition, 2) == true)
		return;
	
	// Clear the old shape in the playfield
	removePiece(playfieldMap, currentPiece, rowOfShadow, *currentPosition);
	
	currentPosition->X -= 1;
	
	drawPiece(playfieldMap, currentPiece, rowOfShadow, currentPosition);
}

void moveRight(
	Tetromino currentPiece[TETROMINO_SIZE][TETROMINO_SIZE],
	Playfield playfieldMap[PLAYFIELD_HEIGHT][PLAYFIELD_WIDTH],
	int *rowOfShadow,
	Location *currentPosition
) {
	if(isInBoundary(currentPiece, playfieldMap, *currentPosition, 1) == true)
		return;
	
	// Clear the old shape in the playfield
	removePiece(playfieldMap, currentPiece, rowOfShadow, *currentPosition);
	
	currentPosition->X += 1;
	
	drawPiece(playfieldMap, currentPiece, rowOfShadow, currentPosition);
}

void moveDown(
	Tetromino currentPiece[TETROMINO_SIZE][TETROMINO_SIZE],
	Playfield playfieldMap[PLAYFIELD_HEIGHT][PLAYFIELD_WIDTH],
	int *rowOfShadow,
	Location *currentPosition
) {
	if(isInBoundary(currentPiece, playfieldMap, *currentPosition, 3) == true) {
		clearCurrentTetromino(currentPiece, currentPosition);
		return;
	}
	
	// Clear the old shape in the playfield
	removePiece(playfieldMap, currentPiece, rowOfShadow, *currentPosition);
	
	currentPosition->Y += 1;

	drawPiece(playfieldMap, currentPiece, rowOfShadow, currentPosition);
}

void moveDownInstant(
	Tetromino currentPiece[TETROMINO_SIZE][TETROMINO_SIZE],
	Playfield playfieldMap[PLAYFIELD_HEIGHT][PLAYFIELD_WIDTH],
	int *rowOfShadow,
	Location *currentPosition
) {	
	do {
		moveDown(currentPiece, playfieldMap, rowOfShadow, currentPosition);
	} while(currentPosition->Y > STARTING_Y);
}

void holdTetromino(
	Tetromino currentPiece[TETROMINO_SIZE][TETROMINO_SIZE],
	Tetromino heldPiece[TETROMINO_SIZE][TETROMINO_SIZE],
	Playfield holdMap[HOLD_HUD_HEIGHT_WIDTH][HOLD_HUD_HEIGHT_WIDTH],
	Playfield playfieldMap[PLAYFIELD_HEIGHT][PLAYFIELD_WIDTH],
	int *rowOfShadow,
	Location *currentPosition,
	bool shouldSwap
) {
	Tetromino tempPiece[TETROMINO_SIZE][TETROMINO_SIZE];
	
	// Clear the old shape in the hold HUD
	for(int i = 0; i < TETROMINO_SIZE; i++) {
		for(int j = 0; j < TETROMINO_SIZE; j++) {
			if(heldPiece[i][j] == FILL) {
				holdMap[i + 1][j + 1] = EMPTY;
			}
		}
	}
	
	// Clear the old shape in the playfield
	removePiece(playfieldMap, currentPiece, rowOfShadow, *currentPosition);
	
	drawHeldPiece(holdMap, currentPiece);
	
	if(shouldSwap) {
		// Reset piece position
		currentPosition->X = STARTING_X;
		currentPosition->Y = STARTING_Y;
		
		drawPiece(playfieldMap, heldPiece, rowOfShadow, currentPosition);
		
		 // Swap the current piece to the held piece
		copyTetromino(heldPiece, tempPiece);
		copyTetromino(currentPiece, heldPiece);
		copyTetromino(tempPiece, currentPiece);
	}
	else {
		copyTetromino(currentPiece, heldPiece);
		clearCurrentTetromino(currentPiece, currentPosition);
	}
}

void checkForLines(
	Playfield playfieldMap[PLAYFIELD_HEIGHT][PLAYFIELD_WIDTH],
	int *score,
	int *linesCleared
) {
	for(int row = 0; row < PLAYFIELD_HEIGHT; row++) {
		bool isPerfectLine = true;
		for(int column = 0; column < PLAYFIELD_WIDTH; column++) {
			if(playfieldMap[row][column] == EMPTY) {
				isPerfectLine = false;
				break;
			}
		}
		
		if(isPerfectLine) {
			*score += LINE_SCORE;
			*linesCleared += 1;
			for(int _row = row; _row > 1; _row--) {
				for(int _column = 0; _column < PLAYFIELD_WIDTH; _column++) {
					// Remove the line then move down the next blocks
					playfieldMap[_row][_column] = playfieldMap[_row - 1][_column];
				}
			}
		}
	}
}

void getUsername(char username[10]) {
	char confirm, temp[1024];
	
	getUsernameStart:
	system("cls");
	moveCursor(0, 10);
	int currentX = printInCenter("Hi, player! What's your name?\n");
	moveCursor(0, 12);
	printSpaces(currentX);
	showCursor();
	
	gets(username);
	hideCursor();
	
	if(isUsernameTaken(username)) {
		system("cls");
		moveCursor(0, 10);
		sprintf(temp, "Seems like \"%s\" has been used already.\n", username);
		printInCenter(temp);
		printInCenter("Press any key to try again...");
		getch();
		goto getUsernameStart;
	}
	
	confirmUsername:
	system("cls");
	moveCursor(0, 10);
	sprintf(temp, "Are you really %s? [y/n]", username);
	printInCenter(temp);
	
	confirm = getch();
	switch(confirm) {
		case 'y': case 'Y':
			break;
		case 'n': case 'N':
			goto getUsernameStart;
		default:
			goto confirmUsername;
	}
}

void showInstructions(char *username) {
	char temp[1024], confirm;
	
	startOfInstructions:
	system("cls");
	moveCursor(0, 1);
	sprintf(temp, "Hello, %s!\n", username);
	printInCenter(temp);
	printInCenter("Tetris is a puzzle video game.\n");
	printInCenter("Players attempt to clear as many lines as possible\n");
	printInCenter("by completing horizontal rows of blocks without empty space,\n");
	printInCenter("but if the Tetriminos surpass the Skyline the game is over!\n");
	printInCenter("It might sound simple, but strategy and speed\n");
	printInCenter("can go a long way!\n\n");
	printInCenter("Are YOU up for the challenge?\n\n\n");
	
	int currentY = getCurrentY() - 1;
	
	moveCursor(0, currentY + 2);
	int justifiedX = printInCenter("=[ C O N T R O L S ]=\n\n");
	
	printSpaces(justifiedX);
	printf("[↑] Rotate\n");
	
	printSpaces(justifiedX);
	printf("[↓] Soft drop\n");
	
	printSpaces(justifiedX);
	printf("[→] Move right\n");
	
	printSpaces(justifiedX);
	printf("[←] Move left\n");
	
	printSpaces(justifiedX);
	printf("[C] Hold piece\n");
	
	printSpaces(justifiedX);
	printf("[Space] Hard drop\n\n");
	
	
	while(confirm != 'Y' && confirm != 'y') {
		moveCursor(0, currentY);
		printInCenter("Press Y to start the game...");
		Sleep(500);
		
		moveCursor(0, currentY);
		printInCenter("                            ");
		Sleep(500);
		if(kbhit())
			confirm = getch();
	}
}

void showLeaderboards() {
	generateLeaderboards();
	
	FILE *filePointer;
    char line[1024];
    int justifiedX, pressedKey;
	
	system("cls");
	moveCursor(0, 1);
	printInCenter(" =================================================================== \n");
	justifiedX = printInCenter("    L    E    A    D    E    R    B    O    A    R    D    S    \n") + 5;
	printInCenter(" =================================================================== \n");
	printf("    [ RANK ]\t\t[ NAME ]\t[ SCORE ] \n");
	printInCenter(" =================================================================== \n\n");
	
	filePointer = fopen(TEMP_LEADERBOARDS_FILE, "r");
	
	// Print each lines
	while (fgets(line, 1024, filePointer)) {
		moveCursor(2, getCurrentY());
	    printf("%s", line);
    }
    
    fclose(filePointer);
    
    moveCursor(0, 23);
	printInCenter("Press ESC to go back...");
	
	pressedKey = getch();
	while(pressedKey != KEY_ESC) pressedKey = getch();
	
	remove(TEMP_LEADERBOARDS_FILE);
}

void startGame(
	char *username,
	int *score,
	int *linesCleared,
	int *level
) {
	char choice;
	
	startOfGame:
	choice = -1;
	
	// Reset stats
	*score = 0;
	*linesCleared = 0;
	*level = 1;
	
	system("cls");
	moveCursor(0, 5);
	printInCenter("[ STARTING IN ]");
	int loading = 3;
	while(loading > -1) {
		moveCursor(0, 10);
		if(loading == 3) {
			printInCenter("----.  \n");
			printInCenter("'.-.  | \n");
			printInCenter("  .' <  \n");
			printInCenter("/'-'  | \n");
			printInCenter("`----'   ");
		}
		else if(loading == 2) {
			printInCenter(" ,---.  \n");
			printInCenter("'.-.  \ \n");
			printInCenter(" .-' .' \n");
			printInCenter("/   '-. \n");
			printInCenter("'-----' ");
		}
		else if(loading == 1) {
			printInCenter(" ,--. \n");
			printInCenter("/   | \n");
			printInCenter("`|  | \n");
			printInCenter(" |  | \n");
			printInCenter(" `--' ");
		}
		else {
			printInCenter("                 ,---. \n");
			printInCenter(" ,----.          |   | \n");
			printInCenter("'  .-./    ,---. |  .' \n");
			printInCenter("|  | .---.| .-. ||  |  \n");
			printInCenter("'  '--'  |' '-' '`--'  \n");
			printInCenter(" `------'  `---' .--.  \n");
			printInCenter("                 '--'  ");
		}
		
		Sleep(1000);
		loading--;
	}
	
	system("cls");
	srand(time(NULL));
	
	int rowOfShadow = PLAYFIELD_HEIGHT - 1, counter = 0;
	
	Location currentPosition = {STARTING_X, STARTING_Y};
	
	Playfield playfieldMap[PLAYFIELD_HEIGHT][PLAYFIELD_WIDTH],
			  holdMap[HOLD_HUD_HEIGHT_WIDTH][HOLD_HUD_HEIGHT_WIDTH],
			  nextMap[NEXT_HUD_HEIGHT][NEXT_HUD_WIDTH];
			  
	Tetromino currentPiece[TETROMINO_SIZE][TETROMINO_SIZE],
			  nextPiece[3][TETROMINO_SIZE][TETROMINO_SIZE],
			  heldPiece[TETROMINO_SIZE][TETROMINO_SIZE];
	
	generateMaps(playfieldMap, holdMap, nextMap);
	
	drawPlayfieldBorder(); // Initialize playfield border
	drawHoldHudBorder(); // Initialize hold HUD border
	drawNextHudBorder(); // Initialize next HUD border
	
	// Generate first 4 tetromino pieces
	generateTetromino(currentPiece);
	generateTetromino(nextPiece[0]);
	generateTetromino(nextPiece[1]);
	generateTetromino(nextPiece[2]);
	
	// Initialize activity
	drawPlayfield(playfieldMap);
	drawHoldHud(holdMap);
	drawNextHudBorder(nextMap);
	drawPiece(playfieldMap, currentPiece, &rowOfShadow, &currentPosition);
	drawNextPieces(nextMap, nextPiece);
	
	bool isGameOver = false,
		 hasReachedBottom = false,
		 isFirstTimeHolding = true,
		 hasUsedHoldAlready = false;
		 
	while(!isGameOver) {
		if(*linesCleared >= 10)
			*level = (*linesCleared / 10) + 1;
		
		if(hasReachedBottom) {
			counter = 0; // Reset counter to 0
			
			*score += BLOCK_SCORE;
			
			checkForLines(playfieldMap, score, linesCleared);
			copyTetromino(nextPiece[0], currentPiece);
			clearLastNextPiece(nextPiece, nextMap);
			
			generateTetromino(nextPiece[2]);
			
			while(isInBoundary(currentPiece, playfieldMap, currentPosition, 3) && currentPosition.Y > -1)
				currentPosition.Y--;
				
			drawPiece(playfieldMap, currentPiece, &rowOfShadow, &currentPosition);
			drawNextPieces(nextMap, nextPiece);
			
			// Check if its game over
			if(currentPosition.Y < 0) {
				isGameOver = true;
			}
			
			hasReachedBottom = false;
			hasUsedHoldAlready = false;
		}
		
		drawPlayfield(playfieldMap);
		drawHoldHud(holdMap);
		drawNextHud(nextMap);
		drawJumbotronHud(*score, *linesCleared, *level);
		
		int pressedKey = getKeyPress();
		
		// Check if user pressed space
		if(pressedKey == KEY_SPACE) {
			moveDownInstant(currentPiece, playfieldMap, &rowOfShadow, &currentPosition);
			hasReachedBottom = currentPosition.X == STARTING_X && currentPosition.Y == STARTING_Y;
			continue;
		}
		
		// Check if user wants to hold
		if(toupper(pressedKey) == KEY_HOLD && !hasUsedHoldAlready) {
			holdTetromino(currentPiece, heldPiece, holdMap, playfieldMap, &rowOfShadow, &currentPosition, !isFirstTimeHolding);
			hasReachedBottom = true && isFirstTimeHolding;
			isFirstTimeHolding = false;
			hasUsedHoldAlready = true;
			continue;
		}
			
		
		// Get arrow key input
		if(pressedKey == 224 || pressedKey == 0) {
			pressedKey = getch();
			switch(pressedKey) {
				case KEY_UP:
					rotateTetromino(currentPiece, playfieldMap, &rowOfShadow, &currentPosition);
					break;
				case KEY_LEFT:
					moveLeft(currentPiece, playfieldMap, &rowOfShadow, &currentPosition);
					break;
				case KEY_RIGHT:
					moveRight(currentPiece, playfieldMap, &rowOfShadow, &currentPosition);
					break;
				case KEY_DOWN:
					moveDown(currentPiece, playfieldMap, &rowOfShadow, &currentPosition);
					hasReachedBottom = currentPosition.X == STARTING_X && currentPosition.Y == STARTING_Y;
					break;
			}
		}
		
		int goDownCounter = 19 - (9);
		goDownCounter = goDownCounter < 0 ? 0 : goDownCounter;
		
		if(pressedKey == -1
			&& counter % goDownCounter == 0) {
			// Sleep(1000 - (200 * level));
			moveDown(currentPiece, playfieldMap, &rowOfShadow, &currentPosition);
			hasReachedBottom = currentPosition.X == STARTING_X && currentPosition.Y == STARTING_Y;
		}
		
		counter++;
	}
	
	gameOverScreen:
	system("cls");
	moveCursor(0, 2);
	printInCenter(" =================================================================== \n");
	int justifiedX = printInCenter("    G    A    M    E        S    T    A    T    S    \n") + 5;
	printInCenter(" =================================================================== \n\n\n");
	
	printSpaces(justifiedX);
	printf("[ USERNAME ]:\t\t%s\n\n", username);
	
	printSpaces(justifiedX);
	printf("[ SCORE ]:\t\t\t%d\n\n", *score);
	
	printSpaces(justifiedX);
	printf("[ LINES CLEARED ]:\t\t%d\n\n", *linesCleared);
	
	printSpaces(justifiedX);
	printf("[ LEVEL REACHED ]:\t\t%d\n\n\n\n", *level);
	int currentY = getCurrentY();
	
	while(toupper(choice) != 'S'
		&& toupper(choice) != 'R') {
		moveCursor(0, currentY);
		printInCenter("< Press [S/R] to save or try again >");
		Sleep(500);
		
		moveCursor(0, currentY);
		printInCenter("                                    ");
		Sleep(500);
		if(kbhit())
			choice = getch();
	}
	
	switch(choice) {
		case 'S': case 's':
			system("cls");
			moveCursor(0, 10);
			printInCenter("Saving");
			
			saveUserData(username, *score);
			for(int i = 0; i < 3; i++) {
				printf(".");
				Sleep(500);
			}
			
			moveCursor(0, 21);
			printInCenter("Press any key to go back to menu...");
			getch();
			break;
		case 'R': case 'r':
			// Confirm if user wants to retry
			do {
				system("cls");
				moveCursor(0, 10);
				printInCenter("Are you sure you want to try again? [y/n]");
				
				choice = getch();
				if(toupper(choice) == 'Y')
					goto startOfGame;
				else if(toupper(choice) == 'N') {
					choice = -1;
					goto gameOverScreen;
				}
			} while(toupper(choice) != 'Y' && toupper(choice) != 'N');
			break;
	}
}
// ===== MAIN GAME ENGINE =======\\

int main(void) {
	char username[10];
	int score = 0, linesCleared = 0, level = 1;
	
	system("color 7");
	system("mode con: cols=69 lines=25");
	system("CHCP 65001");
	system("cls");
	
	HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(handle, LIGHT_AQUA);
    
    char header[6][1024] = {
		"\t     _____    _        _       _         _____ \n",
		"\t    |_   _|  | |      (_)     (_)       /  __ \\\n",
		"\t      | | ___| |_ _ __ _ ___   _ _ __   | /  \\/\n",
		"\t      | |/ _ \\ __| '__| / __| | | '_ \\  | |    \n",
		"\t      | |  __/ |_| |  | \\__ \\ | | | | | | \\__/\\\n",
		"\t      \\_/\\___|\\__|_|  |_|___/ |_|_| |_|  \\____/\n"
	};
    
	for(int i = 0; i < 6; i++) {
		int counter = 0;
		char *string = header[i];
		
		printf("%s", string);
		Sleep(200);
	}
	
	// 3 seconds loading
	hideCursor();
	int loading = 1, currentY = getCurrentY();
	while(loading <= 3) {
		moveCursor(0, currentY + 10);
		printInCenter("Loading the game, please wait...");
		Sleep(500);
		
		moveCursor(0, currentY + 10);
		printInCenter("                                ");
		Sleep(500);
		loading++;
	}
	
	bool shouldStop = false, shouldPrintHeader = false;
	char choice;
	do {
		if(shouldPrintHeader) {
			printf("\t     _____    _        _       _         _____ \n");
			printf("\t    |_   _|  | |      (_)     (_)       /  __ \\\n");
			printf("\t      | | ___| |_ _ __ _ ___   _ _ __   | /  \\/\n");
			printf("\t      | |/ _ \\ __| '__| / __| | | '_ \\  | |    \n");
			printf("\t      | |  __/ |_| |  | \\__ \\ | | | | | | \\__/\\\n");
			printf("\t      \\_/\\___|\\__|_|  |_|___/ |_|_| |_|  \\____/\n");
		}
		
		moveCursor(0, currentY + 5);
		printInCenter("[S] Play Now!  \n");
		printInCenter("[R] Rankings   \n");
		printInCenter("[Esc] Exit    \n\n\n");
		printInCenter("Press any of the option\n\n");
		choice = getch();
		system("cls");
		
		switch(choice) {
			case 'S': case 's':
				if(username[0] == '\0')
					getUsername(username);
				showInstructions(username);
				startGame(username, &score, &linesCleared, &level);
				break;
			case 'R': case 'r':
				showLeaderboards();
				break;
			case KEY_ESC:
				shouldStop = true;
				break;
			default:
				break;
				
		}
		
		system("cls");
		shouldPrintHeader = true;
	} while(!shouldStop);
	
	printf("\n\n");
	
	return 0;
}
