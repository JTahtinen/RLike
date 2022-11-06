#pragma once
#include <vector>
#include <jadel.h>

#define MAX_SCREEN_SURFACES_PER_OBJECT (500)
#define MAX_SCREEN_RECTS_PER_OBJECT (500)

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
    jadel::Rectf sourceRect;
};

struct ScreenObject
{
    jadel::Vec2 pos;
    /*std::vector<ScreenRect> screenRects;
    std::vector<ScreenSurface> screenSurfaces;
    std::vector<uint32> typeQueue;*/
    ScreenRect screenRects[MAX_SCREEN_RECTS_PER_OBJECT];
    ScreenSurface screenSurfaces[MAX_SCREEN_SURFACES_PER_OBJECT];
    uint32 typeQueue[MAX_SCREEN_RECTS_PER_OBJECT + MAX_SCREEN_SURFACES_PER_OBJECT];
    size_t numElements;
    size_t numRects;
    size_t numSurfaces;
};

bool initScreenObject(jadel::Vec2 pos, ScreenObject* target);

ScreenRect createScreenRect(jadel::Vec2 pos, jadel::Vec2 dimensions, jadel::Color color);

ScreenSurface createScreenSurface(jadel::Vec2 pos, jadel::Vec2 dimensions, jadel::Rectf sourceRect, const jadel::Surface* surface);

ScreenSurface createScreenSurface(jadel::Vec2 pos, jadel::Vec2 dimensions, const jadel::Surface* surface);

void screenObjectPushRect(ScreenRect rect, ScreenObject* target);

void screenObjectPushScreenSurface(ScreenSurface surface, ScreenObject* target);

void screenObjectPushRect(jadel::Vec2 pos, jadel::Vec2 dimensions, jadel::Color color, ScreenObject* target);

void screenObjectPushScreenSurface(jadel::Vec2 pos, jadel::Vec2 dimensions, const jadel::Surface* surface, jadel::Rectf sourceRect, ScreenObject* target);

void screenObjectPushScreenSurface(jadel::Vec2 pos, jadel::Vec2 dimensions, const jadel::Surface* surface, ScreenObject* target);

void screenObjectClear(ScreenObject* screenObject);