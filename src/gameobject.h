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
    jadel::Point2i posInsideSquare; //negative 10 to 10 in both axes
    AnimFrames frames;
    bool alive;
    int health;
    int maxHealth;
};

GameObject createGameObject(int x, int y, AnimFrames frames, const char *name);

const jadel::Surface* getCurrentFrame(const GameObject* gameObject);

bool setFrame(int index, GameObject* gameObject);

void setNextFrame(GameObject* gameObject);