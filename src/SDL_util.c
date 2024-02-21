#include "SDL_util.h"
#include "global.h"

int initSDL(SDL_Window** window, SDL_Renderer** renderer)
{
    SDL_Init(SDL_INIT_VIDEO);
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 1024);
    IMG_Init(IMG_INIT_PNG | IMG_INIT_JPG);
    TTF_Init();

    *window = SDL_CreateWindow("Spotifeur", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_RESIZABLE | SDL_WINDOW_SHOWN);
    *renderer = SDL_CreateRenderer(*window, -1, SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_TARGETTEXTURE);

    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "2");

    if ((*window == NULL) || (*renderer == NULL))
    {
        return FAILURE;
    }

    return SUCCESS;
}

void quitSDL(SDL_Window* window, SDL_Renderer* renderer)
{
    SDL_DestroyWindow(window);
    SDL_DestroyRenderer(renderer);

    IMG_Quit();
    Mix_Quit();
    TTF_Quit();
    SDL_Quit();
}