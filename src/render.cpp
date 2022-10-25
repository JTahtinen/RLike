#include "render.h"
#include <jadel.h>

static jadel::Surface workingBuffer = {0};
static jadel::Surface workingTileSurface = {0};

static int xStart;
static int yStart;
static int xEnd;
static int yEnd;

static Timer inventoryOpenTimer;
static jadel::Recti worldScreenEndDim;
static jadel::Recti worldScreenDim;

static bool inventoryOpened = false;

bool initRender(jadel::Window *window)
{
    if (!jadel::graphicsCreateSurface(window->width, window->height, &workingBuffer))
        return false;
    if (!jadel::graphicsCreateSurface(256, 256, &workingTileSurface))
        return false;
    jadel::graphicsPushTargetSurface(&workingBuffer);
    jadel::graphicsSetClearColor(0);
    jadel::graphicsClearTargetSurface();
    jadel::graphicsPopTargetSurface();

    return true;
}

static jadel::Recti getSectorDimensions(int x, int y)
{
    jadel::Recti sectorPos = getSectorScreenPos(x, y);
    jadel::Recti result = {.x = sectorPos.x,
                           .y = sectorPos.y,
                           .w = currentGame->tileScreenW,
                           .h = currentGame->tileScreenH};
    return result;
}

static void renderInventory()
{
    Inventory *inventory = &currentGame->player.inventory;
    static int margin = 5;
    static jadel::Recti dimensions = {20, 20,
                               workingBuffer.width / 2 - 20,
                               workingBuffer.height - 40};
    static jadel::Recti innerDimensions = 
    {dimensions.x + margin, dimensions.y + margin,
    dimensions.w - 2 * margin, dimensions.h - 2 * margin};

    static float inventoryXRadius = (float)dimensions.w * 0.5f;
    static float inventoryYRadius = (float)dimensions.h * 0.5f;
    //jadel::Point2i inventoryRadius = {dimensions.w / 2, dimensions.h / 2};
    static float inventoryXCenter = (float)dimensions.x + inventoryXRadius;
    static float inventoryYCenter = (float)dimensions.y + inventoryYRadius;
    
    //jadel::Point2i inventoryCenter =
    //    {dimensions.x + inventoryRadius.x, dimensions.y + inventoryRadius.y};
    static uint32 targetOpeningTimeMS = 1000;
    static uint32 elapsedTimeMS = 0;
    if (inventory->opening)
    {
        inventoryOpened = false;
        inventoryOpenTimer.start();
        inventory->opening = false;
    }
    if (!inventoryOpened)
    {
        elapsedTimeMS += inventoryOpenTimer.getMillisSinceLastUpdate();
        
        float currentRadiusMod = {(float)elapsedTimeMS / (float)targetOpeningTimeMS};
        if (currentRadiusMod > 1.0f) currentRadiusMod = 1.0f;
        
        float currentXRadius = currentRadiusMod * inventoryXRadius;
        float currentYRadius = currentRadiusMod * inventoryYRadius;
        //jadel::Point2i currentRadius = {jadel::roundToInt(currentRadiusMod * (float)inventoryRadius.x),
        //                                jadel::roundToInt(currentRadiusMod * (float)inventoryRadius.y)};
        jadel::Recti openingDim =
            {jadel::roundToInt(inventoryXCenter - currentXRadius),
             jadel::roundToInt(inventoryYCenter - currentYRadius),
             jadel::roundToInt(currentXRadius * 2.0f), 
             jadel::roundToInt(currentYRadius * 2.0f)};
        jadel::graphicsDrawRect(openingDim, 0xAA665555);
        if (elapsedTimeMS >= targetOpeningTimeMS)
        {
            elapsedTimeMS = 0;
            inventoryOpened = true;
        }

        return;
    }

    static int SpriteSize = 80;
    jadel::graphicsDrawRect(dimensions, 0xAA443333);
    jadel::graphicsDrawRect(innerDimensions, 0xAA665555);
    
    int itemY = dimensions.y + dimensions.h - SpriteSize - margin - 10;
    for (int i = 0; i < 10; ++i)
    {
        jadel::Recti itemDim = {dimensions.x + margin + 10, itemY, SpriteSize, SpriteSize};
        if (!inventory->itemSlots[i].hasItem)
            continue;
        Item *item = inventory->itemSlots[i].item;
        const jadel::Surface *sprite = NULL;
        if (item->gameObject.frames.numFrames > 0)
            sprite = item->gameObject.frames.sprites[0];
        jadel::graphicsDrawRect(itemDim, 0xff888888);
        if (sprite)
            jadel::graphicsBlitFast(sprite, itemDim);
        itemY -= SpriteSize + 10;
    }
}

static void renderBars()
{
    for (int y = yStart; y < yEnd; ++y)
    {
        for (int x = xStart; x < xEnd; ++x)
        {
            jadel::Recti entityDim = getSectorDimensions(x, y);
            Sector currentSector = currentGame->currentWorld->sectors[x + y * currentGame->currentWorld->width];
            if (!currentSector.occupant)
                continue;
            int health = currentSector.occupant->gameObject.health;
            int maxHealth = currentSector.occupant->gameObject.maxHealth;
            jadel::Recti healthBarDim{entityDim.x, entityDim.y + 60, 50, 10};
            jadel::Recti healthRemainingDim{healthBarDim.x, healthBarDim.y,
                                            (int)(50.0f / (float)maxHealth * (float)health), healthBarDim.h};
            jadel::graphicsDrawRect(healthBarDim, 0xff222222);
            jadel::graphicsDrawRect(healthRemainingDim, 0xffaa0000);
        }
    }
}

static void renderTiles()
{
    for (int y = yStart; y < yEnd; ++y)
    {
        for (int x = xStart; x < xEnd; ++x)
        {
            jadel::graphicsPushTargetSurface(&workingTileSurface);
            jadel::Recti entityDim = getSectorDimensions(x, y);

            Sector currentSector = currentGame->currentWorld->sectors[x + y * currentGame->currentWorld->width];

            const jadel::Surface *sectorSprite = NULL;
            if (currentSector.portal)
            {
                sectorSprite = &currentGame->portalSprite;
            }
            else
            {
                sectorSprite = currentSector.tile->surface;
            }

            if (sectorSprite)
            {
                jadel::graphicsCopyEqualSizeSurface(sectorSprite);
            }

            const jadel::Surface *spriteToDraw = NULL;

            /*if (currentSector.occupant && !currentSector.occupant->transit.inTransit)
            {
                AnimFrames* frames
                    = &currentSector.occupant->gameObject.frames;
                spriteToDraw = frames->sprites[frames->currentFrameIndex];
            }*/
            if (currentSector.occupant)
            {
                AnimFrames *frames = &currentSector.occupant->gameObject.frames;
                if (frames->numFrames > 0)
                    spriteToDraw = frames->sprites[frames->currentFrameIndex];
            }
            else if (currentSector.numItems == 1)
            {
                AnimFrames *frames = &currentSector.items[currentSector.numItems - 1]->gameObject.frames;
                if (frames->numFrames > 0)
                    spriteToDraw = frames->sprites[frames->currentFrameIndex];
            }
            else if (currentSector.numItems > 1)
            {
                spriteToDraw = &currentGame->clutterSprite;
            }
            if (spriteToDraw)
            {
                jadel::graphicsBlitFast(spriteToDraw, 0, 0, 255, 255);
            }
            jadel::graphicsMultiplyPixelValues(currentSector.illumination);
            jadel::graphicsPopTargetSurface();

            jadel::graphicsBlitFast(&workingTileSurface, entityDim);

            /*
            for (int i = 0; i < currentGame->currentWorld->numActors; ++i)
            {
                Actor *actor = currentGame->currentWorld->actors[i];
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
                            currentGame->tileScreenH};
                    jadel::graphicsBlit(
                        actor->gameObject.frames.sprites[actor->gameObject.frames.currentFrameIndex], currentPoint);
                }
            }*/
        }
        /*           for (int i = 0; i < actors[1]->pathLength; ++i)
                    {
                        jadel::Recti sectorPos = getSectorScreenPos(actors[1]->path[i]);
                        jadel::graphicsDrawRect(sectorPos, 0x88550000);
                    }*/
    }
}

void render()
{
    jadel::graphicsPushTargetSurface(&workingBuffer);
    jadel::graphicsSetClearColor(0);
    jadel::graphicsClearTargetSurface();
    if (currentGame->screenPos.x > -screenTilemapW && currentGame->screenPos.y > -screenTilemapH && currentGame->screenPos.x < currentGame->currentWorld->width && currentGame->screenPos.y < currentGame->currentWorld->height)
    {
        xStart = currentGame->screenPos.x < 0 ? 0 : currentGame->screenPos.x;
        yStart = currentGame->screenPos.y < 0 ? 0 : currentGame->screenPos.y;
        xEnd = currentGame->screenPos.x + screenTilemapW <= currentGame->currentWorld->width
                   ? (currentGame->screenPos.x + screenTilemapW)
                   : currentGame->currentWorld->width;
        yEnd = currentGame->screenPos.y + screenTilemapH <= currentGame->currentWorld->height
                   ? (currentGame->screenPos.y + screenTilemapH)
                   : currentGame->currentWorld->height;

        worldScreenEndDim = getSectorScreenPos(currentGame->currentWorld->width,
                                               currentGame->currentWorld->height);
        if (worldScreenEndDim.x > currentGame->window->width)
            worldScreenEndDim.x = currentGame->window->width;
        if (worldScreenEndDim.y > currentGame->window->height)
            worldScreenEndDim.y = currentGame->window->height;

        // Recti worldScreenEndDim = getSectorScreenPos(getCameraRight(currentGame->screenPos));

        worldScreenDim = getSectorScreenPos(0, 0);

        if (worldScreenDim.x < 0)
            worldScreenDim.x = 0;
        if (worldScreenDim.y < 0)
            worldScreenDim.y = 0;

        worldScreenDim.w = worldScreenEndDim.x - worldScreenDim.x;
        worldScreenDim.h = worldScreenEndDim.y - worldScreenDim.y;
        /*
                jadel::Recti worldStartPos =
                    {.x = currentGame->screenPos.x * currentGame->tileScreenW,
                     .y = currentGame->screenPos.y * currentGame->tileScreenH,
                     .w = worldScreenEndDim.x,  // screenTilemapW * currentGame->tileScreenW,
                     .h = worldScreenEndDim.y}; // screenTilemapH * currentGame->tileScreenH};

                jadel::graphicsBlit(&currentGame->currentWorld->worldSurface, worldStartPos, worldScreenDim);
        */
        renderTiles();
        renderBars();
        if (currentGame->currentState == SUBSTATE_INVENTORY)
            renderInventory();
        // jadel::graphicsDrawRect(entityDim, jadel::Color{1.0f - currentSector.illumination, 0, 0, 0});
    }

    jadel::graphicsPopTargetSurface();
}

jadel::Surface *getScreenBuffer()
{
    return &workingBuffer;
}