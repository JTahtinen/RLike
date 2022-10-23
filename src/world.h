#pragma once
#include <jadel.h>
#include "actor.h"

struct Tile
{
    const jadel::Surface* surface;
    bool barrier;
};

struct Portal
{
    const jadel::Surface* sprite;
    uint32 linkID;
    uint32 worldLinkID;
    Sector* sector;
};

struct Sector
{
    jadel::Point2i pos;
    const Tile* tile;
    Actor* occupant;
    Item* items[10];
    uint32 numItems;
    Portal* portal;
    float illumination;
};

struct AStarNode
{
    bool isCalculated;
    bool isClosed;
    jadel::Point2i pos;
    const Sector* sector;
    const AStarNode* sourceNode;
    int gCost;
    int hCost;
};

struct World
{
    Entity entity;
    int width;
    int height;
    Sector* sectors;
    jadel::Vector<GameObject*> gameObjects;
    jadel::Vector<Actor*> actors;
    jadel::Vector<Item*> items;
//    jadel::Vector<GameObject*> gameObjects;
//    jadel::Vector<Actor*> actors;
//    jadel::Vector<Item*> items;
    Portal portals[10];
    uint32 numPortals;
    AStarNode* pathNodes;
    uint32 numCalculatedPathNodes;
    AStarNode** calculatedPathNodes;
    jadel::Surface worldSurface;
};

bool inBounds(int x, int y, const World *world);

void pushGameObject(GameObject gameObject, World *world);

void pushGameObject(int x, int y, AnimFrames frames, const char *name, World *world);

void pushItem(Item item, World *world);

void pushActor(Actor actor, World *world);

void pushActor(int x, int y, AnimFrames frames, const char *name, World *world);

Sector *getSectorFromPos(int x, int y, World *world);

Sector *getSectorFromPos(int x, int y);

Sector *getSectorFromPos(jadel::Point2i pos, World *world);

Sector *getSectorFromPos(jadel::Point2i pos);

bool setPortal(int x0, int y0, uint32 world0ID, int x1, int y1, uint32 world1ID);

bool initWorld(int width, int height, World *world);