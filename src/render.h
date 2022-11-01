#pragma once
#include <jadel.h>
#include "game.h"

struct RectfRenderable
{
    jadel::Rectf rect;
    jadel::Color color;
};

struct RenderLayer
{
    jadel::Vector<const jadel::Surface*> surfaces;
    jadel::Vector<RectfRenderable> rects;
};

extern RenderLayer gameLayer;
extern RenderLayer uiLayer;

void pushRenderable(const jadel::Surface* renderable, RenderLayer* layer);
void pushRenderable(RectfRenderable renderable, RenderLayer* layer);
bool initRender(jadel::Window* window);
void render();
jadel::Surface* getScreenBuffer();


bool load_PNG(const char *filename, jadel::Surface *target);
