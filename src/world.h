#pragma once
#include <jadel.h>
#include "actor.h"

struct Tile
{
    const jadel::Surface* surface;
    bool barrier;
    bool flammable;
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
    
    jadel::LinkedList<Item*> items;
    //Item* items[10];
    uint32 numItems;
    Portal* portal;
    jadel::Vec3 illumination;
    bool flammable;
    float ignitionTreshold;
    float temperature;
    bool onFire;
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
    jadel::Vector<Tile> tiles;
    jadel::LinkedList<GameObject*> gameObjects;
    jadel::LinkedList<Actor*> actors;
    jadel::LinkedList<Item*> items;
    jadel::LinkedList<Item*> lights;        
    Portal portals[10];
    uint32 numPortals;
    AStarNode* pathNodes;
    uint32 numCalculatedPathNodes;
    AStarNode** calculatedPathNodes;
};

bool inBounds(int x, int y, const World *world);

int distanceBetweenSectors(const Sector *a, const Sector *b);

void pushGameObject(GameObject gameObject, World *world);

void pushGameObject(int x, int y, AnimFrames frames, const char *name, World *world);

void pushItem(Item item, World *world);

void pushItem(Item* item, World *world);

void pushActor(Actor actor, World *world);

void pushActor(int x, int y, AnimFrames frames, const char *name, World *world);

void addSectorItem(Sector* sector, Item *item, World *world);

void addSectorItem(int x, int y, Item *item, World *world);

void removeSectorItem(Sector* sector, Item *item, World *world);

void removeSectorItem(int x, int y, Item *item, World *world);

Sector *getSectorFromPos(int x, int y, World *world);

Sector *getSectorFromPos(int x, int y);

Sector *getSectorFromPos(jadel::Point2i pos, World *world);

Sector *getSectorFromPos(jadel::Point2i pos);

jadel::Vector<Sector*> getSurroudingSectors(int x, int y, World* world);

void calculateLights(World* world);

bool setPortal(int x0, int y0, uint32 world0ID, int x1, int y1, uint32 world1ID);

bool initWorld(int width, int height, Sector* sectoMap, const jadel::Vector<Tile>& tiles, World *world);

bool initWorld(int width, int height, World *world);

bool loadWorld(const char *filepath, World *target);