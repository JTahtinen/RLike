#pragma once
#include <jadel.h>

struct FontInfo
{
    int lineHeight;
    float base;
    int scaleWidth;
    int scaleHeight;
    char file[40];
};

struct Letter
{
    int id;
    float x;
    float y;
    float width;
    float height;
    float xOffset;
    float yOffset;
    float xAdvance;
};

struct Font
{
    FontInfo info;
    //TODO: Change to pointer after creating asset manager
    jadel::Surface fontAtlas;
    Letter* letters;

    bool loadFont(const char* filepath);
};

