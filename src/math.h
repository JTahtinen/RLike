#pragma once
#include <jadel.h>

inline bool pointInRectf(jadel::Vec2 point, jadel::Vec2 start, jadel::Vec2 end)
{
    float xStart = start.x < end.x ? start.x : end.x;
    float yStart = start.y < end.y ? start.y : end.y;
    float xEnd = start.x < end.x ? end.x : start.x;
    float yEnd = start.y < end.y ? end.y : start.y;
    bool result = point.x >= xStart && point.x < xEnd && point.y >= yStart && point.y < yEnd;
    return result;
}
