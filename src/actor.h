#pragma once
#include "gameobject.h"
#include "inventory.h"

#define ATTRIB_MODIFIER(attrib) (attrib - 10 / 2)
struct Sector;

struct Transit
{
    bool inTransit;
    const Sector* startSector;
    const Sector* endSector;
    float progress;
};

struct Attributes
{
    uint32 strength;
    uint32 dexterity;
    uint32 constitution;
    uint32 intelligence;
    uint32 wisdom;
    uint32 charisma;
};

struct Actor
{
    GameObject gameObject;
    Inventory inventory;
    Attributes attrib;
    uint32 armorClass;
    Item* equippedWeapon;
    Transit transit;    
    uint32 commandInQueue;
    const Sector* path[100];
    uint32 pathLength;
    int pathStepsTaken;
    bool followingPath;

    void clearPath();
};

int getStrengthModifier(const Actor* actor);

int getArmorClassRating(const Actor* actor);

bool rollAttackHit(const Actor* attacker, const Actor* attackTarget);

int rollInitiative(Actor* actor);

int calculateMaxHealth(const Actor* actor);

const jadel::Surface* getCurrentFrame(const Actor* actor);

bool setFrame(int index, Actor* actor);

void setNextFrame(Actor* actor);

Actor createActor(int x, int y, AnimFrames frames, const char *name, Attributes *attrib);

Actor createActor(int x, int y, AnimFrames frames, const char *name);

const jadel::String* getName(const Actor* actor);