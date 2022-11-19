#include "game.h"
#include "render.h"
#include <jadel.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "render.h"
#include "rendergame.h"
#include "inventory.h"
#include "dice.h"
#include "globals.h"
#include <string>
#include "util.h"

Game *currentGame;
uint32 numIDs = 1;
uint32 numPortalIDs = 1;

int screenTilemapW;
int screenTilemapH;

static ControlScheme gameCommands;

float frameTime = 0;

void resetPathNodes(World *world);

World *getWorldByID(uint32 ID)
{
    for (int i = 0; i < currentGame->numWorlds; ++i)
    {
        World *world = &currentGame->worlds[i];
        if (world->entity.id == ID)
        {
            return world;
        }
    }
    return NULL;
}

bool setWorldByID(uint32 ID)
{
    World *world = getWorldByID(ID);
    if (!world)
    {
        return false;
    }
    currentGame->currentWorld = world;
    resetPathNodes(currentGame->currentWorld);

    jadel::Node<Actor *> *currentActorNode = world->actors.head;
    while (currentActorNode)
    {
        currentActorNode->data->clearPath();
        currentActorNode = currentActorNode->next;
    }
    return true;
}

jadel::LinkedList<Actor *> &getActors()
{
    return currentGame->currentWorld->actors;
}

void setGame(Game *game)
{
    currentGame = game;
}

uint32 getPlayerWeaponID()
{
    if (!currentGame->player.equippedWeapon)
        return 0;
    return currentGame->player.equippedWeapon->gameObject.entity.id;
}

/*
void pushItem(int x, int y, AnimFrames frames, const char *name, uint32 effect, int value, World *world)
{
    pushItem(createItem(x, y, frames, name, effect, value), world);
}*/

jadel::Recti getSectorScreenPos(int x, int y)
{
    jadel::Recti result = {.x0 = (x - currentGame->screenPos.x) * currentGame->tileScreenW,
                           .y0 = (y - currentGame->screenPos.y) * currentGame->tileScreenH,
                           .x1 = currentGame->tileScreenW,
                           .y1 = currentGame->tileScreenH};
    return result;
}

jadel::Recti getSectorScreenPos(jadel::Point2i pos)
{
    jadel::Recti result = getSectorScreenPos(pos.x, pos.y);
    return result;
}

jadel::Recti getSectorScreenPos(const Sector *sector)
{
    jadel::Point2i pos = sector->pos;
    jadel::Recti result = getSectorScreenPos(pos.x, pos.y);
    return result;
}

Sector *getSectorOfEntity(Entity *entity)
{
    Sector *result = getSectorFromPos(entity->pos);
    return result;
}

Sector *getSectorOfGameObject(GameObject *gameObject)
{
    Sector *result = getSectorOfEntity(&gameObject->entity);
    return result;
}

Sector *getSectorOfActor(Actor *actor)
{
    Sector *result = getSectorOfGameObject(&actor->gameObject);
    return result;
}

void setSectorOccupant(int x, int y, Actor *occupant)
{
    Sector *sector = getSectorFromPos(x, y);
    if (sector)
    {
        sector->occupant = occupant;
    }
}

void setSectorOccupant(jadel::Point2i coords, Actor *occupant)
{
    setSectorOccupant(coords.x, coords.y, occupant);
}

void addSectorItem(Sector *sector, Item *item)
{
    addSectorItem(sector, item, currentGame->currentWorld);
}

void addSectorItem(int x, int y, Item *item)
{
    addSectorItem(x, y, item, currentGame->currentWorld);
}

void removeSectorItem(Sector *sector, Item *item)
{
    removeSectorItem(sector, item, currentGame->currentWorld);
}

void removeSectorItem(int x, int y, Item *item)
{
    Sector *sector = getSectorFromPos(x, y);
    removeSectorItem(sector, item);
}

bool moveTo(Sector *sector, Actor *actor)
{
    Entity *entity = &actor->gameObject.entity;
    jadel::Point2i nextTile = sector->pos;
    if (nextTile.x < 0 || nextTile.x >= currentGame->currentWorld->width ||
        nextTile.y < 0 || nextTile.y >= currentGame->currentWorld->height)
        return false;
    if (!currentGame->currentWorld->sectors[nextTile.x + nextTile.y * currentGame->currentWorld->width].tile->barrier)
    {
        Sector *currentSector = getSectorFromPos(entity->pos.x, entity->pos.y);
        currentSector->occupant = NULL;
        entity->pos = nextTile;
        Sector *nextSector = getSectorFromPos(entity->pos.x, entity->pos.y);
        nextSector->occupant = actor;
        return true;
    }
    jadel::message("You hit a wall!\n");
    return false;
}

bool move(int x, int y, Actor *actor)
{
    Entity *entity = &actor->gameObject.entity;
    jadel::Point2i nextTile = {.x = x + entity->pos.x, .y = y + entity->pos.y};
    if (nextTile.x < 0 || nextTile.x >= currentGame->currentWorld->width ||
        nextTile.y < 0 || nextTile.y >= currentGame->currentWorld->height)
        return false;
    if (!currentGame->currentWorld->sectors[nextTile.x + nextTile.y * currentGame->currentWorld->width].tile->barrier)
    {
        //    actor->transit.inTransit = true;

        Sector *currentSector = getSectorFromPos(entity->pos.x, entity->pos.y);
        Sector *nextSector = getSectorFromPos(entity->pos.x + x, entity->pos.y + y);
        //    actor->transit.startSector = currentSector;
        //    actor->transit.endSector = nextSector;
        currentSector->occupant = NULL;
        entity->pos.x += x, entity->pos.y += y;
        nextSector->occupant = actor;
        return true;
    }
    jadel::message("You hit a wall!\n");
    return false;
}

void killActor(Actor *actor)
{
    getSectorOfActor(actor)->occupant = NULL;
    auto &actors = getActors();
    actors.deleteWithValue(actor);
    currentGame->actorFactory.release(actor);
}

void attack(Actor *attacker, Actor *target)
{
    const char *weaponName;
    const char *defaultWeapon = "fists";
    if (attacker->equippedWeapon)
    {
        weaponName = attacker->equippedWeapon->gameObject.entity.name.c_str();
    }
    else
        weaponName = defaultWeapon;
    if (!target)
    {
        jadel::message("No target to attack!\n");
        return;
    }
    bool attackHit = rollAttackHit(attacker, target);
    if (!attackHit)
    {
        jadel::message("%s's attack missed\n", attacker->gameObject.entity.name.c_str());
        return;
    }
    uint32 damage;
    if (!attacker->equippedWeapon)
        damage = getStrengthModifier(attacker) + 1; // unarmed strike
    else
        damage = calculateDamage(attacker->equippedWeapon->damage) + getStrengthModifier(attacker);
    GameObject *gameObject = &target->gameObject;
    gameObject->health -= damage;
    if (gameObject->health < 0)
        gameObject->health = 0;
    jadel::message("%s dealt %d damage to %s with their %s ! %s's hp: %d\n",
                   attacker->gameObject.entity.name.c_str(),
                   damage,
                   target->gameObject.entity.name.c_str(),
                   weaponName,
                   target->gameObject.entity.name.c_str(),
                   gameObject->health);
    if (gameObject->health == 0)
    {
        gameObject->alive = false;

        jadel::message("%s was killed...\n", target->gameObject.entity.name.c_str());
        killActor(target);
    }
}

void combat(Actor *actor0, Actor *actor1)
{
    Actor *firstAttacker;
    Actor *secondAttacker;

    int actor0Initiative = rollInitiative(actor0);
    int actor1Initiative = rollInitiative(actor1);

    if (actor0Initiative == actor1Initiative)
    {
        if (rand() % 2 == 0)
        {
            firstAttacker = actor0;
            secondAttacker = actor1;
        }
        else
        {
            firstAttacker = actor1;
            secondAttacker = actor0;
        }
    }
    else if (actor0Initiative > actor1Initiative)
    {
        firstAttacker = actor0;
        secondAttacker = actor1;
    }
    else
    {
        firstAttacker = actor1;
        secondAttacker = actor0;
    }

    attack(firstAttacker, secondAttacker);
    if (secondAttacker->gameObject.alive)
    {
        attack(secondAttacker, firstAttacker);
    }
}

// Returns true if sector is changed
bool tryToMove(int x, int y, Actor *actor)
{
    // if (actor->transit.inTransit)
    //     return false;
    GameObject *gameObject = &actor->gameObject;
    jadel::Point2i nextPosInsideSquare = {.x = gameObject->posInsideSquare.x + x, .y = gameObject->posInsideSquare.y + y};
    if (nextPosInsideSquare.x > -10 && nextPosInsideSquare.x < 10 && nextPosInsideSquare.y > -10 && nextPosInsideSquare.y < 10)
    {
        gameObject->posInsideSquare = nextPosInsideSquare;
        return false;
    }

    jadel::Point2i entityPos = gameObject->entity.pos;

    int nextSectorXDiff;
    int nextSectorYDiff;

    if (nextPosInsideSquare.x <= -10)
        nextSectorXDiff = -1;
    else if (nextPosInsideSquare.x > 10)
        nextSectorXDiff = 1;
    else
        nextSectorXDiff = 0;

    if (nextPosInsideSquare.y <= -10)
        nextSectorYDiff = -1;
    else if (nextPosInsideSquare.y > 10)
        nextSectorYDiff = 1;
    else
        nextSectorYDiff = 0;

    Sector *nextSector = getSectorFromPos(entityPos.x + nextSectorXDiff, entityPos.y + nextSectorYDiff);
    if (!nextSector)
        return false;
    if (!nextSector->occupant)
    {
        if (nextPosInsideSquare.x <= -10)
            nextPosInsideSquare.x += 20;
        else if (nextPosInsideSquare.x > 10)
            nextPosInsideSquare.x -= 20;
        if (nextPosInsideSquare.y <= -10)
            nextPosInsideSquare.y += 20;
        else if (nextPosInsideSquare.y > 10)
            nextPosInsideSquare.y -= 20;
        gameObject->posInsideSquare = nextPosInsideSquare;
        // return move(x, y, actor);
        return moveTo(nextSector, actor);
    }
    else
    {
        Actor *attackTarget = nextSector->occupant;
        combat(actor, attackTarget);
        return false;
    }
}

AStarNode *getPathNode(int x, int y)
{
    if (x < 0 || x >= currentGame->currentWorld->width || y < 0 || y >= currentGame->currentWorld->height)
    {
        return NULL;
    }
    AStarNode *result = &currentGame->currentWorld->pathNodes[x + y * currentGame->currentWorld->width];
    return result;
}

AStarNode *getPathNodeOfSector(int x, int y)
{
    AStarNode *result = &currentGame->currentWorld->pathNodes[x + y * currentGame->currentWorld->width];
    return result;
}

AStarNode *getPathNodeOfSector(const Sector *sector)
{
    AStarNode *result = getPathNodeOfSector(sector->pos.x, sector->pos.y);
    return result;
}

AStarNode *calculatePathNodesFor(const Sector *sector, const Sector *start, const Sector *destination)
{
    if (sector == destination)
        return getPathNodeOfSector(sector);

    AStarNode *currentNode = getPathNodeOfSector(sector);
    if (sector == start)
    {
        currentNode->gCost = 0;
        currentNode->hCost = distanceBetweenSectors(start, destination);
        currentNode->isCalculated = true;
    }
    jadel::Point2i secPos = sector->pos;

    currentNode->isClosed = true;
    currentGame->currentWorld->calculatedPathNodes[currentGame->currentWorld->numCalculatedPathNodes++] = currentNode;
    AStarNode *surroundingNodes[8];
    int surroundingNodeIndex = 0;
    for (int y = 0; y < 3; ++y)
    {
        for (int x = 0; x < 3; ++x)
        {
            if (x == 1 && y == 1)
                continue;
            AStarNode *surroundingNode = getPathNode(secPos.x - 1 + x, secPos.y - 1 + y);
            if (surroundingNode && !surroundingNode->sector->tile->barrier)
            {
                surroundingNodes[surroundingNodeIndex++] = surroundingNode;
            }
        }
    }
    AStarNode *closestNode = NULL;
    int closestValue = -1;
    int prevGCost = -1;
    int prevHCost = -1;
    for (int i = 0; i < surroundingNodeIndex; ++i)
    {
        AStarNode *node = surroundingNodes[i];
        if (!node)
            continue;
        int nextGCost = distanceBetweenSectors(getSectorFromPos(node->pos), start);
        int nextHCost = distanceBetweenSectors(getSectorFromPos(node->pos), destination);

        bool refreshNode = !node->isCalculated;
        // bool refreshNode = (!node->isClosed && (!node->isCalculated ||
        //                                        (nextGCost + nextHCost) < (node->gCost + node->hCost)));
        if (refreshNode)
        {
            node->gCost = nextGCost;
            node->hCost = nextHCost;

            node->sourceNode = currentNode;
            if (!node->isCalculated)
                currentGame->currentWorld->calculatedPathNodes[currentGame->currentWorld->numCalculatedPathNodes++] = node;
            node->isCalculated = true;
        }

        if (currentNode->sourceNode && nextGCost < currentNode->sourceNode->gCost)
        {
            currentNode->sourceNode = node;
        }
    }
    for (int i = 0; i < currentGame->currentWorld->numCalculatedPathNodes; ++i)
    {
        AStarNode *calculatedNode = currentGame->currentWorld->calculatedPathNodes[i];
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

void resetPathNodes(World *world)
{
    world->numCalculatedPathNodes = 0;
    // currentGame->pathLength = 0;
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

void calculatePath(const Sector *start, const Sector *destination, Actor *actor)
{
    if (!actor)
        return;
    actor->clearPath();
    resetPathNodes(currentGame->currentWorld);
    const AStarNode *currentNode = getPathNodeOfSector(start->pos.x, start->pos.y);

    int debugCounter = 0;
    while (currentNode->sector != destination)
    {
        currentNode = calculatePathNodesFor(currentNode->sector, start, destination);
        ++debugCounter;
    }
    debugCounter = 0;
    while (currentNode->sector != start)
    {
        actor->path[actor->pathLength++] = currentNode->sector;
        currentNode = currentNode->sourceNode;
        ++debugCounter;
    }
}

void moveToSector(int x, int y, Item *item)
{
    if (!item)
    {
        return;
    }
    Sector *currentSector = getSectorOfGameObject(&item->gameObject);
    if (!currentSector)
    {
        return;
    }
    Sector *nextSector = getSectorFromPos(x, y);
    if (!nextSector)
    {
        return;
    }
    item->gameObject.entity.pos = {x, y};
    removeSectorItem(currentSector, item);
    addSectorItem(nextSector, item);
}

void moveToSector(int x, int y, Actor *actor)
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
            .y = camera.y + screenTilemapH};
    return result;
}

bool initGame(jadel::Window *window)
{
    if (!currentGame)
    {
        jadel::message("Could not init game. Null game pointer.\n");
        return false;
    }
    if (!window)
    {
        jadel::message("Could not init game. Null window pointer.\n");
        return false;
    }

    RendererInfo rendererInfo;
    rendererInfo.screenObjectPoolSize = 500;
    rendererInfo.stringBufferSize = 500;
    rendererInfo.workingBufferDimensions = {window->width, window->height};
    Renderer *renderer = &currentGame->gameRenderer;
    if (!renderer->init(rendererInfo))
    {
        return false;
    }
    currentGame->gameLayer = renderer->createLayer();
    currentGame->uiLayer = renderer->createLayer();
    /*
        if (!systemInitRender(window))
        {
            return false;
        }
    */

    if (!systemInitRenderGame(window, &currentGame->gameRenderer))
    {
        return false;
    }
    if (!systemInitInventory())
    {
        return false;
    }

    screenTilemapW = 16;
    screenTilemapH = 9;
    currentGame->tileScreenW = window->width / screenTilemapW;
    currentGame->tileScreenH = window->height / screenTilemapH;

    gameCommands.numKeyPressCommands = 4;
    gameCommands.numKeyTypeCommands = 3;
    gameCommands.keyPress[0] = jadel::KEY_A;
    gameCommands.keyPress[1] = jadel::KEY_D;
    gameCommands.keyPress[2] = jadel::KEY_W;
    gameCommands.keyPress[3] = jadel::KEY_S;
    gameCommands.keyType[0] = jadel::KEY_TAB;
    gameCommands.keyType[1] = jadel::KEY_E;
    gameCommands.keyType[2] = jadel::KEY_L;
    gameCommands.keyPressCommands[0] = COMMAND_MOVE_LEFT;
    gameCommands.keyPressCommands[1] = COMMAND_MOVE_RIGHT;
    gameCommands.keyPressCommands[2] = COMMAND_MOVE_UP;
    gameCommands.keyPressCommands[3] = COMMAND_MOVE_DOWN;
    gameCommands.keyTypeCommands[0] = COMMAND_TOGGLE_INVENTORY;
    gameCommands.keyTypeCommands[1] = COMMAND_TAKE_ITEM;
    gameCommands.keyTypeCommands[2] = COMMAND_LOOK;

    currentGame->worlds = (World *)jadel::memoryReserve(2 * sizeof(World));
    currentGame->numWorlds = 2;

    currentGame->actorFactory.init(MAX_ACTORS);
    currentGame->gameObjectFactory.init(MAX_GAMEOBJECTS);
    currentGame->itemFactory.init(MAX_GAMEOBJECTS);
    AnimFrames playerFrames;
    playerFrames.sprites[0] = g_Assets.getSurface("res/dude1.png");
    playerFrames.sprites[1] = g_Assets.getSurface("res/dude1l.png");
    playerFrames.numFrames = 2;

    AnimFrames healthPackFrames;
    healthPackFrames.sprites[0] = g_Assets.getSurface("res/hpotion.png");
    healthPackFrames.sprites[1] = g_Assets.getSurface("res/hpotion2.png");
    healthPackFrames.sprites[2] = g_Assets.getSurface("res/hpotion3.png");
    healthPackFrames.numFrames = 3;

    AnimFrames poisonFrames;
    poisonFrames.sprites[0] = g_Assets.getSurface("res/poison.png");
    poisonFrames.sprites[1] = g_Assets.getSurface("res/poison2.png");
    poisonFrames.sprites[2] = g_Assets.getSurface("res/poison3.png");
    poisonFrames.numFrames = 3;

    AnimFrames daggerFrames;
    daggerFrames.sprites[0] = g_Assets.getSurface("res/dagger.png");
    daggerFrames.numFrames = 1;

    currentGame->walkTile = {.surface = g_Assets.getSurface("res/grass.png"), .barrier = false};
    currentGame->wallTile = {.surface = g_Assets.getSurface("res/wall.png"), .barrier = true};

    //initWorld(20, 10, &currentGame->worlds[0]);
    
    if (!loadWorld("res/testlevel.txt", &currentGame->worlds[0]))
    {
        return false;
    }
    initWorld(8, 7, &currentGame->worlds[1]);
    currentGame->currentWorld = &currentGame->worlds[0];
    currentGame->testBox = (DialogBox *)jadel::memoryReserve(sizeof(DialogBox));

    dialogBoxInit(currentGame->testBox, jadel::Vec2(12.0f, -9.0f), jadel::Vec2(4.0f, 3.0f), "Hello", g_Assets.getSurface("res/inventory3.png"), 0);
    currentGame->button1id = dialogBoxAddButton(currentGame->testBox, "Memory info", g_Assets.getSurface("res/bup.png"), g_Assets.getSurface("res/bdown.png"), {0.7f, 0, 0, 1}, BUTTON_GRAPHICS_TEXT | BUTTON_GRAPHICS_IMAGE);
    currentGame->button2id = dialogBoxAddButton(currentGame->testBox, "Inventory", g_Assets.getSurface("res/bup.png"), g_Assets.getSurface("res/bdown.png"), {0.7f, 0, 0, 1}, BUTTON_GRAPHICS_TEXT | BUTTON_GRAPHICS_IMAGE);
    currentGame->button3id = dialogBoxAddButton(currentGame->testBox, "Quit Game", g_Assets.getSurface("res/bup.png"), g_Assets.getSurface("res/bdown.png"), {0.7f, 0, 0, 1}, BUTTON_GRAPHICS_TEXT | BUTTON_GRAPHICS_IMAGE);
    Attributes playerAttrib = {
        .strength = 10,
        .dexterity = 12,
        .constitution = 30,
        .intelligence = 11,
        .wisdom = 10,
        .charisma = 10};
    currentGame->player = createActor(3, 2, playerFrames, "Player", &playerAttrib);
    currentGame->worlds[0].actors.append(&currentGame->player);
    // currentGame->worlds[1].actors.push(&currentGame->player);

    GameObjectTemplate healthPackTemplate;
    healthPackTemplate.affectedByLight = false;
    healthPackTemplate.frames = healthPackFrames;
    healthPackTemplate.maxHealth = 100;
    healthPackTemplate.name = "Health potion";

    pushActor(2, 1, playerFrames, "Teuvo", &currentGame->worlds[1]);
    pushActor(4, 3, playerFrames, "Jouko", &currentGame->worlds[0]);
    pushItem(createHealthItem(6, 5, &healthPackTemplate, 20), &currentGame->worlds[1]);
    pushItem(createHealthItem(8, 6, poisonFrames, "Poison", -10), &currentGame->worlds[0]);
    pushItem(createHealthItem(9, 4, &healthPackTemplate, 20), &currentGame->worlds[0]);
    pushItem(createHealthItem(10, 4, &healthPackTemplate, 20), &currentGame->worlds[0]);
    pushItem(createHealthItem(9, 5, &healthPackTemplate, 20), &currentGame->worlds[0]);
    pushItem(createHealthItem(9, 4, poisonFrames, "Poison", -10), &currentGame->worlds[0]);
    pushItem(createIlluminatorItem(6, 5, {0}, "Light", jadel::Vec3(250.0f, 4.0f, 150.0f)), &currentGame->worlds[0]);
    pushItem(createIlluminatorItem(18, 3, {0}, "Light", jadel::Vec3(250.0f, 250.0f, 0.0f)), &currentGame->worlds[0]);
    pushItem(createIlluminatorItem(3, 3, {0}, "Light", jadel::Vec3(25.0f, 25.0f, 150.0f)), &currentGame->worlds[1]);
    pushItem(createWeaponItem(7, 4, daggerFrames, "Dagger", 2, 6), &currentGame->worlds[0]);
    resetPathNodes(&currentGame->worlds[0]);
    resetPathNodes(&currentGame->worlds[1]);

    currentGame->worlds[0].portals[0] = {g_Assets.getSurface("res/portal.png"), 1, currentGame->worlds[1].entity.id, &currentGame->worlds[0].sectors[2]};
    currentGame->worlds[1].portals[0] = {g_Assets.getSurface("res/portal.png"), 1, currentGame->worlds[0].entity.id, &currentGame->worlds[1].sectors[3 + 4 * currentGame->worlds[1].width]};
    currentGame->worlds[0].numPortals++;
    currentGame->worlds[1].numPortals++;

    for (int i = 0; i < 2; ++i)
    {
        currentGame->currentWorld = &currentGame->worlds[i];
        World *curWorld = currentGame->currentWorld;

        LinkedListIterator actorIterator(&curWorld->actors);
        Actor** actor;
        while (actor = actorIterator.getNext())
        {
            jadel::Point2i actorPos = (*actor)->gameObject.entity.pos;
            setSectorOccupant(actorPos.x, actorPos.y, *actor);
        }
        calculateLights(curWorld);
    }
    setPortal(2, 0, currentGame->worlds[0].entity.id, 3, 4, currentGame->worlds[1].entity.id);
    
    currentGame->currentWorld = &currentGame->worlds[0];

    currentGame->player.inventory.useMode = false;

    currentGame->updateCamera = true;

    currentGame->screenPos =
        {
            .x = currentGame->player.gameObject.entity.pos.x - screenTilemapW / 2,
            .y = currentGame->player.gameObject.entity.pos.y - screenTilemapH / 2};
    currentGame->currentState = SUBSTATE_GAME;
    currentGame->window = window;
    currentGame->playerCanMove = true;
    currentGame->moveTimerMillis = 0;
    currentGame->spriteTimerMillis = 0;
    currentGame->spriteTimer.start();
    currentGame->gameRenderer.render();
    renderGame();
    return true;
}

static bool followPlayer = false;

void executeCommand(uint32 command, Actor *actor)
{
    if (!actor)
        return;
    bool moved = false;
    switch (command)
    {
    case COMMAND_MOVE_LEFT:
    {
        moved = tryToMove(-20, 0, actor);
        break;
    }
    case COMMAND_MOVE_RIGHT:
    {
        moved = tryToMove(20, 0, actor);
        break;
    }
    case COMMAND_MOVE_UP:
    {
        moved = tryToMove(0, 20, actor);
        break;
    }
    case COMMAND_MOVE_DOWN:
    {
        moved = tryToMove(0, -20, actor);
        break;
    }
    case COMMAND_MOVE_UP_LEFT:
    {
        moved = tryToMove(-14, 14, actor);
        break;
    }
    case COMMAND_MOVE_UP_RIGHT:
    {
        moved = tryToMove(14, 14, actor);
        break;
    }
    case COMMAND_MOVE_DOWN_RIGHT:
    {
        moved = tryToMove(14, -14, actor);
        break;
    }
    case COMMAND_MOVE_DOWN_LEFT:
    {
        moved = tryToMove(-14, -14, actor);
        break;
    }
    }
    if (actor->followingPath && moved)
    {
        if (getSectorOfActor(actor) == actor->path[actor->pathLength - 1 - actor->pathStepsTaken])
            actor->pathStepsTaken++;
    }
    actor->commandInQueue = 0;
}

uint32 getSectorDir(jadel::Point2i currentPos, const Sector *sector)
{
    if (!sector)
        return 0;
    jadel::Point2i sectorPos = sector->pos;
    jadel::Point2i dir = sectorPos - currentPos;
    if (dir == jadel::Point2i{-1, 1})
        return COMMAND_MOVE_UP_LEFT;
    if (dir == jadel::Point2i{0, 1})
        return COMMAND_MOVE_UP;
    if (dir == jadel::Point2i{1, 1})
        return COMMAND_MOVE_UP_RIGHT;
    if (dir == jadel::Point2i{-1, 0})
        return COMMAND_MOVE_LEFT;
    if (dir == jadel::Point2i{1, 0})
        return COMMAND_MOVE_RIGHT;
    if (dir == jadel::Point2i{-1, -1})
        return COMMAND_MOVE_DOWN_LEFT;
    if (dir == jadel::Point2i{0, -1})
        return COMMAND_MOVE_DOWN;
    if (dir == jadel::Point2i{1, -1})
        return COMMAND_MOVE_DOWN_RIGHT;

    return COMMAND_NULL;
}

void pushCommand(uint32 command)
{
    if (command > COMMAND_NULL && command < COMMAND_COUNT)
    {
        currentGame->commandQueue[currentGame->numCommands++] = command;
    }
}

void updateSubstateGame()
{

    for (int i = 0; i < gameCommands.numKeyTypeCommands; ++i)
    {
        if (jadel::inputIsKeyTyped(gameCommands.keyType[i]))
        {
            pushCommand(gameCommands.keyTypeCommands[i]);
        }
    }
    for (int i = 0; i < gameCommands.numKeyPressCommands; ++i)
    {
        if (jadel::inputIsKeyPressed(gameCommands.keyPress[i]))
        {
            pushCommand(gameCommands.keyPressCommands[i]);
        }
    }
    if (jadel::inputIsKeyTyped(jadel::KEY_K))
    {
        if (currentGame->player.pathStepsTaken < currentGame->player.pathLength)
        {
            const Sector *nextSector = currentGame->player.path[currentGame->player.pathLength - 1 - currentGame->player.pathStepsTaken++];
            moveToSector(nextSector->pos.x, nextSector->pos.y, &currentGame->player);
        }
    }

    bool playerMoved = false;
    for (int i = 0; i < currentGame->numCommands; ++i)
    {
        switch (currentGame->commandQueue[i])
        {
        case COMMAND_MOVE_LEFT:
        {
            if (currentGame->playerCanMove)
            {
                if (tryToMove(-20, 0, &currentGame->player))
                    playerMoved = true;
                setFrame(1, &currentGame->player);
            }
            break;
        }
        case COMMAND_MOVE_RIGHT:
        {
            if (currentGame->playerCanMove)
            {
                if (tryToMove(20, 0, &currentGame->player))
                    playerMoved = true;
                setFrame(0, &currentGame->player);
            }
            break;
        }
        case COMMAND_MOVE_UP:
        {
            if (currentGame->playerCanMove)
            {
                if (tryToMove(0, 20, &currentGame->player))
                    playerMoved = true;
            }
            break;
        }
        case COMMAND_MOVE_DOWN:
        {
            if (currentGame->playerCanMove)
            {
                if (tryToMove(0, -20, &currentGame->player))
                    playerMoved = true;
            }
            break;
        }
        case COMMAND_TOGGLE_INVENTORY:
        {
            printInventory(&currentGame->player.inventory);
            currentGame->currentState = SUBSTATE_INVENTORY;
            currentGame->player.inventory.opening = true;
            break;
        }
        case COMMAND_TAKE_ITEM:
        {
            Sector *sector = getSectorFromPos(currentGame->player.gameObject.entity.pos.x,
                                              currentGame->player.gameObject.entity.pos.y);
            jadel::Node<Item *> *currentItemNode = sector->items.head;
            if (currentItemNode)
            {
                ItemSlot *slot = NULL;
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
                    Item *item = currentItemNode->data;
                    slot->item = item;
                    jadel::message("Picked up %s\n", item->gameObject.entity.name.c_str());
                    slot->hasItem = true;
                    removeSectorItem(sector, item);
                    currentGame->updateCamera = true;
                }
                else
                {
                    jadel::message("Inventory full!\n");
                }
            }
            else
            {
                jadel::message("Nothing to pick up\n");
            }
            break;
        }
        case COMMAND_LOOK:
        {
            Sector *curSector = getSectorOfEntity(&currentGame->player.gameObject.entity);
            jadel::Node<Item *> *currentItemNode = curSector->items.head;
            if (!currentItemNode)
            {
                jadel::message("Nothing here...\n");
            }
            else
            {
                while (currentItemNode)
                {
                    jadel::message("You see:\n");
                    jadel::message("%d: %s\n", i + 1, currentItemNode->data->gameObject.entity.name.c_str());

                    currentItemNode = currentItemNode->next;
                }
            }
            break;
        }
        default:
            jadel::message("Invalid action\n");
            break;
        }
    }

    if (jadel::inputIsKeyTyped(jadel::KEY_V))
    {
        Actor *actor = *getActors().getHead();
        actor->followingPath = !actor->followingPath;
        if (actor->followingPath)
        {
            calculatePath(getSectorOfActor(actor),
                          getSectorOfActor(&currentGame->player), actor);
        }
    }

    if (playerMoved) // jadel::inputIsKeyTyped(jadel::KEY_M))
    {
        Portal *portal = getSectorOfActor(&currentGame->player)->portal;
        if (portal)
        {
            World *targetWorld = getWorldByID(portal->worldLinkID);
            for (int i = 0; i < targetWorld->numPortals; ++i)
            {
                Portal *targetPortal = &targetWorld->portals[i];
                if (targetPortal->worldLinkID == currentGame->currentWorld->entity.id && targetPortal->linkID == portal->linkID)
                {
                    setSectorOccupant(currentGame->player.gameObject.entity.pos, NULL);
                    setWorldByID(portal->worldLinkID);

                    // setSectorOccupant(targetPortal->sector->pos, &currentGame->player);
                    moveTo(targetPortal->sector, &currentGame->player);
                }
            }
        }
    }

    if (playerMoved)
    {
        currentGame->updateCamera = true;
        currentGame->playerCanMove = false;
        currentGame->moveTimer.start();
    }
    /*
        for (int i = 0; i < currentGame->currentWorld->numActors; ++i)
        {
            Actor *actor = currentGame->currentWorld->actors[i];
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
        }*/
}

void updateGame()
{
    currentGame->updateCamera = false;
    Renderer *renderer = &currentGame->gameRenderer;
    dialogBoxUpdate(currentGame->testBox);
    if (jadel::inputIsKeyPressed(jadel::KEY_ESCAPE) || isButtonState(currentGame->button3id, BUTTON_STATE_RELEASED, currentGame->testBox))
    {
        exit(0);
    }
    currentGame->numCommands = 0;
    // jadel::message("%f %f\n", jadel::inputGetMouseXRelative(), jadel::inputGetMouseYRelative());
    if (!currentGame->playerCanMove)
    {
        currentGame->moveTimerMillis = currentGame->moveTimer.getMillisSinceLastUpdate();
        if (currentGame->moveTimerMillis >= 125)
        {
            currentGame->moveTimerMillis %= 125;
            currentGame->playerCanMove = true;
        }
    }
    if (isButtonState(currentGame->button2id, BUTTON_STATE_RELEASED, currentGame->testBox))
    {
        pushCommand(COMMAND_TOGGLE_INVENTORY);
    }
    switch (currentGame->currentState)
    {
    case SUBSTATE_GAME:
        updateSubstateGame();
        break;
    case SUBSTATE_INVENTORY:
        updateSubstateInventory(&currentGame->player.inventory);
        break;
    default:
        jadel::message("Trying to update nonexistent substate\n");
        break;
    }
    currentGame->spriteTimerMillis += currentGame->spriteTimer.getMillisSinceLastUpdate();
    currentGame->spriteTimer.update();
    if (currentGame->spriteTimerMillis >= 333)
    {
        currentGame->spriteTimerMillis %= 333;
        auto &items = currentGame->currentWorld->items;
        LinkedListIterator itemIterator(&items);
        Item** item;
        while (item = itemIterator.getNext())
        {
            setNextFrame(&(*item)->gameObject);
        }
    }

    dialogBoxRender(currentGame->testBox, currentGame->uiLayer, &currentGame->gameRenderer);

    auto actors = getActors();

    LinkedListIterator actorIterator(&actors);
    Actor** actorAddr;
    while (actorAddr = actorIterator.getNext())
    {
        Actor *actor = *actorAddr;
        if (actor->followingPath && actor->pathStepsTaken < actor->pathLength)
        {
            actor->commandInQueue = getSectorDir(actor->gameObject.entity.pos, actor->path[actor->pathLength - 1 - actor->pathStepsTaken]);
        }
        if (actor->commandInQueue)
        {
            executeCommand(actor->commandInQueue, actor);
        }
        if (actor->pathStepsTaken == actor->pathLength)
        {
            actor->clearPath();
        }
    }
    static bool viewMemory = false;
    if (isButtonState(currentGame->button1id, BUTTON_STATE_RELEASED, currentGame->testBox))
        viewMemory = !viewMemory;
    if (viewMemory)
    {
        renderer->submitText(jadel::Vec2(-15.5f, 7.0f), 3, g_currentFont, currentGame->uiLayer, "Total Memory Allocation in bytes: %d", jadel::memoryGetTotalAllocationSize());
        renderer->submitText(jadel::Vec2(-15.5f, 6.5f), 3, g_currentFont, currentGame->uiLayer, "Reserved blocks: %d", jadel::memoryGetNumAllocatedBlocks());
        renderer->submitText(jadel::Vec2(-15.5f, 6.0f), 3, g_currentFont, currentGame->uiLayer, "Reserved bytes: %d", jadel::memoryGetNumAllocatedBytes());
        renderer->submitText(jadel::Vec2(-15.5f, 5.5f), 3, g_currentFont, currentGame->uiLayer, "Available bytes: %d", jadel::memoryGetFreeBytes());
    }
    jadel::Point2i nextScreenPos =
        {
            .x = currentGame->player.gameObject.entity.pos.x - screenTilemapW / 2,
            .y = currentGame->player.gameObject.entity.pos.y - screenTilemapH / 2};
    if (currentGame->updateCamera)
    {
        currentGame->screenPos = nextScreenPos;
    }
 
    renderGame();
    renderer->render();
}