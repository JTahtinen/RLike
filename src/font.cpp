#include "font.h"
#include <jadel.h>
#include <string.h>
#include "render.h"
#include "game.h"

struct RawLetter
{
    int id;
    int x;
    int y;
    int width;
    int height;
    int xOffset;
    int yOffset;
    int xAdvance;
};



bool loadFont(const char *filepath, Font *font)
{
    char* data = NULL;
    char *token = NULL;
    size_t numChars;
    RawLetter *letters = NULL;

    if (!font || !jadel::readTextFileAndReserveMemory(filepath, &data, &numChars))
        return false;
    FontInfo *info = &font->info;
    // char *token = strtok(data, "=");
    bool startfound = false;
    int fontElementsFound = 0;
    while (fontElementsFound < 5)
    {
        token = strtok_s(data, "=\n ", &data);

        if (strcmp(token, "lineHeight") == 0)
        {
            info->lineHeight = atoi(strtok_s(data, "=\n ", &data));
            ++fontElementsFound;
        }
        else if (strcmp(token, "base") == 0)
        {
            info->base = atof(strtok_s(data, "=\n ", &data));
            ++fontElementsFound;
        }
        else if (strcmp(token, "scaleW") == 0)
        {
            info->scaleWidth = atoi(strtok_s(data, "=\n ", &data));
            ++fontElementsFound;
        }
        else if (strcmp(token, "scaleH") == 0)
        {
            info->scaleHeight = atoi(strtok_s(data, "=\n ", &data));
            ++fontElementsFound;
        }
        else if (strcmp(token, "file") == 0)
        {
            strncpy_s(info->file, strtok_s(data, "=\"\n ", &data), 40);
            ++fontElementsFound;
        }
    }

    if (fontElementsFound < 5)
    {
        return false;
    }

    while (!startfound)
    {
        startfound = (strncmp(token, "chars count", sizeof("chars count")) == 0);
        token = strtok_s(data, "=\n", &data);
        if (!token)
            break;
    }
    int numLetters = 0;

    int expectedLetterCount = 0;
    if (startfound)
    {
        expectedLetterCount = atoi(token);
        letters = (RawLetter *)jadel::memoryReserve(sizeof(RawLetter) * expectedLetterCount);
        while (numLetters < expectedLetterCount)
        {
            int letterElementsFound = 0;
            RawLetter *currentLetter = &letters[numLetters];
            while (letterElementsFound < 8)
            {
                token = strtok_s(data, "=\n ", &data);
                if (strcmp(token, "id") == 0)
                {
                    token = strtok_s(data, "=\n ", &data);
                    currentLetter->id = atoi(token);
                    ++letterElementsFound;
                }
                else if (strcmp(token, "x") == 0)
                {
                    token = strtok_s(data, "=\n ", &data);
                    currentLetter->x = atoi(token);
                    ++letterElementsFound;
                }
                else if (strcmp(token, "y") == 0)
                {
                    token = strtok_s(data, "=\n ", &data);
                    currentLetter->y = atoi(token);
                    ++letterElementsFound;
                }
                else if (strcmp(token, "width") == 0)
                {
                    token = strtok_s(data, "=\n ", &data);
                    currentLetter->width = atoi(token);
                    ++letterElementsFound;
                }
                else if (strcmp(token, "height") == 0)
                {
                    token = strtok_s(data, "=\n ", &data);
                    currentLetter->height = atoi(token);
                    ++letterElementsFound;
                }
                else if (strcmp(token, "xoffset") == 0)
                {
                    token = strtok_s(data, "=\n ", &data);
                    currentLetter->xOffset = atoi(token);
                    ++letterElementsFound;
                }
                else if (strcmp(token, "yoffset") == 0)
                {
                    token = strtok_s(data, "=\n ", &data);
                    currentLetter->yOffset = atoi(token);
                    ++letterElementsFound;
                }
                else if (strcmp(token, "xadvance") == 0)
                {
                    token = strtok_s(data, "=\n ", &data);
                    currentLetter->xAdvance = atoi(token);
                    ++letterElementsFound;
                }
  /*              else if (strcmp(token, "page") == 0)
                {
                    token = strtok_s(data, "=\n ", &data);
                    currentLetter->page = atoi(token);
                    ++letterElementsFound;
                }
                else if (strcmp(token, "chnl") == 0)
                {
                    token = strtok_s(data, "=\n ", &data);
                    currentLetter->channel = atoi(token);
                    ++letterElementsFound;
                }*/
            }
            ++numLetters;
        }
    }
    /*for (int i = 0; i < numLetters; ++i)
    {
        RawLetter* l = &letters[i];
        jadel::message("Letter[%d]\nid: %d\nX: %d\nY: %d\nWidth: %d\nHeight: %d\nxOffset: %d\nyOffset: %d\nxAdvance: %d\n\n", 
        i,l->id,l->x,l->y,l->width,l->height,l->xOffset,l->yOffset,l->xAdvance);

    }*/
    if (expectedLetterCount != numLetters)
    {
        jadel::message("[ERROR] \"%s\" Font load failed\nLetters expected: %d\nLetters parsed: %d\n", expectedLetterCount, numLetters);
        return false;
    }
    // TODO: Remove hacky png loading and load file through asset manager
    const char* fontFolder = "res/fonts/";
    size_t fontFolderSize = strlen(fontFolder);
    char fontPNGPath[40] = {0};
    strcpy(fontPNGPath, fontFolder);
    strcpy(&fontPNGPath[fontFolderSize], &info->file[0]);
    if (!load_PNG(fontPNGPath, &font->fontAtlas))
    {
        return false;
    }
    Letter* modifiedLetters = (Letter*)jadel::memoryReserve(sizeof(Letter) * 256);
    for (int i = 0; i < numLetters; ++i)
    {
        RawLetter* rawLetter = &letters[i];
        Letter* modLetter = &modifiedLetters[rawLetter->id];
        //Letter* modLetter = &modifiedLetters[i];
        modLetter->id = rawLetter->id;
        modLetter->x = (float)rawLetter->x / (float)font->fontAtlas.width;
        modLetter->y = (float)rawLetter->y / (float)font->fontAtlas.height;
        modLetter->width = (float)rawLetter->width / (float)font->fontAtlas.width;
        modLetter->height = (float)rawLetter->height / (float)font->fontAtlas.height;
        modLetter->xOffset = (float)rawLetter->xOffset / (float)font->fontAtlas.width;
        modLetter->yOffset = (float)rawLetter->yOffset / (float)font->fontAtlas.height;
        modLetter->xAdvance = (float)rawLetter->xAdvance / (float)font->fontAtlas.width;
    }   
    font->letters = modifiedLetters;
    font->info.base /= (float)font->fontAtlas.height;   

    jadel::memoryFree(data);
    jadel::memoryFree(letters);
    return true;
}