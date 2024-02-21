#include "global.h"
#include "SDL_util.h"
#include "util.h"
#include "song.h"
#include "thread.h"

int lockControl = 0;
int endingThread = 0;
int del_song = 0;

int WINDOW_WIDTH  = 500;
int WINDOW_HEIGHT = 500;

int mainLoop(SDL_Window* window, SDL_Renderer* renderer)
{
    int quit = 0;

    SDL_Event event;
    SDL_Thread* thread = NULL;

    SDL_Texture* bg_texture = NULL;
    SDL_Rect bg_pos = {0};
    double bg_ratio;
    int bg_width, bg_height;

    song_t* songs = NULL; 
    int nb_songs;
    int id_song = 0;
    SDL_Rect song_img_pos;

    song_t* song_tmp = NULL;

    data_thread_t* data = NULL;

    char* path = NULL;

    bg_texture = IMG_LoadTexture(renderer, BACKGROUND_PATH);

    if (bg_texture == NULL)
    {
        printf("Failed to load background\n");
        return FAILURE;
    }

    SDL_QueryTexture(bg_texture, NULL, NULL, &bg_width, &bg_height);


    bg_ratio = (double)(bg_height) / bg_width;

    songs = malloc(MAX_SONGS * sizeof(song_t));

    if (initSongs(renderer, songs, &nb_songs) == FAILURE)
    {
        printf("Failed to get songs data\n");
        return FAILURE;
    }

    data = malloc(sizeof(data_thread_t));

    if (nb_songs)
    {
        song_tmp = cpySong(songs[0]);
    }

    while (!quit)
    {
        while (SDL_PollEvent(&event))
        {
            if (endingThread)
            {
                SDL_DetachThread(thread);
                endingThread = 0;
                lockControl = 0;
                SDL_PumpEvents();
                SDL_FlushEvents(SDL_FIRSTEVENT, SDL_LASTEVENT);
                if (del_song) 
                {
                    free_song(*song_tmp);
                    del_song = 0;
                }
                if (nb_songs)
                {
                    free(song_tmp);
                    song_tmp = cpySong(songs[id_song]);
                }
                else
                {
                    free(song_tmp);
                    song_tmp = NULL;
                }
                break;
            }
            else if (lockControl)
            {
                break;
            }
            if (event.type == SDL_QUIT || (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE))
            {
                quit = 1;
            }
            else if (event.type == SDL_KEYDOWN)
            {
                switch (event.key.keysym.sym)
                {
                    case (SDLK_RIGHT):
                        if (song_tmp == NULL)
                        {
                            break;
                        }
                        id_song++;
                        if (id_song >= nb_songs)
                        {
                            id_song -= nb_songs;
                        }

                        free(song_tmp);
                        song_tmp = cpySong(songs[id_song]);

                        if (Mix_PlayingMusic())
                        {
                            Mix_HaltMusic();
                        }

                        break;

                    case (SDLK_LEFT):
                        if (song_tmp == NULL)
                        {
                            break;
                        }
                        id_song--;
                        if (id_song < 0)
                        {
                            id_song += nb_songs;
                        }

                        free(song_tmp);
                        song_tmp = cpySong(songs[id_song]);

                        if (Mix_PlayingMusic())
                        {
                            Mix_HaltMusic();
                        }

                        break;

                    case (SDLK_SPACE):
                        if (song_tmp == NULL)
                        {
                            break;
                        }
                        if (songs[id_song].music == NULL)
                        {
                            break;
                        }
                        if (Mix_PlayingMusic())
                        {
                        if (Mix_PausedMusic())
                        {
                            Mix_ResumeMusic();
                        }
                        else
                        {
                            Mix_PauseMusic();
                        }
                        }
                        else 
                        {
                            Mix_PlayMusic(songs[id_song].music, -1);
                        }
                        break;

                    case (SDLK_d):
                        if (song_tmp == NULL)
                        {
                            break;
                        }
                        data->songs = songs;
                        data->nb_songs = &nb_songs;
                        data->id_song = &id_song;
                        data->renderer = renderer;

                        lockControl = 1;
                        thread = SDL_CreateThread(delSong, "thread_to_del", data);
                        break;
                    
                    case (SDLK_i):
                        data->songs = songs;
                        data->nb_songs = &nb_songs;
                        data->id_song = &id_song;
                        data->renderer = renderer;

                        lockControl = 1;
                        thread = SDL_CreateThread(importSong, "thread_to_import", data);
                        break;
                }
            }
        }

        SDL_GetWindowSize(window, &WINDOW_WIDTH, &WINDOW_HEIGHT);


        if (bg_ratio > 1)
        {
            bg_pos.h = WINDOW_HEIGHT;
            bg_pos.w = bg_pos.h * bg_ratio;

            if (bg_pos.w > WINDOW_WIDTH)
            {
                bg_pos.w = WINDOW_WIDTH;
                bg_pos.h = bg_pos.w / bg_ratio;
            }
        }
        else
        {
            bg_pos.w = WINDOW_WIDTH;
            bg_pos.h = bg_pos.w * bg_ratio;

            if (bg_pos.h > WINDOW_HEIGHT)
            {
                bg_pos.h = WINDOW_HEIGHT;
                bg_pos.w = bg_pos.h / bg_ratio;
            }
        }

        bg_pos.x = (WINDOW_WIDTH - bg_pos.w)/2;
        bg_pos.y = (WINDOW_HEIGHT - bg_pos.h)/2;

        song_img_pos.w = song_img_pos.h = (min(bg_pos.h, bg_pos.w))/2;
        
        song_img_pos.x = (bg_pos.w - song_img_pos.w)/2 + bg_pos.x;
        song_img_pos.y = (bg_pos.h - song_img_pos.h)/3 + bg_pos.y;
        
        if (song_tmp != NULL)
        {
            
            song_tmp->info_pos.h = bg_pos.h / 20.0;
            song_tmp->info_pos.w = song_tmp->info_pos.h * song_tmp->ratio_info;
            song_tmp->info_pos.x = (bg_pos.w - song_tmp->info_pos.w) / 2 + bg_pos.x;
            song_tmp->info_pos.y = (song_img_pos.y + song_img_pos.h) + 10;
        }


        SDL_RenderClear(renderer);
        
        SDL_RenderCopy(renderer, bg_texture, NULL, &bg_pos);

        if (song_tmp != NULL)
        {
            SDL_RenderCopy(renderer, song_tmp->img, NULL, &song_img_pos);
            SDL_RenderCopy(renderer, song_tmp->info_texture, NULL, &(song_tmp->info_pos));
        }

        SDL_RenderPresent(renderer);

    }

    free(song_tmp);
    free(data);
    free_songs(songs, nb_songs);
    SDL_DestroyTexture(bg_texture);

    return SUCCESS;
}

int main()
{
    SDL_Window* window;
    SDL_Renderer* renderer;
    
    if (initSDL(&window, &renderer) == FAILURE)
    {
        printf("Failure during the initialisation\n");
        return FAILURE;
    }

    mainLoop(window, renderer);

    quitSDL(window, renderer);
    
    return 0;
}