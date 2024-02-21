#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_ttf.h>

/**
 * Initialize SDL
 * Return 0 if failed to execute
 * Return 1 if no errors
 */
int initSDL(SDL_Window** window, SDL_Renderer** renderer);

/**
 * Quit SDL
 */
void quitSDL(SDL_Window* window, SDL_Renderer* renderer);