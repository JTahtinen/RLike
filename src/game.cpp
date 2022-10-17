#include "game.h"
#include <jadel.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include "file.h"
#include "inventory.h"

Game* currentGame;
static uint32 numGUIDs = 0;
static uint32 numPortalIDs = 0;

static ControlScheme gameCommands;

float frameTime = 0;

bool inBounds(int x, int y, const World* world)
{
    bool result (x >= 0 && y >= 0 &&
                 x < world->width && y < world->height);
    return result;
}

World* getWorldByID(uint32 ID)
{
    for (int i = 0; i < currentGame->numWorlds; ++i)
    {
        World* world = &currentGame->worlds[i];
        if (world->entity.GUID == ID)
        {
            return world;
        }
    }
    return NULL;
}

bool setWorldByID(uint32 ID)
{
    World* world = getWorldByID(ID);
    if (world)
    {
        currentGame->currentWorld = world;
        return true;
    }
    return false;
}

Actor** getActors()
{
    return currentGame->currentWorld->actors;
}

void setGame(Game* game)
{
    currentGame = game;
}

Entity createEntity(int x, int y)
{
    Entity result;
    result.pos.x = x;
    result.pos.y = y;
    result.GUID = numGUIDs++;
    return result;
}

GameObject createGameObject(int x, int y, AnimFrames frames, const char* name)
{
    GameObject result;
    result.entity = createEntity(x, y);
    result.frames = frames;
    result.maxHealth = 100;
    result.health = result.maxHealth;
    strncpy(result.entity.name, name, sizeof(result.entity.name));    
    result.alive = true;
    return result;
}

void pushGameObject(GameObject gameObject, World* world)
{
    if(currentGame->numGameObjects < MAX_GAMEOBJECTS)
    {
        uint32 index = currentGame->numGameObjects++;
        currentGame->gameObjects[index] = gameObject;
        
        if (world->numGameObjects < MAX_GAMEOBJECTS)
        {
            world->gameObjects[world->numGameObjects++] = &currentGame->gameObjects[index];
        }
    }
}

void pushGameObject(int x, int y, AnimFrames frames, const char* name, World* world)
{
    pushGameObject(createGameObject(x, y, frames, name), world);
}

Item createItem(int x, int y, AnimFrames frames, const char* name, uint32 effect, int value)
{
    Item result;
    result.gameObject = createGameObject(x, y, frames, name);
    result.effect = effect;
    result.value = value;
    return result;
}


void pushItem(Item item, World* world)
{
    if(currentGame->numItems < MAX_GAMEOBJECTS)
    {
        uint32 index = currentGame->numItems++;
        currentGame->items[index] = item;
        
        if (world->numItems < MAX_GAMEOBJECTS)
        {
            world->items[world->numItems++] = &currentGame->items[index];
        }
    }
}


void pushItem(int x, int y, AnimFrames frames, const char* name, uint32 effect, int value, World* world)
{
    pushItem(createItem(x, y, frames, name, effect, value), world);
}

Actor createActor(int x, int y, AnimFrames frames, const char* name)
{
    Actor result;
    result.gameObject = createGameObject(x, y, frames, name);
    result.transit.inTransit = false;
    result.transit.startSector = NULL;
    result.transit.endSector = NULL;
    result.transit.progress = 0;    
    for (int i = 0; i < 10; ++i)
    {
        result.inventory.itemSlots[i].hasItem = false;
    }
    return result;
}

void pushActor(Actor actor, World* world)
{
    if(currentGame->numActors < MAX_ACTORS)
    {
        uint32 index = currentGame->numActors++;
        currentGame->actors[index] = actor;
        
        if (world->numItems < MAX_ACTORS)
        {
            world->actors[world->numActors++] = &currentGame->actors[index];
        }
    }
}

void pushActor(int x, int y, AnimFrames frames, const char* name, World* world)
{
    pushActor(createActor(x, y, frames, name), world);
}


jadel::Recti getSectorScreenPos(int x, int y)
{
    jadel::Recti result = {.x = (x - currentGame->screenPos.x) * currentGame->tileScreenW,
                 .y = (y - currentGame->screenPos.y) * currentGame->tileScreenH,
                 .w = currentGame->tileScreenW,
                 .h = currentGame->tileScreenH};
    return result;
}

jadel::Recti getSectorScreenPos(jadel::Point2i pos)
{
    jadel::Recti result = getSectorScreenPos(pos.x, pos.y);
    return result;
}

jadel::Recti getSectorScreenPos(const Sector* sector)
{
    jadel::Point2i pos = sector->pos;
    jadel::Recti result = getSectorScreenPos(pos.x, pos.y);
    return result;
}

Sector* getSectorFromPos(int x, int y, World* world)
{
    Sector* result = NULL;
    if (inBounds(x, y, world))
    {
        result = &world->sectors[x + y * world->width];
    } 
    return result;
}

Sector* getSectorFromPos(int x, int y)
{
    Sector* result = getSectorFromPos(x, y, currentGame->currentWorld);
    return result;
}

Sector* getSectorFromPos(jadel::Point2i pos, World* world)
{
    Sector* result = getSectorFromPos(pos.x, pos.y, world);
    return result;
}

Sector* getSectorFromPos(jadel::Point2i pos)
{
    Sector* result = getSectorFromPos(pos, currentGame->currentWorld);
    return result;
}

Sector* getSectorOfEntity(Entity* entity)
{
    Sector* result = getSectorFromPos(entity->pos);
    return result;
}

Sector* getSectorOfGameObject(GameObject* gameObject)
{
    Sector* result = getSectorOfEntity(&gameObject->entity);
    return result;
}

Sector* getSectorOfActor(Actor* actor)
{
    Sector* result = getSectorOfGameObject(&actor->gameObject);
    return result;
}

bool setPortal(int x0, int y0, uint32 world0ID, int x1, int y1, uint32 world1ID)
{
    World* world0 = getWorldByID(world0ID);
    World* world1 = getWorldByID(world1ID);
    if (!world0 || !world1) return false;
    if (!inBounds(x0, y0, world0) || !inBounds(x1, y1, world1)) return false;

    Sector* sector0 = getSectorFromPos(x0, y0, world0);
    Sector* sector1 = getSectorFromPos(x1, y1, world1);
    
    world0->portals[world0->numPortals] = {
        .sprite = &currentGame->portalSprite,
        .linkID = numPortalIDs,
        .worldLinkID = world1ID,
        .sector = sector0
    };

    world1->portals[world1->numPortals] = {
        .sprite = &currentGame->portalSprite,
        .linkID = numPortalIDs,
        .worldLinkID = world0ID,
        .sector = sector1
    };
    ++numPortalIDs;
    
    sector0->portal = &world0->portals[world0->numPortals++];
    sector1->portal = &world1->portals[world1->numPortals++];
    return true; 
}

void setSectorOccupant(int x, int y, Actor* occupant)
{
    Sector* sector = getSectorFromPos(x, y);
    if (sector)
    {
        sector->occupant = occupant;
    }
}

void setSectorOccupant(jadel::Point2i coords, Actor* occupant)
{
    setSectorOccupant(coords.x, coords.y, occupant);
}

void addSectorItem(Sector* sector, Item* item)
{
    if (sector && sector->numItems < 10)
    {
        sector->items[sector->numItems++] = item;
    }
}

void addSectorItem(int x, int y, Item* item)
{
    Sector* sector = getSectorFromPos(x, y);
    addSectorItem(sector, item);
}

bool moveTo(Sector* sector, Actor* actor)
{
    Entity* entity = &actor->gameObject.entity;
    jadel::Point2i nextTile = sector->pos;
    if (nextTile.x < 0 || nextTile.x >= currentGame->currentWorld->width ||
        nextTile.y < 0 || nextTile.y >= currentGame->currentWorld->height) return false;
    if (!currentGame->currentWorld->sectors[nextTile.x + nextTile.y * currentGame->currentWorld->width].tile->barrier)
    {
        Sector* currentSector = getSectorFromPos(entity->pos.x, entity->pos.y);
        currentSector->occupant = NULL;
        entity->pos = nextTile;
        Sector* nextSector = getSectorFromPos(entity->pos.x, entity->pos.y);
        nextSector->occupant = actor;
        return true;
    }
    printf("You hit a wall!\n");
    return false;

}

bool move(int x, int y, Actor* actor)
{
    Entity* entity = &actor->gameObject.entity;
    jadel::Point2i nextTile = {.x = x + entity->pos.x, .y = y + entity->pos.y};
    if (nextTile.x < 0 || nextTile.x >= currentGame->currentWorld->width ||
        nextTile.y < 0 || nextTile.y >= currentGame->currentWorld->height) return false;
    if (!currentGame->currentWorld->sectors[nextTile.x + nextTile.y * currentGame->currentWorld->width].tile->barrier)
    {
        actor->transit.inTransit = true;
        
        Sector* currentSector = getSectorFromPos(entity->pos.x, entity->pos.y);
        Sector* nextSector = getSectorFromPos(entity->pos.x + x, entity->pos.y + y);
        actor->transit.startSector = currentSector;
        actor->transit.endSector = nextSector;
        currentSector->occupant = NULL;
        entity->pos.x += x, entity->pos.y += y;
        nextSector->occupant = actor;
        return true;
    }
    printf("You hit a wall!\n");
    return false;
}


void attack(Actor* attacker, Actor* target)
{
    if (!target)
    {
        printf("No target to attack!\n");
        return;
    }
    int damage = 15;
    GameObject* gameObject = &target->gameObject;
    gameObject->health -= damage;
    if (gameObject->health < 0) gameObject->health = 0;
    printf("%s dealt %d damage to %s ! %s's hp: %d\n",
           attacker->gameObject.entity.name,
           damage,
           target->gameObject.entity.name,
           target->gameObject.entity.name,
           gameObject->health);
    if (gameObject->health <= 0)
    {
        gameObject->alive = false;
        printf("%s was killed...\n", target->gameObject.entity.name);
    }
}

bool tryToMove(int x, int y, Actor* actor)
{
    if (actor->transit.inTransit) return false;
    jadel::Point2i entityPos = actor->gameObject.entity.pos;
    Sector* nextSector = getSectorFromPos(entityPos.x + x, entityPos.y + y);
    if (!nextSector) return false;
    if (!nextSector->occupant)
    {
        return move(x, y, actor);
    }
    else
    {
        attack(actor, nextSector->occupant);
        if (nextSector->occupant->gameObject.alive)
        {
            attack(nextSector->occupant, actor);
        }
        else
        {
            nextSector->occupant = NULL;
        }
        return false;
    }
}

void initSector(int x, int y, const Tile* tile, Sector* target)
{
    target->pos = {x, y};
    target->tile = tile;
    target->occupant = NULL;
    target->numItems = 0;
    target->portal = NULL;
}

int distanceBetweenSectors(const Sector* a, const Sector* b)
{
    jadel::Point2i posA = a->pos;
    jadel::Point2i posB = b->pos;

    if (posA.x == posB.x && posA.y == posB.y) return 0;

    int xDiff = jadel::absInt(posB.x - posA.x);
    int yDiff = jadel::absInt(posB.y - posA.y);

    int higherDiff = xDiff > yDiff ? xDiff : yDiff;
    int lowerDiff = xDiff > yDiff ? yDiff : xDiff;
        
    int result = lowerDiff * 14 + (higherDiff - lowerDiff) * 10;
    return result;
}


AStarNode* getPathNode(int x, int y)
{
    if (x < 0 || x >= currentGame->currentWorld->width || y < 0 || y >= currentGame->currentWorld->height)
    {
        return NULL;
    }
    AStarNode* result = &currentGame->currentWorld->pathNodes[x + y * currentGame->currentWorld->width];
    return result;
}

AStarNode* getPathNodeOfSector(int x, int y)
{
    AStarNode* result = &currentGame->currentWorld->pathNodes[x + y * currentGame->currentWorld->width];
    return result;
}

AStarNode* getPathNodeOfSector(const Sector* sector)
{
    AStarNode* result = getPathNodeOfSector(sector->pos.x, sector->pos.y);
    return result;
}

AStarNode* calculatePathNodesFor(const Sector* sector, const Sector* start, const Sector* destination)
{
    if (sector == destination) return getPathNodeOfSector(sector);

    AStarNode* currentNode = getPathNodeOfSector(sector);
    if (sector == start)
    {
        currentNode->gCost = 0;
        currentNode->hCost = distanceBetweenSectors(start, destination);
        currentNode->isCalculated = true;
    }
    jadel::Point2i secPos = sector->pos;
    
    currentNode->isClosed = true;
    currentGame->currentWorld->calculatedPathNodes[currentGame->currentWorld->numCalculatedPathNodes++] = currentNode;
    AStarNode* surroundingNodes[8];
    int surroundingNodeIndex = 0;
    for (int y = 0; y < 3; ++y)
    {
        for (int x = 0; x < 3; ++x)
        {
            if (x == 1 && y == 1) continue;
            AStarNode* surroundingNode = getPathNode(secPos.x - 1 + x, secPos.y - 1 + y);
            if (surroundingNode && !surroundingNode->sector->tile->barrier)
            {
                surroundingNodes[surroundingNodeIndex++] = surroundingNode;
            }
        }
    }
    AStarNode* closestNode = NULL;
    int closestValue = -1;
    int prevGCost = -1;
    int prevHCost = -1;
    for (int i = 0; i < surroundingNodeIndex; ++i)
    {
        AStarNode* node = surroundingNodes[i];
        if (!node) continue;
        int nextGCost = distanceBetweenSectors(getSectorFromPos(node->pos), start);
        int nextHCost = distanceBetweenSectors(getSectorFromPos(node->pos), destination);
        
        bool refreshNode = !node->isCalculated;
        //bool refreshNode = (!node->isClosed && (!node->isCalculated ||
        //                                       (nextGCost + nextHCost) < (node->gCost + node->hCost)));
        if (refreshNode)
        {
            node->gCost = nextGCost;
            node->hCost = nextHCost;
            
            node->sourceNode = currentNode;
            if (!node->isCalculated) currentGame->currentWorld->calculatedPathNodes[currentGame->currentWorld->numCalculatedPathNodes++] = node;
            node->isCalculated = true;            
        }
        
        if (currentNode->sourceNode && nextGCost < currentNode->sourceNode->gCost)
        {
            currentNode->sourceNode = node;
        }
    }
    for (int i = 0; i < currentGame->currentWorld->numCalculatedPathNodes; ++i)
    {
        AStarNode* calculatedNode = currentGame->currentWorld->calculatedPathNodes[i];
        if (!calculatedNode->isClosed)
        {
            if (calculatedNode->gCost + calculatedNode->hCost < closestValue || closestValue == -1)
            {
                closestNode = calculatedNode;
                closestValue = calculatedNode->gCost + calculatedNode->hCost;
            }
            else if (calculatedNode->gCost + calculatedNode->hCost == closestValue)
            {
                if (calculatedNode->hCost < prevHCost)
                {
                    closestNode = calculatedNode;
                }
            }            
            prevHCost = calculatedNode->hCost;
            prevGCost = calculatedNode->gCost;

        }
    }
    
    return closestNode;
}


void resetPathNodes(World* world)
{
    world->numCalculatedPathNodes = 0;
    currentGame->pathLength = 0;
    for (int i = 0; i < world->width * world->height; ++i)
    {
        world->pathNodes[i].isClosed = false;
        world->pathNodes[i].isCalculated = false;
        world->pathNodes[i].isClosed = false;
        world->pathNodes[i].sourceNode = NULL;
        world->pathNodes[i].gCost = -1;
        world->pathNodes[i].hCost = -1;
    }
}


void calculatePath(const Sector* start, const Sector* destination)
{
    resetPathNodes(currentGame->currentWorld);
    const AStarNode* currentNode = getPathNodeOfSector(start->pos.x, start->pos.y);

    int debugCounter = 0;
    while (currentNode->sector != destination)
    {
        currentNode = calculatePathNodesFor(currentNode->sector, start, destination);
        ++debugCounter;
    }
    debugCounter = 0;
    while (currentNode->sector != start)
    {
        currentGame->path[currentGame->pathLength++] = currentNode->sector;
        currentNode = currentNode->sourceNode;
        ++debugCounter;
    }
}

void moveToSector(int x, int y, Actor* actor)
{
    getSectorOfActor(actor)->occupant = NULL;
    actor->gameObject.entity.pos = {x, y};
    getSectorFromPos(x, y)->occupant = actor;
}

jadel::Point2i getCameraLeft(jadel::Point2i camera)
{
    jadel::Point2i result = camera;
    return result;
}

jadel::Point2i getCameraRight(jadel::Point2i camera)
{
    jadel::Point2i result =
    {
        .x = camera.x + screenTilemapW,
        .y = camera.y + screenTilemapH
    };
    return result;
}

bool initWorld(int width, int height, World* world)
{
    if (!world) return false;

    world->entity = createEntity(0, 0);
    world->width = width;
    world->height = height;
    world->sectors = (Sector*)malloc(width * height * sizeof(Sector));
    world->numGameObjects = 0;
    world->numActors = 0;
    world->numItems = 0;
    world->numPortals = 0;
    world->pathNodes = (AStarNode*)malloc(width * height * sizeof(AStarNode));
    world->numCalculatedPathNodes = 0;
    world->calculatedPathNodes = (AStarNode**)malloc(width * height * sizeof(AStarNode*));
    currentGame->currentWorld = world;
    jadel::graphicsCreateSurface(width * currentGame->tileScreenW, height * currentGame->tileScreenH, &world->worldSurface);
    jadel::graphicsPushTargetSurface(&world->worldSurface);
    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            Sector* currentSector = &world->sectors[x + y * width];
            if (x % 3 == 0 && y % 3 == 0)
                initSector(x, y, &currentGame->wallTile, currentSector);
            else
                initSector(x, y, &currentGame->walkTile, currentSector);
            AStarNode* node = &world->pathNodes[x + y * width];
            node->pos = {x, y};
            node->sector = getSectorFromPos(x, y);
            const jadel::Surface* sectorSprite = currentSector->tile->surface;
            jadel::Recti sectorPos = getSectorScreenPos(x, y);
          
            jadel::graphicsBlit(sectorSprite, sectorPos);
            
        }
    }

    jadel::graphicsPopTargetSurface();

    
    return true;
}

bool initGame(jadel::Window* window)
{
    if (!currentGame)
    {
        printf("Could not init game. Null game pointer.\n");
        return false;
    }
    if (!window)
    {
        printf("Could not init game. Null window pointer.\n");
        return false;
    }
    
    currentGame->tileScreenW = window->width / screenTilemapW;
    currentGame->tileScreenH = window->height / screenTilemapH;


    gameCommands.numCommands = 7;
    gameCommands.keys[0] = jadel::KEY_LEFT;
    gameCommands.keys[1] = jadel::KEY_RIGHT;
    gameCommands.keys[2] = jadel::KEY_UP;
    gameCommands.keys[3] = jadel::KEY_DOWN;
    gameCommands.keys[4] = jadel::KEY_I;
    gameCommands.keys[5] = jadel::KEY_P;
    gameCommands.keys[6] = jadel::KEY_L;
    gameCommands.commands[0] = COMMAND_MOVE_LEFT;
    gameCommands.commands[1] = COMMAND_MOVE_RIGHT;
    gameCommands.commands[2] = COMMAND_MOVE_UP;
    gameCommands.commands[3] = COMMAND_MOVE_DOWN;
    gameCommands.commands[4] = COMMAND_TOGGLE_INVENTORY;
    gameCommands.commands[5] = COMMAND_TAKE_ITEM;
    gameCommands.commands[6] = COMMAND_LOOK;    

    currentGame->worlds = (World*)malloc(2 * sizeof(World));
    currentGame->numWorlds = 2;


    
 
    if (!loadBMP("res/clutter.bmp", &currentGame->clutterSprite)) return false;
    if (!loadBMP("res/grass.bmp", &currentGame->walkSurface)) return false;
    if (!loadBMP("res/dude1.bmp", &currentGame->playerSprite)) return false;
    if (!loadBMP("res/dude1l.bmp", &currentGame->playerSpriteLeft)) return false;
    if (!loadBMP("res/wall.bmp", &currentGame->wallSurface)) return false;
    if (!loadBMP("res/poison.bmp", &currentGame->poisonSprite)) return false;
    if (!loadBMP("res/hpotion.bmp", &currentGame->hpackSprite)) return false;
    if (!loadBMP("res/portal.bmp", &currentGame->portalSprite)) return false;

    currentGame->actors = (Actor*)malloc(MAX_ACTORS * sizeof(Actor));
    currentGame->items = (Item*)malloc(MAX_GAMEOBJECTS * sizeof(Item));
    currentGame->gameObjects = (GameObject*)malloc(MAX_GAMEOBJECTS * sizeof(GameObject));

    AnimFrames playerFrames;
    playerFrames.sprites[0] = &currentGame->playerSprite;
    playerFrames.sprites[1] = &currentGame->playerSpriteLeft;
    playerFrames.numFrames = 2;

    AnimFrames healthPackFrames;
    healthPackFrames.sprites[0] = &currentGame->hpackSprite;    
    healthPackFrames.numFrames = 1;

    AnimFrames poisonFrames;
    poisonFrames.sprites[0] = &currentGame->poisonSprite;    
    poisonFrames.numFrames = 1;
    

        
    
    jadel::graphicsCreateSurface(window->width, window->height, &currentGame->workingBuffer);
    
    
    currentGame->walkTile = {.surface = &currentGame->walkSurface, .barrier = false};
    currentGame->wallTile = {.surface = &currentGame->wallSurface, .barrier = true};


    initWorld(20, 10, &currentGame->worlds[0]);
    initWorld(8, 7, &currentGame->worlds[1]);

    jadel::graphicsPushTargetSurface(&currentGame->workingBuffer);
    jadel::graphicsSetClearColor(0);
    jadel::graphicsClearTargetSurface();
    
    currentGame->player = createActor(2, 3, playerFrames, "Player");

    currentGame->worlds[0].actors[0] = &currentGame->player;
    currentGame->worlds[1].actors[0] = &currentGame->player;
    currentGame->worlds[0].numActors++;
    currentGame->worlds[1].numActors++;

    pushActor(2, 1, playerFrames,"Teuvo", &currentGame->worlds[1]);
    pushActor(4, 3, playerFrames, "Jouko", &currentGame->worlds[0]);
    pushItem(6, 6, healthPackFrames, "health pack", ADJUST_HEALTH, 20, &currentGame->worlds[1]);
    pushItem(8, 6, poisonFrames, "poison", ADJUST_HEALTH, -10, &currentGame->worlds[0]);
    pushItem(9, 4, healthPackFrames, "health pack", ADJUST_HEALTH, 20, &currentGame->worlds[0]);
    pushItem(9, 4, poisonFrames, "poison", ADJUST_HEALTH, -10, &currentGame->worlds[0]);


    resetPathNodes(&currentGame->worlds[0]);
    resetPathNodes(&currentGame->worlds[1]);
    
    currentGame->worlds[0].portals[0] = {&currentGame->portalSprite, 1, currentGame->worlds[1].entity.GUID, &currentGame->worlds[0].sectors[2]};
    currentGame->worlds[1].portals[0] = {&currentGame->portalSprite, 1, currentGame->worlds[0].entity.GUID, &currentGame->worlds[1].sectors[3 + 4 * currentGame->worlds[1].width]};
    currentGame->worlds[0].numPortals++;
    currentGame->worlds[1].numPortals++;
    
    for (int i = 0; i < 2; ++i)
    {
        currentGame->currentWorld = &currentGame->worlds[i];
        World* curWorld = currentGame->currentWorld;
            
        for (int a = 0; a < curWorld->numActors; ++a)
        {
            jadel::Point2i actorPos = curWorld->actors[a]->gameObject.entity.pos;
            setSectorOccupant(actorPos.x, actorPos.y, curWorld->actors[a]);
        }
        
        for (int j = 0; j < curWorld->numItems; ++j)
        {
            jadel::Point2i itemPos = curWorld->items[j]->gameObject.entity.pos;
            addSectorItem(itemPos.x, itemPos.y, curWorld->items[j]);
        }
    }

    setPortal(2, 0, currentGame->worlds[0].entity.GUID, 3, 4, currentGame->worlds[1].entity.GUID); 
    //currentGame->worlds[0].sectors[2].portal = &currentGame->worlds[0].portals[0];
    //currentGame->worlds[1].sectors[3 + 4 * currentGame->worlds[1].width].portal = &currentGame->worlds[1].portals[0]; 
    currentGame->currentWorld = &currentGame->worlds[0];
    

    currentGame->player.inventory.useMode = false;
    
    currentGame->updateGame = true;

    currentGame->pSteps = 0;
    currentGame->currentState = SUBSTATE_GAME;
    currentGame->window = window;
    return true;
}



void updateSubstateGame()
{
    currentGame->numCommands = 0;
    for (int i = 0; i < gameCommands.numCommands; ++i)
    {
        if (jadel::inputIsKeyTyped(gameCommands.keys[i]))
        {
            currentGame->commandQueue[currentGame->numCommands++] = gameCommands.commands[i];
        }
    }

    AnimFrames* playerFrames = &currentGame->player.gameObject.frames;
    for (int i = 0; i < currentGame->numCommands; ++i)
    {
        switch (currentGame->commandQueue[i])
        {
            case COMMAND_MOVE_LEFT:
            {
                tryToMove(-1, 0, &currentGame->player);
                playerFrames->currentFrameIndex = 1;
                currentGame->updateGame = true;
                break;
            }
            case COMMAND_MOVE_RIGHT:
            {
                tryToMove(1, 0, &currentGame->player);
                playerFrames->currentFrameIndex = 0;
                currentGame->updateGame = true;
                break;
            }
            case COMMAND_MOVE_UP:
            {
                tryToMove(0, 1, &currentGame->player);
                currentGame->updateGame = true;
                break;
            }
            case COMMAND_MOVE_DOWN:
            {
                tryToMove(0, -1, &currentGame->player);
                currentGame->updateGame = true;
                break;
            }
            case COMMAND_TOGGLE_INVENTORY:
            {
                printInventory(&currentGame->player.inventory);
                currentGame->currentState = SUBSTATE_INVENTORY;
                break;
            }
            if (jadel::inputIsKeyTyped(jadel::KEY_K))
            {
                if (currentGame->pSteps < currentGame->pathLength)
                {
                    const Sector* nextSector = currentGame->path[currentGame->pathLength - 1 - currentGame->pSteps++];
                    moveToSector(nextSector->pos.x, nextSector->pos.y, &currentGame->player);
                    currentGame->updateGame = true;
                }
            }
            case COMMAND_TAKE_ITEM:
            {
                Sector* sector = getSectorFromPos(currentGame->player.gameObject.entity.pos.x,
                                                  currentGame->player.gameObject.entity.pos.y);
                if (sector->numItems > 0)
                {
                    ItemSlot* slot = NULL;
                    for (int i = 0; i < 10; ++i)
                    {
                        if (!currentGame->player.inventory.itemSlots[i].hasItem)
                        {
                            slot = &currentGame->player.inventory.itemSlots[i];
                            break;
                        }
                    }
                    if (slot)
                    {
                        Item* item = sector->items[sector->numItems - 1];
                        slot->item = item;
                        --sector->numItems;            
                        printf("Picked up %s\n", item->gameObject.entity.name);
                        slot->hasItem = true;
                    }
                    else
                    {
                        printf("Inventory full!\n");
                    }
                }
                else
                {
                    printf("Nothing to pick up\n");
                }
                break;
            }
            case COMMAND_LOOK:
            {
                Sector* curSector = getSectorOfEntity(&currentGame->player.gameObject.entity);
                if (curSector->numItems == 0)
                {
                    printf("Nothing here...\n");
                }
                else
                {
                    printf("You see:\n");
                    for (int i = 0; i < curSector->numItems; ++i)
                    {
                        printf("%d: %s\n", i + 1, curSector->items[i]->gameObject.entity.name);
                    }
                }
                break;
            }
            default:
                printf("Invalid action\n");
                break;
        }

    }
    if (jadel::inputIsKeyTyped(jadel::KEY_C))
    {
        currentGame->pSteps = 0;
        calculatePath(getSectorOfActor(&currentGame->player),
                      getSectorOfGameObject(&currentGame->currentWorld->items[1]->gameObject));
        currentGame->updateGame = true;
    }
    if (jadel::inputIsKeyTyped(jadel::KEY_M))
    {
        Portal* portal = getSectorOfActor(&currentGame->player)->portal;
        if (portal)
        {
            World* targetWorld = getWorldByID(portal->worldLinkID);
            for (int i = 0; i < targetWorld->numPortals; ++i)
            {
                Portal* targetPortal = &targetWorld->portals[i];
                if (targetPortal->worldLinkID == currentGame->currentWorld->entity.GUID
                    && targetPortal->linkID == portal->linkID)
                {
                    setSectorOccupant(currentGame->player.gameObject.entity.pos, NULL);
                    setWorldByID(portal->worldLinkID);
                    
                    //setSectorOccupant(targetPortal->sector->pos, &currentGame->player);
                    moveTo(targetPortal->sector, &currentGame->player);
                    currentGame->updateGame = true;        
                }
            }
            
        }
    }
    for (int i = 0; i < currentGame->currentWorld->numActors; ++i)
    {
        Actor* actor = currentGame->currentWorld->actors[i];
        if (actor->transit.inTransit)
        {
            actor->transit.progress += frameTime * 2;
            if (actor->transit.progress >= 1.0f)
            {
                actor->transit.progress = 0;
                actor->transit.inTransit = false;
            }
            currentGame->updateGame = true;
        }
    }

}

void updateGame()
{
    if (jadel::inputIsKeyPressed(jadel::KEY_ESCAPE))
    {
        exit(0);
    }

    switch (currentGame->currentState)
    {
        case SUBSTATE_GAME:
            updateSubstateGame();
            break;
        case SUBSTATE_INVENTORY:
            updateSubstateInventory();
           break;
        default:
            printf("Trying to update nonexistent substate");
            break;
    }
                     

    if (currentGame->updateGame)
    {              
        currentGame->screenPos =
            {.x = currentGame->player.gameObject.entity.pos.x - screenTilemapW / 2,
             .y = currentGame->player.gameObject.entity.pos.y - screenTilemapH / 2};
        currentGame->updateGame = false;
           
        jadel::graphicsClearTargetSurface();
        if (currentGame->screenPos.x > -screenTilemapW
            && currentGame->screenPos.y > -screenTilemapH
            && currentGame->screenPos.x < currentGame->currentWorld->width
            && currentGame->screenPos.y < currentGame->currentWorld->height)
        {
            int xStart = currentGame->screenPos.x < 0 ? 0 : currentGame->screenPos.x;
            int yStart = currentGame->screenPos.y < 0 ? 0 : currentGame->screenPos.y;
            int xEnd = currentGame->screenPos.x + screenTilemapW <= currentGame->currentWorld->width
                ? (currentGame->screenPos.x + screenTilemapW) : currentGame->currentWorld->width;
            int yEnd = currentGame->screenPos.y + screenTilemapH <= currentGame->currentWorld->height
                ? (currentGame->screenPos.y + screenTilemapH) : currentGame->currentWorld->height;

            jadel::Recti worldScreenEndDim = getSectorScreenPos(currentGame->currentWorld->width,
                                                         currentGame->currentWorld->height);
            if (worldScreenEndDim.x > currentGame->window->width) worldScreenEndDim.x = currentGame->window->width;
            if (worldScreenEndDim.y > currentGame->window->height) worldScreenEndDim.y = currentGame->window->height;

            //Recti worldScreenEndDim = getSectorScreenPos(getCameraRight(currentGame->screenPos));            
            
            jadel::Recti worldScreenDim = getSectorScreenPos(0, 0);
            if (worldScreenDim.x < 0) worldScreenDim.x = 0;
            if (worldScreenDim.y < 0) worldScreenDim.y = 0;

             
            worldScreenDim.w = worldScreenEndDim.x - worldScreenDim.x;
            worldScreenDim.h = worldScreenEndDim.y - worldScreenDim.y;
            
            jadel::Recti worldStartPos =
                {.x = currentGame->screenPos.x * currentGame->tileScreenW,
                 .y = currentGame->screenPos.y * currentGame->tileScreenH,
                 .w = worldScreenEndDim.x, //screenTilemapW * currentGame->tileScreenW,
                 .h = worldScreenEndDim.y}; //screenTilemapH * currentGame->tileScreenH};


               
            //jadel::graphicsBlit(&currentGame->currentWorld->worldSurface, worldStartPos, worldScreenDim);
                
            
            for (int y = yStart; y < yEnd; ++y)
            {
                for (int x = xStart; x < xEnd; ++x)
                {
                    jadel::Recti sectorPos = getSectorScreenPos(x, y);
                    Sector currentSector = currentGame->currentWorld->sectors[x + y * currentGame->currentWorld->width];
                    
                    const jadel::Surface* sectorSprite = NULL;
                    if (currentSector.portal)
                    {
                        sectorSprite = &currentGame->portalSprite;
                    }
                    else
                    {
                        sectorSprite = currentSector.tile->surface;
                    }
                    /*
                    else
                    {
                        sectorSprite = currentSector.tile->surface;
                    }
                    jadel::blitSurface(sectorSprite, sectorPos);
                    */
                    jadel::Recti entityDim
                            = {.x = sectorPos.x,
                               .y = sectorPos.y,
                               .w = currentGame->tileScreenW,
                               .h = currentGame->tileScreenH};
                    
                        
                    if (sectorSprite)
                    {
                        jadel::graphicsBlit(sectorSprite, entityDim);
                    }
                    
                    const jadel::Surface* spriteToDraw = NULL;

                    if (currentSector.occupant && !currentSector.occupant->transit.inTransit)
                    {
                        AnimFrames* frames
                            = &currentSector.occupant->gameObject.frames;
                        spriteToDraw = frames->sprites[frames->currentFrameIndex];
                    }
                    else if (currentSector.numItems == 1)
                    {
                        AnimFrames* frames
                            = &currentSector.items[currentSector.numItems - 1]->gameObject.frames;
                        spriteToDraw = frames->sprites[frames->currentFrameIndex];
                    }
                    else if (currentSector.numItems > 1)
                    {
                        spriteToDraw = &currentGame->clutterSprite;
                    }
                    if (spriteToDraw)
                    {
                        /*Recti sourceDim
                            ={.x = 122,
                               .y = 0,
                               .w = 122,
                               .h = 255};
                        */
                        
                        jadel::graphicsBlit(spriteToDraw, entityDim);

                    }
                    for (int i = 0; i < currentGame->currentWorld->numActors; ++i)
                    {
                        Actor* actor = currentGame->currentWorld->actors[i];
                        if (actor->transit.inTransit)
                        {
                            jadel::Recti startPos = getSectorScreenPos(actor->transit.startSector);
                            jadel::Recti endPos = getSectorScreenPos(actor->transit.endSector);
                            jadel::Point2i posDiff = {endPos.x - startPos.x, endPos.y - startPos.y};
                            jadel::Recti currentPoint =
                            {
                                jadel::roundToInt((float)startPos.x + (float)posDiff.x * actor->transit.progress),
                                jadel::roundToInt((float)startPos.y + (float)posDiff.y * actor->transit.progress),
                                currentGame->tileScreenW,
                                currentGame->tileScreenH
                            };
                            jadel::graphicsBlit(
                                actor->gameObject.frames.sprites[actor->gameObject.frames.currentFrameIndex], currentPoint); 
                        }
                    }
                }
            }
        }

            
        for (int i = 0; i < currentGame->pathLength; ++i)
        {            
            jadel::Recti sectorPos = getSectorScreenPos(currentGame->path[i]);
            for (int py = sectorPos.y; py < sectorPos.y + sectorPos.h; ++py)
            {
                for (int px = sectorPos.x; px < sectorPos.x + sectorPos.w; ++px)
                {
                    jadel::graphicsDrawPixelFast(px, py, 0xff550000);
                }
            }
        }
            
        //jadel::flipVertically(&currentGame->workingBuffer);
        //jadel::applySurface(&currentGame->workingBuffer, true, currentGame->window);
    }
}
