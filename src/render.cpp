#include "render.h"
#include <jadel.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

RenderLayer gameLayer;
RenderLayer uiLayer;

static jadel::Surface workingBuffer = {0};
static jadel::Surface workingTileSurface = {0};

static int xStart;
static int yStart;
static int xEnd;
static int yEnd;

static jadel::Recti worldScreenEndDim;
static jadel::Recti worldScreenDim;

static jadel::Mat3 viewMatrix(1.0f / 16.0f, 0.0f, 0.0f,
                              0.0f, 1.0f / 9.0f, 0.0f,
                              0.0f, 0.0f, 1.0f);

static jadel::Surface inventorySurface;

static jadel::Color idleHeaderColor = {0.6f, 0.2f, 0.3f, 0.8f};
static jadel::Color hoverHeaderColor = {0.6f, 0.3f, 0.4f, 1.0f};
static jadel::Color hookedHeaderColor = {0.6f, 0.8f, 0.6f, 1.0f};

struct InventoryRenderable
{
    float margin;
    jadel::Vec2 dimensions;
    jadel::Vec2 inventoryStart;
    jadel::Vec2 inventoryEnd;
    jadel::Vec2 innerStart;
    jadel::Vec2 innerEnd;
    jadel::Vec2 inventoryRadius;
    jadel::Vec2 inventoryCenter;
    jadel::Vec2 headerStart;
    jadel::Vec2 headerEnd;
    jadel::Color headerColor;
    uint32 targetOpeningTimeMS;
    uint32 elapsedTimeMS;
    Timer inventoryOpenTimer;
    bool inventoryOpened;
};

static InventoryRenderable inRenderable;

void setInventoryPos(jadel::Vec2 pos, InventoryRenderable *renderable)
{
    renderable->inventoryStart = pos;
    renderable->inventoryEnd = pos + renderable->dimensions;
    renderable->innerStart = jadel::Vec2(pos.x + 0.2f, pos.y - 0.2f);
    renderable->innerEnd = jadel::Vec2(renderable->inventoryEnd.x - 0.2f, renderable->inventoryEnd.y - 0.2f);
    renderable->inventoryCenter = pos + renderable->inventoryRadius;
    renderable->headerStart = jadel::Vec2(renderable->inventoryStart.x, renderable->inventoryEnd.y);
    renderable->headerEnd = jadel::Vec2(renderable->inventoryEnd.x, renderable->inventoryEnd.y + 0.5f);    
}

InventoryRenderable createInventoryRenderable(jadel::Vec2 pos, jadel::Vec2 dimensions)
{
    InventoryRenderable renderable;
    renderable.dimensions = dimensions;
    renderable.inventoryRadius = jadel::Vec2(dimensions.x * 0.5f, dimensions.y * 0.5f);
    setInventoryPos(pos, &renderable);
    renderable.margin = 0.08f;

    renderable.targetOpeningTimeMS = 1000;
    renderable.elapsedTimeMS = 0;
    renderable.inventoryOpened = false;
    renderable.headerColor = idleHeaderColor;
    return renderable;
}

void pushRenderable(const jadel::Surface *renderable, RenderLayer *layer)
{
    layer->surfaces.push(renderable);
}

void pushRenderable(RectfRenderable renderable, RenderLayer *layer)
{
    layer->rects.push(renderable);
}

void renderSurface(const jadel::Surface *surface, jadel::Vec2 start, jadel::Vec2 end)
{
    jadel::Vec2 screenStart = viewMatrix.mul(start);
    jadel::Vec2 screenEnd = viewMatrix.mul(end);
    jadel::graphicsBlitRelative(surface, jadel::Rectf{screenStart.x, screenStart.y, screenEnd.x, screenEnd.y});
}

void renderRect(float a, float r, float g, float b, jadel::Vec2 start, jadel::Vec2 end)
{
    jadel::Vec2 screenStart = viewMatrix.mul(start);
    jadel::Vec2 screenEnd = viewMatrix.mul(end);
    jadel::graphicsDrawRectRelative(jadel::Rectf{screenStart.x, screenStart.y, screenEnd.x, screenEnd.y},
                                    jadel::Color{a, r, g, b});
}

bool load_PNG(const char *filename, jadel::Surface *target)
{
    int width;
    int height;
    int channels;
    target->pixels = stbi_load(filename, &width, &height, &channels, 0);
    if (!target->pixels)
        return false;
    for (int i = 0; i < width * height; ++i)
    {
        uint8 *pixel = (uint8 *)target->pixels + (channels * i);
        jadel::flipBytes(pixel, 3);
    }
    target->width = width;
    target->height = height;
    return true;
}

bool initRender(jadel::Window *window)
{
    stbi_set_flip_vertically_on_load(true);
    if (!jadel::graphicsCreateSurface(window->width, window->height, &workingBuffer))
        return false;
    if (!jadel::graphicsCreateSurface(256, 256, &workingTileSurface))
        return false;
    jadel::vectorInit<const jadel::Surface *>(50, &gameLayer.surfaces);
    jadel::vectorInit<RectfRenderable>(50, &gameLayer.rects);
    jadel::vectorInit<const jadel::Surface *>(50, &uiLayer.surfaces);
    jadel::vectorInit<RectfRenderable>(50, &uiLayer.rects);
    jadel::graphicsPushTargetSurface(&workingBuffer);
    jadel::graphicsSetClearColor(0);
    jadel::graphicsClearTargetSurface();
    jadel::graphicsPopTargetSurface();
    load_PNG("res/inventory3.png", &inventorySurface);

    inRenderable = createInventoryRenderable(jadel::Vec2(-15.0f, -8.0f), jadel::Vec2(15.0f, 16.0f));

    return true;
}

static jadel::Recti getSectorDimensions(int x, int y)
{
    jadel::Recti sectorPos = getSectorScreenPos(x, y);
    jadel::Recti result = {.x0 = sectorPos.x0,
                           .y0 = sectorPos.y0,
                           .x1 = currentGame->tileScreenW,
                           .y1 = currentGame->tileScreenH};
    return result;
}

static void renderInventory(InventoryRenderable *renderable)
{
    Inventory *inventory = &currentGame->player.inventory;

    if (inventory->opening)
    {
        renderable->inventoryOpened = false;
        renderable->inventoryOpenTimer.start();
        inventory->opening = false;
    }

    if (!renderable->inventoryOpened)
    {
        renderable->elapsedTimeMS += renderable->inventoryOpenTimer.getMillisSinceLastUpdate();

        float currentRadiusMod = {(float)renderable->elapsedTimeMS / (float)renderable->targetOpeningTimeMS};
        if (currentRadiusMod > 1.0f)
            currentRadiusMod = 1.0f;
        jadel::Vec2 currentRadius(currentRadiusMod * renderable->inventoryRadius.x, currentRadiusMod * renderable->inventoryRadius.y);
        jadel::Vec2 openingStart = renderable->inventoryCenter - currentRadius;
        jadel::Vec2 openingEnd = renderable->inventoryCenter + currentRadius;

        renderSurface(&inventorySurface, openingStart, openingEnd);

        if (renderable->elapsedTimeMS >= renderable->targetOpeningTimeMS)
        {
            renderable->elapsedTimeMS = 0;
            renderable->inventoryOpened = true;
        }

        return;
    }

    static const float SpriteSize = 1.5f;
    renderSurface(&inventorySurface, renderable->inventoryStart, renderable->inventoryEnd);
    renderRect(renderable->headerColor.a, renderable->headerColor.r, renderable->headerColor.g, renderable->headerColor.b, renderable->headerStart, renderable->headerEnd);
    float itemY = -1.0f;
    for (int i = 0; i < 10; ++i)
    {
        jadel::Vec2 itemPos(renderable->innerStart.x + 0.2f, renderable->innerEnd.y + SpriteSize * itemY - 0.2f); // - SpriteSize * itemY));
        jadel::Vec2 itemWH(SpriteSize, SpriteSize);
        jadel::Rectf itemDim = {itemPos.x, itemPos.y,
                                itemPos.x + itemWH.x, itemPos.y + itemWH.y};
        if (!inventory->itemSlots[i].hasItem)
            continue;
        Item *item = inventory->itemSlots[i].item;
        const jadel::Surface *sprite = NULL;
        if (item->gameObject.frames.numFrames > 0)
            sprite = item->gameObject.frames.sprites[item->gameObject.frames.currentFrameIndex];

        jadel::Color itemColor;
        if (item->gameObject.entity.id == getPlayerWeaponID())
            itemColor = jadel::Color{1.0f, 1.0f, 0.2f, 1.0f};
        else
            itemColor = jadel::Color{1.0f, 0.45f, 0.4f, 1.0f};
        
        renderRect(itemColor.a, itemColor.r, itemColor.g, itemColor.b, itemPos, itemPos + itemWH);
        if (sprite)
            renderSurface(sprite, itemPos, itemPos + itemWH);
        itemY -= 1.2f;
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
            jadel::Recti healthBarDim{entityDim.x0, entityDim.y0 + entityDim.y1 + 10 - 10, entityDim.x1, 10};
            jadel::Recti healthRemainingDim{healthBarDim.x0, healthBarDim.y0,
                                            (int)((float)healthBarDim.x1 / (float)maxHealth * (float)health), healthBarDim.y1};
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
                sectorSprite = currentGame->assets.getSurface("res/portal.png");
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
                spriteToDraw = currentGame->assets.getSurface("res/clutter.png");
                ;
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
        if (worldScreenEndDim.x0 > currentGame->window->width)
            worldScreenEndDim.x0 = currentGame->window->width;
        if (worldScreenEndDim.y0 > currentGame->window->height)
            worldScreenEndDim.y0 = currentGame->window->height;

        worldScreenDim = getSectorScreenPos(0, 0);

        if (worldScreenDim.x0 < 0)
            worldScreenDim.x0 = 0;
        if (worldScreenDim.y0 < 0)
            worldScreenDim.y0 = 0;

        worldScreenDim.x1 = worldScreenEndDim.x0 - worldScreenDim.x0;
        worldScreenDim.y1 = worldScreenEndDim.y0 - worldScreenDim.y0;

        renderTiles();
        renderBars();
        if (currentGame->player.equippedWeapon)
        {
            static jadel::Vec2 weaponStart(13.0f, 6.0f);
            static jadel::Vec2 weaponEnd(16.0f, 9.0f);

            static jadel::Vec2 wScreenStart = viewMatrix.mul(weaponStart);
            static jadel::Vec2 wScreenEnd = viewMatrix.mul(weaponEnd);

            jadel::graphicsBlitRelative(currentGame->player.equippedWeapon->gameObject.frames.sprites[0],
                                        {wScreenStart.x, wScreenStart.y, wScreenEnd.x, wScreenEnd.y});
        }
        if (currentGame->currentState == SUBSTATE_INVENTORY)
        {
            static bool hooked = false;
            jadel::Vec2 inPos = inRenderable.inventoryStart;
            jadel::Vec2 headerStart = inRenderable.headerStart;
            jadel::Vec2 headerEnd = inRenderable.headerEnd;

            jadel::Vec2 mouseScreenPos = jadel::inputGetMouseRelative();

            // TODO: multiply by matrix
            mouseScreenPos.x *= 16.0f;
            mouseScreenPos.y *= 9.0f;
            static jadel::Vec2 hookPosition;
            if (mouseScreenPos.x >= headerStart.x && mouseScreenPos.x < headerEnd.x && mouseScreenPos.y >= headerStart.y && mouseScreenPos.y < headerEnd.y)
            {
                inRenderable.headerColor = hoverHeaderColor;
                if (!hooked && jadel::inputLButtonDown)
                {
                    hooked = true;
                    jadel::message("Hooked\n");
                    hookPosition = mouseScreenPos - inPos;
                }
            }
            else inRenderable.headerColor = idleHeaderColor;
            if (hooked)
            {
                inRenderable.headerColor = hookedHeaderColor;
                inPos = mouseScreenPos - hookPosition;
            }

            if (!jadel::inputLButtonDown)
                hooked = false;

            setInventoryPos(inPos, &inRenderable);
            renderInventory(&inRenderable);
        }
    }

    jadel::graphicsPopTargetSurface();
}

jadel::Surface *getScreenBuffer()
{
    return &workingBuffer;
}