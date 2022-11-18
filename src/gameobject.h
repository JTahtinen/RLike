#pragma once
#include "entity.h"

struct AnimFrames
{
    const jadel::Surface* sprites[10];
    int numFrames;
    int currentFrameIndex = 0;
};

struct GameObjectTemplate
{
    jadel::String name;
    AnimFrames frames;
    int maxHealth;
    bool affectedByLight;
};

struct GameObject
{
    Entity entity;
    jadel::Point2i posInsideSquare; //negative 10 to 10 in both axes
    AnimFrames frames;
    bool alive;
    int health;
    int maxHealth;
    bool affectedByLight;
    bool moved;
};

GameObject createGameObject(int x, int y, AnimFrames frames, const char *name);

GameObject createGameObject(int x, int y, const GameObjectTemplate* obj);

const jadel::Surface* getCurrentFrame(const GameObject* gameObject);

bool setFrame(int index, GameObject* gameObject);

void setNextFrame(GameObject* gameObject);

const jadel::String* getName(const GameObject* gameObject);