#include "world.h"
#include "game.h"
#include "actor.h"

bool inBounds(int x, int y, const World *world)
{
    bool result(x >= 0 && y >= 0 &&
                x < world->width && y < world->height);
    return result;
}

void pushGameObject(GameObject gameObject, World *world)
{
    if (currentGame->numGameObjects < MAX_GAMEOBJECTS)
    {
        uint32 index = currentGame->numGameObjects++;
        currentGame->gameObjects[index] = gameObject;

        if (world->gameObjects.size < MAX_GAMEOBJECTS)
        {
            GameObject *gameObject = &currentGame->gameObjects[index];
            world->gameObjects.push(gameObject);
            // world->gameObjects[world->numGameObjects++] = &currentGame->gameObjects[index];
        }
    }
}

void pushGameObject(int x, int y, AnimFrames frames, const char *name, World *world)
{
    pushGameObject(createGameObject(x, y, frames, name), world);
}

void pushItem(Item item, World *world)
{
    if (currentGame->numItems < MAX_GAMEOBJECTS)
    {
        uint32 index = currentGame->numItems++;
        currentGame->items[index] = item;

        if (world->items.size < MAX_GAMEOBJECTS)
        {
            Item *item = &currentGame->items[index];
            world->items.push(item);
        }
    }
}

void pushActor(Actor actor, World *world)
{
    if (currentGame->numActors < MAX_ACTORS)
    {
        uint32 index = currentGame->numActors++;
        currentGame->actors[index] = actor;

        if (world->actors.size < MAX_ACTORS)
        {
            Actor *actor = &currentGame->actors[index];
            world->actors.push(actor);
        }
    }
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
        .sprite = currentGame->assets.getSurface("res/portal.png"),
        .linkID = numPortalIDs,
        .worldLinkID = world1ID,
        .sector = sector0};

    world1->portals[world1->numPortals] = {
        .sprite = currentGame->assets.getSurface("res/portal.png"),
        .linkID = numPortalIDs,
        .worldLinkID = world0ID,
        .sector = sector1};
    ++numPortalIDs;

    sector0->portal = &world0->portals[world0->numPortals++];
    sector1->portal = &world1->portals[world1->numPortals++];
    return true;
}

bool initWorld(int width, int height, World *world)
{
    if (!world)
        return false;

    jadel::vectorInit<Actor *>(MAX_ACTORS, &world->actors);
    jadel::vectorInit<Item *>(MAX_GAMEOBJECTS, &world->items);
    jadel::vectorInit<GameObject *>(MAX_GAMEOBJECTS, &world->gameObjects);

    world->entity = createEntity(0, 0);
    world->width = width;
    world->height = height;
    world->sectors = (Sector *)malloc(width * height * sizeof(Sector));
    world->numPortals = 0;
    world->pathNodes = (AStarNode *)malloc(width * height * sizeof(AStarNode));
    world->numCalculatedPathNodes = 0;
    world->calculatedPathNodes = (AStarNode **)malloc(width * height * sizeof(AStarNode *));
    currentGame->currentWorld = world;
    jadel::graphicsCreateSurface(width * currentGame->tileScreenW, height * currentGame->tileScreenH, &world->worldSurface);
    jadel::graphicsPushTargetSurface(&world->worldSurface);
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
            jadel::Recti sectorPos = getSectorScreenPos(x, y);

            jadel::graphicsBlit(sectorSprite, sectorPos);
        }
    }

    jadel::graphicsPopTargetSurface();

    return true;
}