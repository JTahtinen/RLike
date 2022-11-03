#include "screenobject.h"

bool initScreenObject(jadel::Vec2 pos, ScreenObject* target)
{
    if (!target) return false;
    target->pos = pos;
    target->screenRects.reserve(15);
    target->screenSurfaces.reserve(15);
    target->typeQueue.reserve(30);
    return true;
}

ScreenRect createScreenRect(jadel::Vec2 pos, jadel::Vec2 dimensions, jadel::Color color)
{
    ScreenRect result;
    result.pos = pos;
    result.dimensions = dimensions;
    result.color = color;
    return result;
}

ScreenSurface createScreenSurface(jadel::Vec2 pos, jadel::Vec2 dimensions, const jadel::Surface* surface)
{
    ScreenSurface result;
    result.pos = pos;
    result.dimensions = dimensions;
    result.surface = surface;
    return result;
}

void screenObjectPushRect(ScreenRect rect, ScreenObject* target)
{
    target->screenRects.emplace_back(rect);
    target->typeQueue.emplace_back(RENDERABLE_TYPE_RECT);
}

void screenObjectPushScreenSurface(ScreenSurface surface, ScreenObject* target)
{
    target->screenSurfaces.emplace_back(surface);
    target->typeQueue.emplace_back(RENDERABLE_TYPE_SURFACE);
}

void screenObjectPushRect(jadel::Vec2 pos, jadel::Vec2 dimensions, jadel::Color color, ScreenObject* target)
{
    ScreenRect rect = {pos, dimensions, color};
    screenObjectPushRect(rect, target);
}

void screenObjectPushScreenSurface(jadel::Vec2 pos, jadel::Vec2 dimensions, const jadel::Surface* surface, ScreenObject* target)
{
    ScreenSurface scrSurface = {pos, dimensions, surface};
    screenObjectPushScreenSurface(scrSurface, target);
}

void screenObjectClear(ScreenObject* screenObject)
{
    screenObject->screenRects.clear();
    screenObject->screenSurfaces.clear();
    screenObject->typeQueue.clear();
}