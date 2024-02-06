## How to use
To compile, call "gcc spotifeur.c -o spotifeur -lSDL2 -lSDL2_image -lSDL2_ttf -lSDL2_mixer"

Just launch spotifeur

SDL2/image/mixer/ttf and zenity must be installed

If you want a log of the errors, you must redirect the output file when calling ./spotifeur (the errors are given with printf)

Sometimes it bugs, don't worry, just reopen the app lol

## Controls:
- `esc` to quit
- `right` and `left` arrows to navigate between songs
- `space` to play and pause a song
- `d` to delete current song
- `i` to import a song

## Importing a song:
Only .wav et .mp3 files are supported for now

.wav 24bits CAN'T be played 

An audio file, a title, and an image are required

Album and artists are facultative

If you want to enter multiple artists names, you have to enter one name, hit "ok" (or enter), enter a new name, etc. When you finish, just hit enter without any name.

## Changing information
To change the background of the app, go in `data/img`, put the image you want here and rename it `background.jpg`
Any other name will NOT work.


To change the font, go in `data/font` and place your font here. Rename it `font.ttf`
Any other name will NOT work.


To change an information of a song, go in its directory in `songs/name_of_song`

Song file must be named `audio.mp3` or `audio.wav`, depending on its extension

Image file must be named `img.jpg`

To change the title, album, artists, go in the file `info.dat`

`info.dat` is structured like this:

`TITLE=title of your song`

`ALBUM=name of the album`

`ARTIST=name;of;the;artists`

the lines can be in any order, and every lines aren't required


if you change the name of the directory of a song, you must update it in the file `songs/dir_list.dat`
