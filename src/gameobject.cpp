#include "gameobject.h"

GameObject createGameObject(int x, int y, AnimFrames frames, const char *name)
{
    GameObject result;
    result.entity = createEntity(x, y);
    result.posInsideSquare = {.x = 0, .y = 0};
    result.frames = frames;
    result.maxHealth = 100;
    result.health = result.maxHealth;
    strncpy(result.entity.name, name, sizeof(result.entity.name));
    result.alive = true;
    return result;
}