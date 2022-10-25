#include "entity.h"
#include "game.h"

Entity createEntity(int x, int y)
{
    Entity result;
    result.pos.x = x;
    result.pos.y = y;
    result.id = numIDs++;
    return result;
}