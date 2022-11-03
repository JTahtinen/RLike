#pragma once
#include <jadel.h>
#include "screenobject.h"

struct RenderLayer
{
    std::vector<ScreenObject> screenObjects;
};

extern RenderLayer gameLayer;
extern RenderLayer uiLayer;


bool systemInitRender(jadel::Window* window);
void render();
jadel::Surface* getScreenBuffer();
void pushRenderable(ScreenObject scrObj, RenderLayer* layer);
void pushRenderable(const jadel::Surface* surface, jadel::Vec2 pos, jadel::Vec2 dimensions, RenderLayer* layer);
bool load_PNG(const char *filename, jadel::Surface *target);
