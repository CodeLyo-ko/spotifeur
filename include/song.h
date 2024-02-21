#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_ttf.h>

typedef struct 
{
    SDL_Texture* img;

    char* title;

    char** artist;
    int nb_artist;
    char* artist_concat;

    char* album;

    SDL_Texture* info_texture;
    SDL_Rect info_pos;
    double ratio_info;

    Mix_Music* music;

    char* dir_name;
} song_t;

/**
 * Free a song and all its data 
 */
void free_song(song_t song);

/**
 * Free an array of songs, and all the songs in it
 */
void free_songs(song_t* songs, int n);

/**
 * Return a copy of the song given in input
 */
song_t* cpySong(song_t song);

/**
 * Get the title, artists, album of a song from its info file
 */
int getSongInfo(char* song_path, char** title, char** artist, int* nb_artist, char** album);

/**
 * Initalize the array of songs
 */
int initSongs(SDL_Renderer* renderer, song_t* songs, int* nb_songs);

/**
 * Import a song
 */
int importSong(void* data);

/**
 * Delete a song
 */
int delSong(void* data);