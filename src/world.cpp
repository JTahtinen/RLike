#include "world.h"
#include "game.h"
#include "actor.h"
#include "globals.h"

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

void calculateLights(World *world)
{
    jadel::Node<Item *> *currentLightNode = world->lights.head;
    for (int i = 0; i < world->width * world->height; ++i)
    {
        Sector *sector = &world->sectors[i];
        sector->illumination = jadel::Vec3(0, 0, 0);
    }
    while (currentLightNode)
    {
        Item *item = currentLightNode->data;
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
            jadel::Vec3 illumination(lightIntensity.x / sectorIntensity,lightIntensity.y / sectorIntensity,lightIntensity.z / sectorIntensity);
            illuminateSector->illumination += illumination;
        }
        currentLightNode = currentLightNode->next;
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

void addSectorItem(Sector* sector, Item *item, World *world)
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

void removeSectorItem(Sector* sector, Item *item, World *world)
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
    target->illumination = jadel::Vec3(0, 0 ,0);
}

bool initWorld(int width, int height, World *world)
{
    if (!world)
        return false;
    world->actors.head = NULL;
    world->items.head = NULL;
    world->gameObjects.head = NULL;
    world->lights.head = NULL;
    world->entity = createEntity(0, 0);
    world->width = width;
    world->height = height;
    world->sectors = (Sector *)malloc(width * height * sizeof(Sector));
    world->numPortals = 0;
    world->pathNodes = (AStarNode *)malloc(width * height * sizeof(AStarNode));
    world->numCalculatedPathNodes = 0;
    world->calculatedPathNodes = (AStarNode **)malloc(width * height * sizeof(AStarNode *));
    currentGame->currentWorld = world;
    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            Sector *currentSector = &world->sectors[x + y * width];
            //  if (x % 3 == 0 && y % 3 == 0)
            //      initSector(x, y, &currentGame->wallTile, currentSector);
            //  else
            initSector(x, y, &currentGame->walkTile, currentSector);
            AStarNode *node = &world->pathNodes[x + y * width];
            node->pos = {x, y};
            node->sector = getSectorFromPos(x, y);
            const jadel::Surface *sectorSprite = currentSector->tile->surface;
        }
    }

    return true;
}