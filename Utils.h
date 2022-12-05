#include <stdio.h>
#include <Windows.h>
#include <conio.h>

void hideCursor() {
	HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO info;
    info.dwSize = 100;
    info.bVisible = FALSE;
    SetConsoleCursorInfo(consoleHandle, &info);
}

void showCursor() {
	HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO info;
    info.dwSize = 100;
    info.bVisible = TRUE;
    SetConsoleCursorInfo(consoleHandle, &info);
}

void moveCursor(int positionX, int positionY) {
	COORD coordinates;
    coordinates.X = 2 * positionX;
    coordinates.Y = positionY;
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coordinates);
}

int getKeyPress() {
    return kbhit() ? getch() : -1;
}

int getCurrentX() {
	HANDLE hConsoleOutput = GetStdHandle (STD_OUTPUT_HANDLE);
	CONSOLE_SCREEN_BUFFER_INFO csbi = { };
	BOOL ok = GetConsoleScreenBufferInfo (hConsoleOutput, &csbi);
	return csbi.dwCursorPosition.X;
}

int getCurrentY() {
	HANDLE hConsoleOutput = GetStdHandle (STD_OUTPUT_HANDLE);
	CONSOLE_SCREEN_BUFFER_INFO csbi = { };
	BOOL ok = GetConsoleScreenBufferInfo (hConsoleOutput, &csbi);
	return csbi.dwCursorPosition.Y;
}

void printSpaces(int n) {
    for (int i = 0; i < n; i++) printf(" ");
}

int printInCenter(char* text) {
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    int width, midPoint;

    // Get width of terminal/console
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
    width = csbi.srWindow.Right - csbi.srWindow.Left + 1;
    
    midPoint = width / 2;
    printSpaces(midPoint - (strlen(text) / 2));
    int currentX = getCurrentX();
    printf("%s", text);
	return currentX;
}