#include <windows.h>

inline void SetConsoleScreen(SHORT x = 157, SHORT y = 200)
{
	COORD WindowSize;
	WindowSize.X = 157;
	WindowSize.Y = 200;
	SetConsoleScreenBufferSize(GetStdHandle(STD_OUTPUT_HANDLE), WindowSize);
	SetConsoleDisplayMode(GetStdHandle(STD_OUTPUT_HANDLE),CONSOLE_WINDOWED_MODE,&WindowSize);
}
