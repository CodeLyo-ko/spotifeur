#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_ttf.h>

#define min(a, b) (a) < (b) ? (a) : (b)    

#define FAILURE 1
#define SUCCESS 0

#define MAX_STR 500
#define MAX_SONGS 999
#define MAX_ARTIST 10

#define FONT_NAME "data/font/font.ttf"
#define FONT_SIZE 50

#define RED_VALUE_BG 255
#define GREEN_VALUE_BG 255
#define BLUE_VALUE_BG 255

#define RED_VALUE_TXT 0
#define GREEN_VALUE_TXT 0
#define BLUE_VALUE_TXT 0

#define BACKGROUND_PATH "data/img/background.jpg"

#define SONG_DIR "data/songs/"
#define SONG_LIST "dir_list.dat"

#define SONG_IMG_NAME "img.jpg"
#define SONG_IMG_DEFAULT "data/img/default_song_img.jpg"

#define SONG_AUDIO_NAME "audio.mp3"
#define SONG_AUDIO_NAME_WAV "audio.wav"

#define SONG_INFO_NAME "info.dat"
#define SONG_TITLE_UNDEFINED "Title unknown"
#define SONG_ARTIST_UNDEFINED "Artist unknown"
#define SONG_ALBUM_UNDEFINED "Album unknown"

#define ARTIST_SEP ", "

extern int lockControl;
extern int endingThread;
extern int del_song;

extern int WINDOW_WIDTH;
extern int WINDOW_HEIGHT;