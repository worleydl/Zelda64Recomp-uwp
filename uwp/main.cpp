#include <Windows.h>
#include "SDL2/SDL.h"

// If I don't include this for some reason the game_warps don't get initialized?
#include "../include/zelda_debug.h"

extern "C" __declspec(dllimport) void* uwp_GetWindowReference();

int bootstrap(int argc, char** argv)
{
	uwp_GetWindowReference(); // Call once to init reference for other threads

	return SDL_main(argc, argv);
}

int CALLBACK WinMain(HINSTANCE, HINSTANCE, LPSTR argv, int argc)
{
    auto warps = zelda64::game_warps;

	return SDL_WinRTRunApp(bootstrap, NULL);
}
