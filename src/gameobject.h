#pragma once
#include "entity.h"

struct AnimFrames
{
    const jadel::Surface* sprites[10];
    int numFrames;
    int currentFrameIndex = 0;
};

struct GameObject
{
    Entity entity;
    AnimFrames frames;
    bool alive;
    int health;
    int maxHealth;
};

