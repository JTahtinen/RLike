#include "screenobject.h"

bool initScreenObject(ScreenObject* target)
{
    if (!target) return false;
    target->screenRects = (ScreenRect*)jadel::memoryReserve(MAX_SCREEN_RECTS_PER_OBJECT * sizeof(ScreenRect));
    target->screenSurfaces = (ScreenSurface*)jadel::memoryReserve(MAX_SCREEN_SURFACES_PER_OBJECT * sizeof(ScreenSurface));
    target->typeQueue = (uint32*)jadel::memoryReserve((MAX_SCREEN_RECTS_PER_OBJECT + MAX_SCREEN_SURFACES_PER_OBJECT) * sizeof(uint32));
    //target->screenRects.reserve(15);
    //target->screenSurfaces.reserve(15);
    //target->typeQueue.reserve(30);
    target->numElements = 0;
    target->numRects = 0;
    target->numSurfaces = 0;
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

ScreenSurface createScreenSurface(jadel::Vec2 pos, jadel::Vec2 dimensions, jadel::Rectf sourceRect, const jadel::Surface* surface)
{
    ScreenSurface result;
    result.pos = pos;
    result.dimensions = dimensions;
    result.surface = surface;
    result.sourceRect = sourceRect;
    return result;
}

ScreenSurface createScreenSurface(jadel::Vec2 pos, jadel::Vec2 dimensions, const jadel::Surface* surface)
{
    ScreenSurface result = createScreenSurface(pos, dimensions, jadel::Rectf{0, 0, 1.0f, 1.0f}, surface);
    return result;
}

void screenObjectSetPos(jadel::Vec2 pos, ScreenObject* target)
{
    if (!target) return;
    target->pos = pos;
}

void screenObjectFree(ScreenObject* target)
{
    if (!target) return;
    jadel::memoryFree(target->screenRects);
    jadel::memoryFree(target->screenSurfaces);
    jadel::memoryFree(target->typeQueue);
    target->screenRects = NULL;
    target->screenSurfaces = NULL;
    target->typeQueue = NULL;
    screenObjectClear(target);
}

void screenObjectClear(ScreenObject* target)
{
    if (!target) return;
    target->numElements = 0;
    target->numRects = 0;
    target->numSurfaces = 0;
}

void screenObjectPushScreenSurface(ScreenSurface surface, ScreenObject* target)
{
    if (!surface.surface) jadel::message("[WARNING] Tried to push null screen surface to screen object\n");
    //target->screenSurfaces.emplace_back(surface);
    //target->typeQueue.emplace_back(RENDERABLE_TYPE_SURFACE);
    if (target->numSurfaces >= MAX_SCREEN_SURFACES_PER_OBJECT)
    {
        jadel::message("[ERROR] Too many surfaces in screen object!\n");
        return;
    }
    target->screenSurfaces[target->numSurfaces++] = surface;
    target->typeQueue[target->numElements++] = RENDERABLE_TYPE_SURFACE;
}

void screenObjectPushScreenSurface(jadel::Vec2 pos, jadel::Vec2 dimensions, const jadel::Surface* surface, jadel::Rectf sourceRect, ScreenObject* target)
{
    ScreenSurface scrSurface = createScreenSurface(pos, dimensions, sourceRect, surface);
    screenObjectPushScreenSurface(scrSurface, target);
}

void screenObjectPushScreenSurface(jadel::Vec2 pos, jadel::Vec2 dimensions, const jadel::Surface* surface, ScreenObject* target)
{
    screenObjectPushScreenSurface(pos, dimensions, surface, jadel::Rectf{0, 0, 1.0f, 1.0f}, target);
}

void screenObjectPushRect(ScreenRect rect, ScreenObject* target)
{
    //target->screenRects.emplace_back(rect);
    //target->typeQueue.emplace_back(RENDERABLE_TYPE_RECT);
    if (target->numRects >= MAX_SCREEN_RECTS_PER_OBJECT)
    {
        jadel::message("[ERROR] Too many rects in screen object!\n");
        return;
    }
    target->screenRects[target->numRects++] = rect;
    target->typeQueue[target->numElements++] = RENDERABLE_TYPE_RECT;
}

void screenObjectPushRect(jadel::Vec2 pos, jadel::Vec2 dimensions, jadel::Color color, ScreenObject* target)
{
    ScreenRect rect = createScreenRect(pos, dimensions, color);
    screenObjectPushRect(rect, target);
}