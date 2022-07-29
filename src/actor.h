#pragma once
#include "gameobject.h"
#include "inventory.h"

struct Sector;

struct Transit
{
    bool inTransit;
    const Sector* startSector;
    const Sector* endSector;
    float progress;
};

struct Actor
{
    GameObject gameObject;
    Inventory inventory;
    Transit transit;
};
