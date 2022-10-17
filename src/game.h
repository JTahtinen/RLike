#pragma once
#include <jadel.h>
//#include "file.h"
#include "entity.h"
#include "inventory.h"
#include "gameobject.h"
#include "actor.h"

#define screenTilemapW (15)
#define screenTilemapH (10)
#define MAX_ACTORS (50)
#define MAX_GAMEOBJECTS (200)

extern float frameTime;

enum
{
    COMMAND_NULL = 0,
    COMMAND_MOVE_LEFT,
    COMMAND_MOVE_RIGHT,
    COMMAND_MOVE_UP,
    COMMAND_MOVE_DOWN,
    COMMAND_TOGGLE_INVENTORY,
    COMMAND_DROP_ITEM,
    COMMAND_USE_ITEM,
    COMMAND_TAKE_ITEM,
    COMMAND_LOOK,
    COMMAND_COUNT
};


struct ControlScheme
{
    uint32 keys[20];
    uint32 commands[20];
    uint32 numCommands;
};

struct Game;
struct Sector;

extern Game* currentGame;

enum
{
    ADJUST_HEALTH = 0,
    NUM_EFFECTS
};

enum
{
    SUBSTATE_GAME = 0,
    SUBSTATE_INVENTORY,
    SUBSTATE_COUNT
};


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
    uint32 numGameObjects = 0;
    GameObject* gameObjects[MAX_GAMEOBJECTS];
    uint32 numActors = 0;
    Actor* actors[MAX_ACTORS];
    uint32 numItems = 0;
    Item* items[MAX_GAMEOBJECTS];
    Portal portals[10];
    uint32 numPortals = 0;
    AStarNode* pathNodes;
    uint32 numCalculatedPathNodes = 0;
    AStarNode** calculatedPathNodes;
    jadel::Surface worldSurface;
};

struct Game
{
    Actor* actors;
    uint32 numActors;
    GameObject* gameObjects;
    uint32 numGameObjects;
    Item* items;
    uint32 numItems;
    
    uint32 commandQueue[10];
    uint32 numCommands = 0;
    World* worlds;
    uint32 numWorlds;
    World* currentWorld;
    uint32 currentState;
    jadel::Point2i screenPos = {.x = 0, .y = 0};
    Actor player;

    const Sector* path[100];
    uint32 pathLength = 0;
    
    int tileScreenW;
    int tileScreenH;
    Tile walkTile;
    Tile wallTile;
    int pSteps = 0;
    jadel::Window* window;

    bool updateGame = true;
    
    jadel::Surface playerSprite;
    jadel::Surface playerSpriteLeft;
    jadel::Surface walkSurface;
    jadel::Surface wallSurface;
    jadel::Surface clutterSprite;
    jadel::Surface poisonSprite;
    jadel::Surface hpackSprite;
    jadel::Surface portalSprite;


    jadel::Surface workingBuffer;
    
};

jadel::Recti getSectorScreenPos(int x, int y);
jadel::Recti getSectorScreenPos(jadel::Point2i pos);
jadel::Recti getSectorScreenPos(const Sector* sector);
void addSectorItem(Sector* sector, Item* item);
Sector* getSectorFromPos(int x, int y);
Sector* getSectorFromPos(jadel::Point2i pos);
Sector* getSectorOfEntity(Entity* entity);
Sector* getSectorOfGameObject(GameObject* gameObject);
Sector* getSectorOfActor(Actor* actor);


void setGame(Game* game);
bool initGame(jadel::Window* window);
void updateGame();
