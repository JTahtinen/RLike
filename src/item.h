#pragma once
#include "gameobject.h"

struct Actor;

enum
{
    ITEM_EFFECT_ADJUST_HEALTH = 1,
    ITEM_EFFECT_ILLUMINATE = 1 << 1,
};

struct Item
{
    GameObject gameObject;
    uint32 flags;

    int healthModifier;

    float illumination;
    int distanceFromGround;
};

void useItem(Item *item, Actor *actor);

Item createItem(int x, int y, AnimFrames frames, const char *name, uint32 flags);

Item createHealthItem(int x, int y, AnimFrames frames, const char *name, int healthModifier);

Item createIlluminatorItem(int x, int y, AnimFrames frames, const char *name, float illumination);