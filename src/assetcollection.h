#pragma once
#include <jadel.h>
#include "font.h"

struct AssetCollection
{
    jadel::Vector<jadel::Surface> surfaces;
    jadel::Vector<jadel::String> surfaceNames;

    jadel::Vector<Font> fonts;
    jadel::Vector<jadel::String> fontNames;

    jadel::Surface *getSurface(const char *name);
    void pushSurface(jadel::Surface surface, const char *name);
    bool loadSurface(const char *filepath);

    Font *getFont(const char *name);
    void pushFont(const Font& font, const char *name);
    bool loadFont(const char *filepath);
};