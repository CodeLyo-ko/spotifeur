#include "util.h"
#include "global.h"

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