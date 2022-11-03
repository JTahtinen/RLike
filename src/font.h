#pragma once
#include <jadel.h>

struct FontInfo
{
    int lineHeight;
    int base;
    int scaleWidth;
    int scaleHeight;
    char file[40];
};

struct Letter
{
    unsigned int id;
    int x;
    int y;
    int width;
    int height;
    int xOffset;
    int yOffset;
    int xAdvance;
    int page;
    int channel;
};

struct Font
{
    FontInfo info;
    //TODO: Change to pointer after creating asset manager
    jadel::Surface fontAtlas;
    Letter* letters;
};

bool loadFont(const char* filepath, Font* font);