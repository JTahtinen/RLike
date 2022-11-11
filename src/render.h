#pragma once
#include <jadel.h>
#include <vector>
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

bool submitText(jadel::Vec2 pos, float scale, const Font* font, ScreenObject* target, const char *content,  ...);
bool submitText(jadel::Vec2 pos, float scale, const Font* font, RenderLayer* layer, const char *content,  ...);

//bool submitText(const char *content, jadel::Vec2 pos, float scale, const Font* font, ScreenObject* target);
//bool submitText(const char *content, jadel::Vec2 pos, float scale, const Font* font, RenderLayer* layer);
//void renderText(const char *text, jadel::Vec2 pos, float scale, const Font *font, ScreenObject* target);
jadel::Vec2 getTextScreenSize(const char* text, float scale, const Font* font);
