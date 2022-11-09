#include "gameobject.h"

GameObject createGameObject(int x, int y, AnimFrames frames, const char *name)
{
    GameObject result;
    result.entity = createEntity(x, y);
    result.posInsideSquare = {.x = 0, .y = 0};
    result.frames = frames;
    result.frames.currentFrameIndex = 0;
    result.maxHealth = 100;
    result.health = result.maxHealth;
    jadel::String::init(&result.entity.name, name);
    //strncpy(result.entity.name, name, sizeof(result.entity.name));
    result.alive = true;
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