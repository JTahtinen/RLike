#pragma once
#include <vector>
#include <jadel.h>

enum RenderableType
{
    RENDERABLE_TYPE_SURFACE = 0,
    RENDERABLE_TYPE_RECT,
    NUM_RENDERABLE_TYPES
};

struct ScreenRect
{
    jadel::Vec2 pos;
    jadel::Vec2 dimensions;
    jadel::Color color;
};

struct ScreenSurface
{
    jadel::Vec2 pos;
    jadel::Vec2 dimensions;
    const jadel::Surface* surface;
};

struct ScreenObject
{
    jadel::Vec2 pos;
    std::vector<ScreenRect> screenRects;
    std::vector<ScreenSurface> screenSurfaces;
    std::vector<uint32> typeQueue;
};

bool initScreenObject(jadel::Vec2 pos, ScreenObject* target);

ScreenRect createScreenRect(jadel::Vec2 pos, jadel::Vec2 dimensions, jadel::Color color);

ScreenSurface createScreenSurface(jadel::Vec2 pos, jadel::Vec2 dimensions, const jadel::Surface* surface);

void screenObjectPushRect(ScreenRect rect, ScreenObject* target);

void screenObjectPushScreenSurface(ScreenSurface surface, ScreenObject* target);

void screenObjectPushRect(jadel::Vec2 pos, jadel::Vec2 dimensions, jadel::Color color, ScreenObject* target);

void screenObjectPushScreenSurface(jadel::Vec2 pos, jadel::Vec2 dimensions, const jadel::Surface* surface, ScreenObject* target);

void screenObjectClear(ScreenObject* screenObject);