#include "font.h"
#include <jadel.h>
#include <string.h>
#include "render.h"

bool loadFont(const char *filepath, Font *font)
{
    char* data = NULL;
    size_t numChars;
    Letter *letters = NULL;

    if (!font || !jadel::readTextFileAndReserveMemory(filepath, &data, &numChars))
        return false;
    FontInfo *info = &font->info;
    // char *token = strtok(data, "=");
    char *token = NULL;
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
            info->base = atoi(strtok_s(data, "=\n ", &data));
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
        letters = (Letter *)jadel::memoryReserve(sizeof(Letter) * expectedLetterCount);
        while (numLetters < expectedLetterCount)
        {
            int letterElementsFound = 0;
            Letter *currentLetter = &letters[numLetters];
            while (letterElementsFound < 10)
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
                else if (strcmp(token, "page") == 0)
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
                }
            }
            ++numLetters;
        }
    }
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

    jadel::memoryFree(data);
    jadel::memoryFree(letters);
    return true;
}