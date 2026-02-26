#include <iostream>

#include "raylib.h"

int main()
{
	InitWindow(800, 600, "Game");

	while (!WindowShouldClose())
	{
		// Update

		// Draw

		BeginDrawing();
		ClearBackground(WHITE);

		EndDrawing();
	}
	CloseWindow();

	return 0;
}