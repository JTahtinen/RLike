#pragma once
#include "gameobject.h"
#include <jadel.h>

struct Actor;

enum
{
    ITEM_EFFECT_ADJUST_HEALTH = 1,
    ITEM_EFFECT_ILLUMINATE = 1 << 1,
    ITEM_EFFECT_WEAPON = 1 << 2,
    ITEM_EFFECT_CONSUMABLE = 1 << 3,
    ITEM_EFFECT_EQUIPPABLE = 1 << 4
};

struct DamageRange
{
    uint32 numDice;
    uint32 numFaces;
};

struct Item
{
    GameObject gameObject;
    uint32 flags;

    int healthModifier;

    jadel::Vec3 illumination;
    int distanceFromGround;

    DamageRange damage;
};

uint32 calculateDamage(DamageRange damageRange);

void useItem(Item *item, Actor *actor);

Item createItem(int x, int y, AnimFrames frames, const char *name, uint32 flags);

Item createHealthItem(int x, int y, AnimFrames frames, const char *name, int healthModifier);

Item createHealthItem(int x, int y, const GameObjectTemplate* obj, int healthModifier);

Item createIlluminatorItem(int x, int y, AnimFrames frames, const char *name, jadel::Vec3 illumination);

Item createWeaponItem(int x, int y, AnimFrames frames, const char* name, uint32 numDice, uint32 numFaces);