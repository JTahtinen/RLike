#include "world.h"
#include "game.h"
#include "actor.h"
#include "globals.h"
#include "util.h"
#include <string.h>

bool inBounds(int x, int y, const World *world)
{
    bool result(x >= 0 && y >= 0 &&
                x < world->width && y < world->height);
    return result;
}

int distanceBetweenSectors(const Sector *a, const Sector *b)
{
    jadel::Point2i posA = a->pos;
    jadel::Point2i posB = b->pos;

    if (posA.x == posB.x && posA.y == posB.y)
        return 0;

    int xDiff = jadel::absInt(posB.x - posA.x);
    int yDiff = jadel::absInt(posB.y - posA.y);

    int higherDiff = xDiff > yDiff ? xDiff : yDiff;
    int lowerDiff = xDiff > yDiff ? yDiff : xDiff;

    int result = lowerDiff * 14 + (higherDiff - lowerDiff) * 10;
    return result;
}

void pushGameObject(GameObject gameObject, World *world)
{
    GameObject *reservedGameObject = currentGame->gameObjectFactory.get();
    if (!reservedGameObject)
    {
        return;
    }
    *reservedGameObject = gameObject;

    world->gameObjects.append(reservedGameObject);
}

void pushGameObject(int x, int y, AnimFrames frames, const char *name, World *world)
{
    pushGameObject(createGameObject(x, y, frames, name), world);
}

void pushItem(Item item, World *world)
{
    if (!world)
    {
        return;
    }
    Item *reservedItem = currentGame->itemFactory.get();
    if (!reservedItem)
    {
        return;
    }
    *reservedItem = item;

    // world->items.append(reservedItem);
    /*if (reservedItem->flags & ITEM_EFFECT_ILLUMINATE)
    {
        world->lights.append(reservedItem);
    }*/
    jadel::Point2i itemPos = item.gameObject.entity.pos;
    Sector *targetSector = getSectorFromPos(itemPos, world);
    addSectorItem(targetSector, reservedItem, world);
}

void pushActor(Actor actor, World *world)
{
    Actor *reservedActor = currentGame->actorFactory.get();
    if (!reservedActor)
    {
        return;
    }
    *reservedActor = actor;

    world->actors.append(reservedActor);
}

void pushActor(int x, int y, AnimFrames frames, const char *name, World *world)
{
    pushActor(createActor(x, y, frames, name), world);
}

Sector *getSectorFromPos(int x, int y, World *world)
{
    Sector *result = NULL;
    if (inBounds(x, y, world))
    {
        result = &world->sectors[x + y * world->width];
    }
    return result;
}

Sector *getSectorFromPos(int x, int y)
{
    Sector *result = getSectorFromPos(x, y, currentGame->currentWorld);
    return result;
}

Sector *getSectorFromPos(jadel::Point2i pos, World *world)
{
    Sector *result = getSectorFromPos(pos.x, pos.y, world);
    return result;
}

Sector *getSectorFromPos(jadel::Point2i pos)
{
    Sector *result = getSectorFromPos(pos, currentGame->currentWorld);
    return result;
}

jadel::Vector<Sector*> getSurroudingSectors(int x, int y, World* world)
{   
    jadel::Vector<Sector*> result;
    jadel::vectorInit(8, &result);
    for (int yy = 0; yy < 3; ++yy)
    {
        for (int xx = 0; xx < 3; ++xx)
        {
            if (xx == 1 && yy == 1) continue;

            Sector* sector = getSectorFromPos(x - 1 + xx, y - 1 + yy);
            if (sector)
            {
                result.push(sector);
            }
        }
    }
    return result;
}

void calculateLights(World *world)
{
    for (int i = 0; i < world->width * world->height; ++i)
    {
        Sector *sector = &world->sectors[i];
        sector->illumination = jadel::Vec3(0, 0, 0);
    }
    LinkedListIterator lightIterator(&world->lights);
    Item **itemAddr;
    while (itemAddr = lightIterator.getNext())
    {
        Item *item = *itemAddr;
        jadel::Vec3 lightIntensity = item->illumination;
        Sector *currentSector = getSectorFromPos(item->gameObject.entity.pos, world);
        for (int i = 0; i < world->width * world->height; ++i)
        {
            Sector *illuminateSector = &world->sectors[i];
            jadel::Point2i sectorPos = illuminateSector->pos;
            int dist;
            if (illuminateSector == currentSector)
            {
                dist = item->distanceFromGround;
            }
            else
            {
                dist = distanceBetweenSectors(currentSector, illuminateSector) / 3;
                dist = (int)sqrtf((float)dist * (float)dist + (float)item->distanceFromGround * (float)item->distanceFromGround);
            }
            float sectorIntensity = (float)(dist * dist);
            jadel::Vec3 illumination(lightIntensity.x / sectorIntensity, lightIntensity.y / sectorIntensity, lightIntensity.z / sectorIntensity);
            illuminateSector->illumination += illumination;
        }
    }
}

bool setPortal(int x0, int y0, uint32 world0ID, int x1, int y1, uint32 world1ID)
{
    World *world0 = getWorldByID(world0ID);
    World *world1 = getWorldByID(world1ID);
    if (!world0 || !world1)
        return false;
    if (!inBounds(x0, y0, world0) || !inBounds(x1, y1, world1))
        return false;

    Sector *sector0 = getSectorFromPos(x0, y0, world0);
    Sector *sector1 = getSectorFromPos(x1, y1, world1);

    world0->portals[world0->numPortals] = {
        .sprite = g_Assets.getSurface("res/portal.png"),
        .linkID = numPortalIDs,
        .worldLinkID = world1ID,
        .sector = sector0};

    world1->portals[world1->numPortals] = {
        .sprite = g_Assets.getSurface("res/portal.png"),
        .linkID = numPortalIDs,
        .worldLinkID = world0ID,
        .sector = sector1};
    ++numPortalIDs;

    sector0->portal = &world0->portals[world0->numPortals++];
    sector1->portal = &world1->portals[world1->numPortals++];
    return true;
}

void addSectorItem(Sector *sector, Item *item, World *world)
{
    if (!sector || !item || !world)
    {
        return;
    }
    sector->items.append(item);
    // TODO: Something else
    world->items.append(item);
    item->gameObject.entity.pos = sector->pos;
    ++sector->numItems;
    if (item->flags & ITEM_EFFECT_ILLUMINATE)
    {
        world->lights.append(item);
        calculateLights(world);
        currentGame->updateCamera = true;
    }
}

void addSectorItem(int x, int y, Item *item, World *world)
{
    addSectorItem(getSectorFromPos(x, y, world), item, world);
}

void removeSectorItem(Sector *sector, Item *item, World *world)
{
    if (!sector || !item || !world)
    {
        return;
    }
    sector->items.deleteWithValue(item);
    world->items.deleteWithValue(item);
    --sector->numItems;
    if (item->flags & ITEM_EFFECT_ILLUMINATE)
    {
        world->lights.deleteWithValue(item);
        calculateLights(world);
    }
}

void removeSectorItem(int x, int y, Item *item, World *world)
{
    removeSectorItem(getSectorFromPos(x, y, world), item, world);
}

void initSector(int x, int y, const Tile *tile, Sector *target)
{
    target->pos = {x, y};
    target->numItems = 0;
    target->tile = tile;
    target->occupant = NULL;
    target->items.head = NULL;
    target->portal = NULL;
    target->illumination = jadel::Vec3(0, 0, 0);
    target->ignitionTreshold = 0.5f;
    target->temperature = 0;
    target->onFire = false;
    target->flammable = tile->flammable;
}

bool initWorld(int width, int height, Sector *sectorMap, const jadel::Vector<Tile>& tiles, World *world)
{
    if (!world || !sectorMap)
        return false;
    world->sectors = sectorMap;
    world->actors.head = NULL;
    world->items.head = NULL;
    world->gameObjects.head = NULL;
    world->lights.head = NULL;
    world->entity = createEntity(0, 0);
    world->width = width;
    world->height = height;
    world->tiles = tiles;
    world->numPortals = 0;
    world->pathNodes = (AStarNode *)jadel::memoryReserve(width * height * sizeof(AStarNode));
    world->numCalculatedPathNodes = 0;
    world->calculatedPathNodes = (AStarNode **)jadel::memoryReserve(width * height * sizeof(AStarNode *));
    currentGame->currentWorld = world;

    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            AStarNode *node = &world->pathNodes[x + y * width];
            node->pos = {x, y};
            node->sector = getSectorFromPos(x, y);
        }
    }
    return true;
}

bool initWorld(int width, int height, World *world)
{
    Sector *sectors = (Sector *)jadel::memoryReserve(width * height * sizeof(Sector));
    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            Sector *currentSector = &sectors[x + y * width];
            // if (x % 4 == 0 && y % 4 == 0)
            //     initSector(x, y, &currentGame->wallTile, currentSector);
            //   else
            initSector(x, y, &currentGame->walkTile, currentSector);

        }
    }
    jadel::Vector<Tile> tiles;
    jadel::vectorInit(10, &tiles);
    bool result = initWorld(width, height, sectors, tiles, world);
    if (!result)
    {
        jadel::memoryFree(sectors);
        tiles.freeVector();
    }
    return result;
}

bool loadWorld(const char *filepath, World *target)
{
    char *file = NULL;
    size_t numCharacters;
    int width;
    int height;
    const char *errorMessage = "[ERROR] Trying to load world file";
    if (!target)
    {
        jadel::message("%s%s: world pointer was null!\n", errorMessage, filepath);
        return false;
    }
    int tileIDs[10];
    jadel::Vector<Tile> tileTypes;
    jadel::vectorInit(10, &tileTypes);
    Sector *sectorMap;
    int numTilesExpected = 0;
    if (!jadel::readTextFileAndReserveMemory(filepath, &file, &numCharacters))
    {
        jadel::message("%s%s: file not found!\n", errorMessage, filepath);
        return false;
    }

    char *token;
    while (token = strtok_s(file, ": \n", &file))
    {
        if (strcmp(token, "width") == 0)
        {
            token = strtok_s(file, ": \n", &file);
            width = atoi(token);
            continue;
        }
        else if (strcmp(token, "height") == 0)
        {
            token = strtok_s(file, ": \n", &file);
            height = atoi(token);
            continue;
        }
        else if (strcmp(token, "tiles") == 0)
        {
            token = strtok_s(file, ": \n", &file);
            if (strcmp(token, "count") == 0)
            {
                token = strtok_s(file, ": \n", &file);
                int numTilesLoaded = 0;
                numTilesExpected = atoi(token);
                for (; numTilesLoaded < numTilesExpected; ++numTilesLoaded)
                {
                    bool surfaceFound = false;
                    bool barrierFound = false;
                    bool flammableFound = false;
                    Tile tile;
                    token = strtok_s(file, ": \n", &file);
                    int id = atoi(token);
                    if (id == 0)
                    {
                        jadel::message("%s%s: invalid tile ID\n", errorMessage, filepath);
                        return false;
                    }
                    tileIDs[numTilesLoaded] = id;
                    while (!surfaceFound || !barrierFound || !flammableFound)
                    {
                        token = strtok_s(file, ": \n", &file);
                        if (strcmp(token, "surface") == 0)
                        {
                            if (surfaceFound)
                            {
                                jadel::message("%s%s: duplicate surface in tile definition\n", errorMessage, filepath);
                                return false;
                            }
                            token = strtok_s(file, ": \n", &file);
                            tile.surface = g_Assets.getSurface(token);
                            surfaceFound = true;
                            continue;
                        }
                        else if (strcmp(token, "barrier") == 0)
                        {
                            if (barrierFound)
                            {
                                jadel::message("%s%s: duplicate barrier in tile definition\n", errorMessage, filepath);
                                return false;
                            }
                            token = strtok_s(file, ": \n", &file);
                            if (strcmp(token, "true") == 0 || strcmp(token, "1") == 0)
                            {
                                tile.barrier = true;
                                barrierFound = true;
                                continue;
                            }
                            else if (strcmp(token, "false") == 0 || strcmp(token, "0") == 0)
                            {
                                tile.barrier = false;
                                barrierFound = true;
                                continue;
                            }
                        }
                        else if (strcmp(token, "flammable") == 0)
                        {
                            if (flammableFound)
                            {
                                jadel::message("%s%s: duplicate flammable in tile definition\n", errorMessage, filepath);
                                return false;
                            }
                            token = strtok_s(file, ": \n", &file);
                            if (strcmp(token, "true") == 0 || strcmp(token, "1") == 0)
                            {
                                tile.flammable = true;
                                flammableFound = true;
                                continue;
                            }
                            else if (strcmp(token, "false") == 0 || strcmp(token, "0") == 0)
                            {
                                tile.flammable = false;
                                flammableFound = true;
                                continue;
                            }
                        }
                    }
                    tileTypes.push(tile);
                }
                if (numTilesLoaded != numTilesExpected)
                {
                    jadel::message("%s%s: expected %d tile types, received %d\n", errorMessage, filepath, numTilesExpected, numTilesLoaded);
                    return false;
                }
            }
        }
        else if (strcmp(token, "map") == 0)
        {
            sectorMap = (Sector *)jadel::memoryReserve(width * height * sizeof(Sector));
            if (!sectorMap)
            {
                jadel::message("%s%s: could not reserve memory for sector map!\n", errorMessage, filepath);
                return false;
            }

            for (int y = 0; y < height; ++y)
            {
                token = strtok_s(file, ": \n", &file);
                for (int x = 0; x < width; ++x)
                {
                    Tile* tileType;
                    char idChar[2];
                    idChar[0] = token[x];
                    idChar[1] = '\0';
                    int tileTypeID = atoi(idChar);
                    if (tileTypeID != 0)
                    {
                        bool tileTypeFound = false;
                        for (int j = 0; j < numTilesExpected; ++j)
                        {
                            if (tileIDs[j] == tileTypeID)
                            {
                                Sector sector;
                                tileType = &tileTypes[j];
                                if (tileTypes[j].surface == NULL) return false;
                                tileTypeFound = true;
                                initSector(x, y, tileType, &sector);
                                sectorMap[x + y * width] = sector;
                                break;
                            }
                        }
                        if (!tileTypeFound)
                        {
                            jadel::message("%s%s: invalid tile ID in map!\n", errorMessage, filepath);
                            jadel::memoryFree(sectorMap);
                            tileTypes.freeVector();
                            return false;
                        }
                    }
                }
            }
        }
    }

    bool result = initWorld(width, height, sectorMap, tileTypes, target);
    return result;
}