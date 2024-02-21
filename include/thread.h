#pragma once

typedef struct
{
    song_t* songs;
    int* nb_songs;
    int* id_song;

    SDL_Renderer* renderer;
} data_thread_t;