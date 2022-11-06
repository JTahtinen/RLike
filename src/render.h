#pragma once
#include <jadel.h>
#include "screenobject.h"

struct RenderLayer
{
    std::vector<ScreenObject*> screenObjects;
};

extern RenderLayer gameLayer;
extern RenderLayer uiLayer;

struct Font;

bool systemInitRender(jadel::Window* window);
void render();
jadel::Surface* getScreenBuffer();
void submitRenderable(ScreenObject* scrObj, RenderLayer* layer);
void submitRenderable(const jadel::Surface* surface, jadel::Vec2 pos, jadel::Vec2 dimensions, RenderLayer* layer);
void submitRenderable(const jadel::Color color, jadel::Vec2 pos, jadel::Vec2 dimensions, RenderLayer* layer);
bool load_PNG(const char *filename, jadel::Surface *target);
void renderText(const char *text, jadel::Vec2 pos, float scale, const Font *font, ScreenObject* target);
void renderText(const char *text, jadel::Vec2 pos, float scale, const Font *font, RenderLayer* layer);
jadel::Vec2 getTextScreenSize(const char* text, float scale, const Font* font);
