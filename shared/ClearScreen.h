#pragma once
#include <windows.h>

void clearScreen(char fill = ' ') {
	COORD topLeft = { 0,0 };
	CONSOLE_SCREEN_BUFFER_INFO info;
	HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
	GetConsoleScreenBufferInfo(console, &info);
	DWORD written, 
		cells = info.dwSize.X * info.dwSize.Y;
	FillConsoleOutputCharacter(console, fill, cells, topLeft, &written);
	FillConsoleOutputAttribute(console, info.wAttributes, cells, topLeft, &written);
	SetConsoleCursorPosition(console, topLeft);
}