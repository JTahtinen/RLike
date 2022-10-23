#pragma once
#include <jadel.h>
#include "entity.h"
#include "inventory.h"
#include "gameobject.h"
#include "world.h"
#include "actor.h"
#include "timer.h"

#define screenTilemapW (15)
#define screenTilemapH (10)
#define MAX_ACTORS (50)
#define MAX_GAMEOBJECTS (200)

extern float frameTime;

extern uint32 numIDs;
extern uint32 numPortalIDs;


enum
{
    COMMAND_NULL = 0,
    COMMAND_MOVE_LEFT,
    COMMAND_MOVE_RIGHT,
    COMMAND_MOVE_UP,
    COMMAND_MOVE_DOWN,
    COMMAND_MOVE_UP_LEFT,
    COMMAND_MOVE_UP_RIGHT,
    COMMAND_MOVE_DOWN_RIGHT,
    COMMAND_MOVE_DOWN_LEFT,
    COMMAND_TOGGLE_INVENTORY,
    COMMAND_DROP_ITEM,
    COMMAND_USE_ITEM,
    COMMAND_TAKE_ITEM,
    COMMAND_LOOK,
    COMMAND_COUNT
};


struct ControlScheme
{
    uint32 keyPress[20];
    uint32 keyPressCommands[20];
    uint32 numKeyPressCommands;
    uint32 keyType[20];
    uint32 keyTypeCommands[20];
    uint32 numKeyTypeCommands;
};

struct Game;
struct Sector;

extern Game* currentGame;

enum
{
    SUBSTATE_GAME = 0,
    SUBSTATE_INVENTORY,
    SUBSTATE_COUNT
};

struct Game
{
    Actor* actors;
    uint32 numActors = 0;
    GameObject* gameObjects;
    uint32 numGameObjects = 0;
    Item* items;
    uint32 numItems = 0;
    
    uint32 commandQueue[10];
    uint32 numCommands = 0;
    World* worlds;
    uint32 numWorlds = 0;
    World* currentWorld;
    uint32 currentState;
    jadel::Point2i screenPos = {.x = 0, .y = 0};
    Actor player;
    
    int tileScreenW;
    int tileScreenH;
    Tile walkTile;
    Tile wallTile;
    jadel::Window* window;

    Timer moveTimer;
    bool playerCanMove;
    size_t moveTimerMillis;
    bool updateGame = true;
    
    jadel::Surface playerSprite;
    jadel::Surface playerSpriteLeft;
    jadel::Surface walkSurface;
    jadel::Surface wallSurface;
    jadel::Surface clutterSprite;
    jadel::Surface poisonSprite;
    jadel::Surface hpackSprite;
    jadel::Surface portalSprite;    
};

jadel::Recti getSectorScreenPos(int x, int y);
jadel::Recti getSectorScreenPos(jadel::Point2i pos);
jadel::Recti getSectorScreenPos(const Sector* sector);
void addSectorItem(Sector* sector, Item* item);
void initSector(int x, int y, const Tile *tile, Sector *target);
Sector* getSectorFromPos(int x, int y);
Sector* getSectorFromPos(jadel::Point2i pos);
Sector* getSectorOfEntity(Entity* entity);
Sector* getSectorOfGameObject(GameObject* gameObject);
Sector* getSectorOfActor(Actor* actor);
World *getWorldByID(uint32 ID);

Entity createEntity(int x, int y);

GameObject createGameObject(int x, int y, AnimFrames frames, const char *name);

Item createItem(int x, int y, AnimFrames frames, const char *name, uint32 flags);

Item createHealthItem(int x, int y, AnimFrames frames, const char *name, int healthModifier);

Item createIlluminatorItem(int x, int y, AnimFrames frames, const char *name, float illumination);


void setGame(Game* game);
bool initGame(jadel::Window* window);
void updateGame();
