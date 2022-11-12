#include "gameobject.h"



GameObject createGameObject(int x, int y, AnimFrames frames, const char *name,  int maxHealth, bool affectedByLight)
{
    GameObject result;
    result.entity = createEntity(x, y);
    result.posInsideSquare = {.x = 0, .y = 0};
    result.frames = frames;
    result.frames.currentFrameIndex = 0;
    result.maxHealth = maxHealth;
    result.health = result.maxHealth;
    result.entity.name = name;
    result.alive = true;
    result.affectedByLight = true;
    return result;
}

GameObject createGameObject(int x, int y, AnimFrames frames, const char *name)
{
    GameObject result = createGameObject(x, y, frames, name, 100, true);
    return result;
}

GameObject createGameObject(int x, int y, const GameObjectTemplate* obj)
{
    GameObject result = createGameObject(x, y, obj->frames, obj->name.c_str(), obj->maxHealth, obj->affectedByLight);
    return result;
}

const jadel::Surface* getCurrentFrame(const GameObject* gameObject)
{
    if (!gameObject) return NULL;
    const jadel::Surface* result = gameObject->frames.sprites[gameObject->frames.currentFrameIndex];
    return result;
}

bool setFrame(int index, GameObject* gameObject)
{
    if (!gameObject || index >= gameObject->frames.numFrames) return false;
    gameObject->frames.currentFrameIndex = index;
    return true;
}

void setNextFrame(GameObject* gameObject)
{
    if (!gameObject) return;
    AnimFrames* frames = &gameObject->frames;
    ++frames->currentFrameIndex;
    if (frames->currentFrameIndex >= frames->numFrames)
    {
        frames->currentFrameIndex = 0;
    }
}

const jadel::String* getName(const GameObject* gameObject)
{
    if (!gameObject) return NULL;
    const jadel::String* result = &gameObject->entity.name;
    return result;
}