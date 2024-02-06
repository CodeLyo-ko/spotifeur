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

#define FAILURE 0
#define SUCCESS 1

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

int lockControl = 0;
int endingThread = 0;
int del_song = 0;

int WINDOW_WIDTH  = 500;
int WINDOW_HEIGHT = 500;

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

typedef struct
{
    song_t* songs;
    int* nb_songs;
    int* id_song;

    SDL_Renderer* renderer;
} data_song_t;

int str_isValid(char* str)
{
    int len = strlen(str);
    for (int i = 0; i < len; i++)
    {
        if ((str[i] >= 'a' && str[i] <= 'z') || (str[i] >= 'A' && str[i] <= 'Z') || (str[i] >= '0' && str[i] <= '9'))
        {
            return 1;
        }
    }

    return 0;
}

char* str_strip(char* src)
{
    char* dest = strdup(src);

    int len = strlen(dest);
    for (int i = 0; i < len; i++)
    {
        if (!((dest[i] >= 'a' && dest[i] <= 'z') || (dest[i] >= 'A' && dest[i] <= 'Z') || (dest[i] >= '0' && dest[i] <= '9')))
        {
            dest[i] = '_';
        }
    }

    return dest;
}

char* extension(char* path)
{
    char* ext;

    int len = strlen(path);
    int i = len - 1;
    
    while (i >= 0 && path[i] != '.')
    {
        i--;
    }
    
    if (i < 0)
    {
        ext = calloc(1, sizeof(char));
        return ext;
    }

    ext = calloc(len - i + 1, sizeof(char));
    int i_ext = 0;

    while (i < len)
    {
        ext[i_ext++] = path[i++];
    }

    return ext;
}

void free_song(song_t song)
{
    free(song.title);
    free(song.album);
    free(song.artist_concat);
    free(song.dir_name);
    for (int j = 0; j < song.nb_artist; j++)
    {
        free(song.artist[j]);
    }
    free(song.artist);
    SDL_DestroyTexture(song.img);
    SDL_DestroyTexture(song.info_texture);
    Mix_FreeMusic(song.music);
}

void free_songs(song_t* songs, int n)
{
    for (int i = 0; i < n; i++)
    {
        free_song(songs[i]);
    }
    free(songs);
}

song_t* cpySong(song_t song)
{
    song_t* cpy = malloc(sizeof(song_t));
    cpy->img = song.img;

    cpy->title = song.title;

    cpy->artist = song.artist;
    cpy->nb_artist = song.nb_artist;
    cpy->artist_concat = song.artist_concat;

    cpy->album = song.album;

    cpy->info_texture = song.info_texture;
    cpy->info_pos = song.info_pos;
    cpy->ratio_info = song.ratio_info;

    cpy->music = song.music;

    cpy->dir_name = song.dir_name;
    return cpy;
}

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

int dirExist(char* path)
{
    struct stat stats;
    stat(path, &stats);
    if (S_ISDIR(stats.st_mode))
    {
        return 1;
    }
    return 0;
}

int getSongInfo(char* song_path, char** title, char** artist, int* nb_artist, char** album)
{
    FILE* file = NULL;
    char* path = NULL;
    char* buffer = NULL;
    int i = 0;
    *nb_artist = 0;
    char c;

    path = calloc(strlen(song_path) + strlen("/") + strlen(SONG_INFO_NAME) + 1, sizeof(char));
    sprintf(path, "%s/%s", song_path, SONG_INFO_NAME);

    file = fopen(path, "r");

    free(path);

    if (file == NULL)
    {
        printf("Failed to open info file of song \"%s\"\n", song_path);
        return FAILURE;
    }

    buffer = calloc(MAX_STR, sizeof(char));

    c = fgetc(file);
    while (c != EOF)
    {
        i = 0;
        while (c != '=' && c != '\n' && c != EOF)
        {
            buffer[i++] = c;
            c = fgetc(file);
        }

        buffer[i] = '\0';
        

        if (!strcmp(buffer, "TITLE"))
        {
            c = fgetc(file);
            i = 0;
            while(c != '\n' && c != EOF)
            {
                buffer[i++] = c;
                c = fgetc(file);
            }
            buffer[i] = '\0';
            *title = strdup(buffer);
        }
        else if (!strcmp(buffer, "ALBUM"))
        {
            c = fgetc(file);
            i = 0;
            while(c != '\n' && c != EOF)
            {
                buffer[i++] = c;
                c = fgetc(file);
            } 
            buffer[i] = '\0';
            *album = strdup(buffer);
        }
        else if (!strcmp(buffer, "ARTIST"))
        {
            c = fgetc(file);
            while (c != '\n' && c != EOF)
            {
                i = 0;
                while (c != ';' && c != '\n' && c != EOF)
                {
                    buffer[i++] = c;
                    c = fgetc(file);
                }
                buffer[i] = '\0';
                artist[(*nb_artist)++] = strdup(buffer);
                if (c == ';')
                {
                    c = fgetc(file);
                }
            }
        }

        c = fgetc(file);
    }

    free(buffer);
    return SUCCESS;

}

int initSongs(SDL_Renderer* renderer, song_t* songs, int* nb_songs)
{
    char* song_file_dir = NULL;
    char* path_dir = NULL;
    char* song_dir = NULL;
    char* path_info = NULL;
    char* info_concat = NULL;

    SDL_Surface* text_surface = NULL;

    TTF_Font* font = TTF_OpenFont(FONT_NAME, FONT_SIZE);
    if (font == NULL)
    {
        printf("Failed to open font\n");
        return FAILURE;
    }

    SDL_Color color = {RED_VALUE_TXT, GREEN_VALUE_TXT, BLUE_VALUE_TXT};
    SDL_Color color_bg = {RED_VALUE_BG, GREEN_VALUE_BG, BLUE_VALUE_BG};
    song_file_dir = calloc(strlen(SONG_DIR) + strlen(SONG_LIST) + 1, sizeof(char));
    sprintf(song_file_dir, "%s%s", SONG_DIR, SONG_LIST);

    FILE* songs_file = fopen(song_file_dir, "r");

    free(song_file_dir);

    if (songs_file == NULL)
    {
        printf("Failed to open file of song list\n");
        TTF_CloseFont(font);
        return FAILURE;
    }

    *nb_songs = 0;

    song_dir = calloc(MAX_STR, sizeof(char));

    while(fgets(song_dir, MAX_STR, songs_file) != NULL)
    {
        if (song_dir[strlen(song_dir) - 1] == '\n')
        {
            song_dir[strlen(song_dir) - 1] = '\0';
        }

        free(path_dir);
        path_dir = calloc(strlen(SONG_DIR) + strlen(song_dir) + strlen("/") + 1, sizeof(char));
        sprintf(path_dir, "%s%s/", SONG_DIR, song_dir);

        if (!dirExist(path_dir))
        {
            printf("Directory missing for song \"%s\"\n", song_dir);
            continue;
        }

        songs[(*nb_songs)].dir_name = strdup(song_dir);

        free(path_info);
        path_info = calloc(strlen(path_dir) + strlen(SONG_IMG_NAME) + 1, sizeof(char));
        sprintf(path_info, "%s%s", path_dir, SONG_IMG_NAME);

        songs[(*nb_songs)].img = IMG_LoadTexture(renderer, path_info);

        if (songs[(*nb_songs)].img == NULL)
        {
            printf("Failed to load song \"%s\" image\n", song_dir);
            songs[(*nb_songs)].img = IMG_LoadTexture(renderer, SONG_IMG_DEFAULT);

            if (songs[(*nb_songs)].img == NULL)
            {
                printf("Failed to load song default image for \"%s\"\n", song_dir);
                free(path_dir);
                free(path_info);
                free(song_dir);
                free(info_concat);
                TTF_CloseFont(font);
                return FAILURE;
            }
        }

        songs[(*nb_songs)].title = NULL;
        songs[(*nb_songs)].artist = malloc(MAX_ARTIST * sizeof(char*));
        songs[(*nb_songs)].nb_artist = 0;
        songs[(*nb_songs)].album = NULL;

        if (getSongInfo(path_dir, &(songs[(*nb_songs)].title), songs[(*nb_songs)].artist, &(songs[(*nb_songs)].nb_artist), &(songs[(*nb_songs)].album)) == FAILURE)
        {
            //return FAILURE;
        }

        if (songs[(*nb_songs)].title == NULL)
        {
            songs[(*nb_songs)].title = strdup(SONG_TITLE_UNDEFINED);
        }
        if (songs[(*nb_songs)].album == NULL)
        {
            songs[(*nb_songs)].album = strdup(SONG_ALBUM_UNDEFINED);
        }
        if (songs[(*nb_songs)].nb_artist == 0)
        {
            songs[(*nb_songs)].artist_concat = strdup(SONG_ARTIST_UNDEFINED);
        }
        else 
        {
            int len = strlen(songs[(*nb_songs)].artist[0]);
            for (int i = 1; i < songs[(*nb_songs)].nb_artist; i++)
            {
                len += strlen(ARTIST_SEP);
                len += strlen(songs[(*nb_songs)].artist[i]);
            }

            songs[(*nb_songs)].artist_concat = calloc(len + 1, sizeof(char));
            strcpy(songs[(*nb_songs)].artist_concat, songs[(*nb_songs)].artist[0]);
            for (int i = 1; i < songs[(*nb_songs)].nb_artist; i++)
            {
                strcat(songs[(*nb_songs)].artist_concat, ARTIST_SEP);
                strcat(songs[(*nb_songs)].artist_concat, songs[(*nb_songs)].artist[i]);
            }
        }
        
        free(path_info);
        path_info = calloc(strlen(path_dir) + strlen(SONG_AUDIO_NAME) + 1, sizeof(char));
        sprintf(path_info, "%s%s", path_dir, SONG_AUDIO_NAME);
        songs[(*nb_songs)].music = Mix_LoadMUS(path_info);

        if (songs[(*nb_songs)].music == NULL)
        {
            free(path_info);
            path_info = calloc(strlen(path_dir) + strlen(SONG_AUDIO_NAME_WAV) + 1, sizeof(char));
            sprintf(path_info, "%s%s", path_dir, SONG_AUDIO_NAME_WAV);
            songs[(*nb_songs)].music = Mix_LoadMUS(path_info);

            if (songs[(*nb_songs)].music == NULL)
            {
                printf("Failed to load song \"%s\" audio\n", song_dir);
            }
        }

        free(info_concat);
        info_concat = calloc(strlen(songs[(*nb_songs)].title) + strlen(" - ") + strlen(songs[(*nb_songs)].artist_concat) + strlen(" - ") + strlen(songs[(*nb_songs)].album) + 1, sizeof(char));
        strcat(info_concat, songs[(*nb_songs)].title);
        strcat(info_concat, " - ");
        strcat(info_concat, songs[(*nb_songs)].artist_concat);
        strcat(info_concat, " - ");
        strcat(info_concat, songs[(*nb_songs)].album);

        text_surface = TTF_RenderText_Shaded(font, info_concat, color, color_bg);
        songs[(*nb_songs)].info_texture = SDL_CreateTextureFromSurface(renderer, text_surface);

        SDL_FreeSurface(text_surface);

        if (songs[(*nb_songs)].info_texture == NULL)
        {
            printf("Failed to load song \"%s\" info\n", song_dir);
            free(path_dir);
            free(path_info);
            free(song_dir);
            free(info_concat);
            TTF_CloseFont(font);
            return FAILURE;
        }

        TTF_SizeText(font, info_concat, &songs[(*nb_songs)].info_pos.w, &songs[(*nb_songs)].info_pos.h);
        songs[(*nb_songs)].ratio_info = (double)(songs[(*nb_songs)].info_pos.w) / songs[(*nb_songs)].info_pos.h;

        (*nb_songs)++;
    }
    
    TTF_CloseFont(font);

    free(path_dir);
    free(path_info);
    free(song_dir);
    free(info_concat);

    fclose(songs_file);

    return SUCCESS;
}

int importSong(void* data)
{
    char* line = NULL;
    char* tmp = NULL;

    char* audio_path = NULL;
    char* img_path = NULL;
    char** artist = NULL;

    char* info_concat = NULL;
    char* ext = NULL;

    song_t song;

    song.title = NULL;
    song.img = NULL;
    song.artist = NULL;
    song.nb_artist = 0;
    song.artist_concat = NULL;
    song.album = NULL;
    song.info_texture = NULL;
    song.ratio_info = 0;
    song.music = NULL;
    song.dir_name = NULL;

    data_song_t* songs_data = data;
    
    FILE* fp = NULL;

    //SELECT SONG FILE
    fp = popen("zenity --file-selection --title='Select the song you want to import' --file-filter='Music file (wav or mp3) | *.wav *.mp3'", "r");

    if (fp == NULL)
    {
        printf("Failure when using zenity during import\n");
        endingThread = 1;
        return FAILURE;
    }

    line = calloc(MAX_STR, sizeof(char));

    fgets(line, MAX_STR, fp);

    if (WEXITSTATUS(pclose(fp)))
    //Said no
    {
        endingThread = 1;

        free(line);

        return SUCCESS;
    }

    line[strlen(line) -  1] = '\0';

    song.music = Mix_LoadMUS(line);

    if (song.music == NULL)
    {
        fp = popen("zenity --error --text='Audio file not supported'", "r");


        if (fp == NULL)
        {
            printf("Failure when using zenity during import\n");
            endingThread = 1;

            free(line);

            return FAILURE;
        }

        pclose(fp);
        endingThread = 1;
        free(line);
        return SUCCESS;
    }

    audio_path = strdup(line);

    //ENTER TITLE
    choose_title:
    fp = popen("zenity --entry --title='Enter title' --text='Enter the title of the song (Required)'", "r");

    if (fp == NULL)
    {
        printf("Failure when using zenity during import\n");
        endingThread = 1;

        free(line);
        free(audio_path);
        
        free_song(song);

        return FAILURE;
    }

    fgets(line, MAX_STR, fp);

    if (WEXITSTATUS(pclose(fp)))
    //Said no
    {
        endingThread = 1;
        free(line);
        free(audio_path);

        free_song(song);

        return SUCCESS;
    }

    line[strlen(line) - 1] = '\0';

    if (!str_isValid(line))
    //no title entered
    {
        fp = popen("zenity --warning --title='WARNING' --text='No title entered or title invalid'", "r");
        pclose(fp);
        goto choose_title;
    }

    song.title = strdup(line);

    song.dir_name = str_strip(line);

    tmp = calloc(strlen(SONG_DIR) + strlen(song.dir_name) + 1, sizeof(char));
    sprintf(tmp, "%s%s", SONG_DIR, song.dir_name);

    if (dirExist(tmp))
    {
        fp = popen("zenity --warning --title='WARNING' --text='Already existing song, please enter a new title'", "r");
        pclose(fp);
        free(song.dir_name);
        free(song.title);
        free(tmp);
        goto choose_title;
    }

    free(tmp);

    //ENTER ALBUM
    fp = popen("zenity --entry --title='Enter album' --text='Enter the name of the album of the song'", "r");

    if (fp == NULL)
    {
        printf("Failure when using zenity during import\n");
        endingThread = 1;

        free(line);
        free(audio_path);
        
        free_song(song);
        
        return FAILURE;
    }

    fgets(line, MAX_STR, fp);

    if (WEXITSTATUS(pclose(fp)))
    //Said no
    {
        endingThread = 1;
        free(line);
        free(audio_path);
        
        free_song(song);

        return SUCCESS;
    }

    line[strlen(line) - 1] = '\0';

    if (str_isValid(line))
    {
        song.album = strdup(line);
    }
    else
    {
        song.album = strdup(SONG_ALBUM_UNDEFINED);
    }

    //ENTER ARTIST
    artist = malloc(MAX_ARTIST * sizeof(char*));
    song.nb_artist = 0;

    enter_artist:
    fp = popen("zenity --entry --title='Enter artist' --text='Enter the name of an artist of the song'", "r");

    if (fp == NULL)
    {
        printf("Failure when using zenity during import\n");
        endingThread = 1;

        free(line);
        free(audio_path);
       
       free_song(song);

        return FAILURE;
    }

    fgets(line, MAX_STR, fp);

    if (WEXITSTATUS(pclose(fp)))
    //Said no
    {
        endingThread = 1;
        free(line);
        free(audio_path);
        
        free_song(song);

        return SUCCESS;
    }

    line[strlen(line) - 1] = '\0';

    if (str_isValid(line))
    {
        artist[song.nb_artist++] = strdup(line);
        goto enter_artist;
    }

    song.artist = malloc(song.nb_artist * sizeof(char*));

    if (song.nb_artist == 0)
    {
        song.artist_concat = strdup(SONG_ARTIST_UNDEFINED);
    }
    else
    {
        int len = 0;
        for (int i = 0; i < song.nb_artist; i++)
        {
            len += strlen(artist[i]) + strlen(ARTIST_SEP);
        }

        song.artist_concat = calloc(len + 1, sizeof(char));

        song.artist[0] = strdup(artist[0]);
        strcpy(song.artist_concat, artist[0]);
        free(artist[0]);
    }
    
    for (int i = 1; i < song.nb_artist; i++)
    {
        song.artist[i] = strdup(artist[i]);
        strcat(song.artist_concat, ARTIST_SEP);
        strcat(song.artist_concat, artist[i]);

        free(artist[i]);
    }

    free(artist);

    //SELECT IMAGE
    fp = popen("zenity --file-selection --title='Select the image of the song' --file-filter='Image file (png or jpg) | *.png *.jpg'", "r");

    if (fp == NULL)
    {
        printf("Failure when using zenity during import\n");
        endingThread = 1;
        free(line);
        free(audio_path);
        
        free_song(song);

        return FAILURE;
    }

    fgets(line, MAX_STR, fp);

    if (WEXITSTATUS(pclose(fp)))
    //Said no
    {
        endingThread = 1;

        free(line);
        free(audio_path);
        
        free_song(song);

        return SUCCESS;
    }

    line[strlen(line) -  1] = '\0';

    img_path = strdup(line);

    song.img = IMG_LoadTexture(songs_data->renderer, img_path);
    if (song.img == NULL)
    {
        printf("Failed to load song image\n");
        song.img = IMG_LoadTexture(songs_data->renderer, SONG_IMG_DEFAULT);
        if (song.img == NULL)
        {
            printf("Failed to load default image\n");
            endingThread = 1;

            free(line);
            free(audio_path);
            free(img_path);
            
            free_song(song);
            
            return FAILURE;
        }
    }    

    //create info texture
    TTF_Font* font = TTF_OpenFont(FONT_NAME, FONT_SIZE);
    if (font == NULL)
    {
        printf("Failed to open font\n");
        endingThread = 1;

        free(line);
        free(audio_path);
        free(img_path);
        
        free_song(song);
        
        return FAILURE;
    }

    SDL_Color color = {RED_VALUE_TXT, GREEN_VALUE_TXT, BLUE_VALUE_TXT};
    SDL_Color color_bg = {RED_VALUE_BG, GREEN_VALUE_BG, BLUE_VALUE_BG};

    info_concat = calloc(strlen(song.title) + strlen(" - ") + strlen(song.artist_concat) + strlen(" - ")  + strlen(song.album) + 1, sizeof(char));
    strcat(info_concat, song.title);
    strcat(info_concat, " - ");
    strcat(info_concat, song.artist_concat);
    strcat(info_concat, " - ");
    strcat(info_concat, song.album);

    SDL_Surface* text_surface = TTF_RenderText_Shaded(font, info_concat, color, color_bg);
    song.info_texture = SDL_CreateTextureFromSurface(songs_data->renderer, text_surface);

    SDL_FreeSurface(text_surface);

    if (song.info_texture == NULL)
    {
        printf("Failed to load song info\n");
        endingThread = 1;

        free(line);
        free(audio_path);
        free(img_path);
        free(info_concat);
        
        free_song(song);
        
        return FAILURE;
    }

    TTF_SizeText(font, info_concat, &song.info_pos.w, &song.info_pos.h);
    song.ratio_info = (double)(song.info_pos.w) / song.info_pos.h;

    TTF_CloseFont(font);
    free(info_concat);

    //create directory for song
    sprintf(line, "mkdir %s%s", SONG_DIR, song.dir_name);
    system(line);

    sprintf(line, "cp '%s' '%s%s/%s'", img_path, SONG_DIR, song.dir_name, SONG_IMG_NAME);
    system(line);

    ext = extension(audio_path);
    if (!strcmp(ext, ".mp3"))
    {
        sprintf(line, "cp '%s' '%s%s/%s'", audio_path, SONG_DIR, song.dir_name, SONG_AUDIO_NAME);
    }
    else if (!strcmp(ext, ".wav"))
    {
        sprintf(line, "cp '%s' '%s%s/%s'", audio_path, SONG_DIR, song.dir_name, SONG_AUDIO_NAME_WAV);
    }
    system(line);
    free(ext);

    sprintf(line, "%s%s/%s", SONG_DIR, song.dir_name, SONG_INFO_NAME);
    fp = fopen(line, "w");

    sprintf(line, "TITLE=%s\n", song.title);
    fputs(line, fp);

    if (!strcmp(song.album, SONG_ALBUM_UNDEFINED))
    {
        sprintf(line, "ALBUM=%s\n", song.album);
        fputs(line, fp);
    }

    if (song.nb_artist > 0)
    {
        strcpy(line, "ARTIST=");
        strcat(line, song.artist[0]);
        for (int i = 1; i < song.nb_artist; i++)
        {
            strcat(line, ";");
            strcat(line, song.artist[i]);
        }
        fputs(line, fp);
    }

    fclose(fp);

    sprintf(line, "%s%s", SONG_DIR, SONG_LIST);
    fp = fopen(line, "a+");

    fputs(song.dir_name, fp);

    fclose(fp);

    songs_data->songs[(*songs_data->nb_songs)++] = song;

    endingThread = 1;
    free(line);
    free(audio_path);
    free(img_path);
    return SUCCESS;
}

int delSong(void* data)
{
    FILE* fp = NULL;

    fp = popen("zenity --question --window-icon='info' --title='WARNING' --text='Are you sure you want to delete that song?'", "r");

    if (fp == NULL)
    {
        printf("Unable to ask for confirmation during deletion\n");
        endingThread = 1;
        return FAILURE;
    }

    if (WEXITSTATUS(pclose(fp)))
    //Said no
    {
        endingThread = 1;
        return SUCCESS;
    }


    data_song_t* dat = data;

    song_t* songs = dat->songs;
    int* id_song = dat->id_song;
    int* nb_songs = dat->nb_songs;

    char* cmdline = NULL;
    char* line = NULL;

    cmdline = calloc(strlen("rm -r ") + strlen(SONG_DIR) + strlen(songs[*id_song].dir_name) + 1, sizeof(char));
    sprintf(cmdline, "rm -r %s%s", SONG_DIR, songs[*id_song].dir_name);
    system(cmdline);

    free(cmdline);
    cmdline = calloc(strlen("cp ") + strlen(SONG_DIR) + strlen(SONG_LIST) + strlen(" ") + strlen(SONG_DIR) + strlen("tmp") + 1, sizeof(char));
    sprintf(cmdline, "cp %s%s %stmp", SONG_DIR, SONG_LIST, SONG_DIR);
    system(cmdline);

    free(cmdline);
    cmdline = calloc(strlen(SONG_DIR) + strlen("tmp") + 1, sizeof(char));
    sprintf(cmdline, "%stmp", SONG_DIR);

    FILE* src = fopen(cmdline, "r");
    if (src == NULL)
    {
        printf("Error: failed to open tmp file\n");

        free(cmdline);

        return FAILURE;
    }

    free(cmdline);
    cmdline = calloc(strlen(SONG_DIR) + strlen(SONG_LIST) + 1, sizeof(char));
    sprintf(cmdline, "%s%s", SONG_DIR, SONG_LIST);

    FILE* dest = fopen(cmdline, "w");

    if (dest == NULL)
    {
        printf("Error: failed to open dest file\n");
        fclose(src);
        free(cmdline);
        return FAILURE;
    }

    line = calloc(MAX_STR, sizeof(char));

    while (fgets(line, MAX_STR, src) != NULL)
    {
        if (line[strlen(line) - 1] == '\n')
        {
            line[strlen(line) - 1] = '\0';
        }
        if (strcmp(line, songs[*id_song].dir_name))
        {
            fputs(line, dest);
            fputc('\n', dest);
        }
    }

    fclose(src); fclose(dest);
    free(line);

    free(cmdline);
    cmdline = calloc(strlen("rm ") + strlen(SONG_DIR) + strlen("tmp") + 1, sizeof(char));
    sprintf(cmdline, "rm %stmp", SONG_DIR);
    system(cmdline);

    free(cmdline);

    --(*nb_songs);

    for (int i = (*id_song); i < (*nb_songs); i++)
    {
        songs[i] = songs[i + 1];
    }

    if ((*id_song) >= (*nb_songs))
    {
        (*id_song) = 0;
    }

    endingThread = 1;
    del_song = 1;
    return SUCCESS;
}

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


    data_song_t* data = NULL;

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

    data = malloc(sizeof(data_song_t));

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

    SDL_DestroyWindow(window);
    SDL_DestroyRenderer(renderer);
    IMG_Quit();
    Mix_Quit();
    TTF_Quit();
    SDL_Quit();


    return 0;
}