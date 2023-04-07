#include "rendergame.h"
#include <jadel.h>
#include "globals.h"
#include "timer.h"

Renderer *gameRenderer = NULL;

jadel::Surface _workingBuffer;
jadel::Surface _worldBuffer;
jadel::Surface _workingTileSurface;

static jadel::Mat3 viewMatrix(1.0f / 16.0f, 0.0f, 0.0f,
                              0.0f, 1.0f / 9.0f, 0.0f,
                              0.0f, 0.0f, 1.0f);


void renderWorld();
void renderTiles();
void renderGameObjects();
void renderBars();

static int _xStart;
static int _yStart;
static int _xEnd;
static int _yEnd;

static Timer fireTimer;
static size_t fireTimerInMillis;
bool drawFireOne;

static ScreenObject _worldScrObj;

static jadel::Recti getSectorDimensions(int x, int y)
{
    jadel::Recti sectorPos = getSectorScreenPos(x, y);
    jadel::Recti result = {.x0 = sectorPos.x0,
                           .y0 = sectorPos.y0,
                           .x1 = sectorPos.x0 + currentGame->tileScreenW,
                           .y1 = sectorPos.y0 + currentGame->tileScreenH};
    return result;
}

void renderGame()
{
    fireTimerInMillis += fireTimer.getMillisSinceLastUpdate();
    fireTimer.update();
    if (fireTimerInMillis >= 500)
    {
        fireTimerInMillis %= 500;
        drawFireOne = !drawFireOne;
    }
    if (!gameRenderer)
        return;
    jadel::graphicsPushTargetSurface(&_workingBuffer);
    renderWorld();
    if (currentGame->player.equippedWeapon)
    {
        static jadel::Vec2 weaponStart(13.0f, 6.0f);
        static jadel::Vec2 weaponEnd(16.0f, 9.0f);

        static jadel::Vec2 wScreenStart = viewMatrix.mul(weaponStart);
        static jadel::Vec2 wScreenEnd = viewMatrix.mul(weaponEnd);

        jadel::graphicsBlitRelative(currentGame->player.equippedWeapon->gameObject.frames.sprites[0],
                                    {wScreenStart.x, wScreenStart.y, wScreenEnd.x, wScreenEnd.y});
    }
    jadel::graphicsPopTargetSurface();
}

bool systemInitRenderGame(jadel::Window *window, Renderer *renderer)
{
    if (!window || !renderer)
        return false;
    if (!jadel::graphicsCreateSurface(window->width, window->height, &_worldBuffer))
        return false;
    if (!jadel::graphicsCreateSurface(window->width, window->height, &_workingBuffer))
        return false;
    if (!jadel::graphicsCreateSurface(256, 256, &_workingTileSurface))
        return false;
    gameRenderer = renderer;
    // initScreenObject(&_worldScrObj)
    fireTimer.start();
    drawFireOne = true;
    fireTimerInMillis = 0;
    return true;
}

void renderWorld()
{
    if (currentGame->screenPos.x > -screenTilemapW && currentGame->screenPos.y > -screenTilemapH && currentGame->screenPos.x < currentGame->currentWorld->width && currentGame->screenPos.y < currentGame->currentWorld->height)
    {
        _xStart = currentGame->screenPos.x < 0 ? 0 : currentGame->screenPos.x;
        _yStart = currentGame->screenPos.y < 0 ? 0 : currentGame->screenPos.y;
        _xEnd = currentGame->screenPos.x + screenTilemapW <= currentGame->currentWorld->width
                    ? (currentGame->screenPos.x + screenTilemapW)
                    : currentGame->currentWorld->width;
        _yEnd = currentGame->screenPos.y + screenTilemapH <= currentGame->currentWorld->height
                    ? (currentGame->screenPos.y + screenTilemapH)
                    : currentGame->currentWorld->height;
        //if (currentGame->updateCamera)
        //{
            renderTiles();
        //}
        jadel::graphicsCopyEqualSizeSurface(&_worldBuffer);
        renderGameObjects();
        renderBars();
        gameRenderer->submitRenderable(&_workingBuffer, jadel::Vec2(-16.0f, -9.0f), jadel::Vec2(32.0f, 18.0f), currentGame->gameLayer);
    }
}

void renderTiles()
{
    jadel::graphicsPushTargetSurface(&_worldBuffer);
    jadel::graphicsSetClearColor(0);
    jadel::graphicsClearTargetSurface();
    for (int y = _yStart; y < _yEnd; ++y)
    {
        for (int x = _xStart; x < _xEnd; ++x)
        {
            jadel::graphicsPushTargetSurface(&_workingTileSurface);
            jadel::Recti entityDim = getSectorDimensions(x, y);

            Sector currentSector = currentGame->currentWorld->sectors[x + y * currentGame->currentWorld->width];

            const jadel::Surface *sectorSprite = NULL;
            if (currentSector.portal)
            {
                sectorSprite = g_Assets.getSurface("res/portal.png");
            }
            else
            {
                sectorSprite = currentSector.tile->surface;
            }

            if (sectorSprite)
            {
                jadel::graphicsCopyEqualSizeSurface(sectorSprite);
            }


            jadel::Vec3 illumination = currentSector.illumination;
            jadel::graphicsMultiplyPixelValues(illumination.x, illumination.y, illumination.z);
            jadel::graphicsPopTargetSurface();
            jadel::graphicsBlit(&_workingTileSurface, entityDim);
            if (currentSector.onFire)
            {
                if (drawFireOne)
                    jadel::graphicsBlit(g_Assets.getSurface("res/fire.png"), entityDim);    
                else
                    jadel::graphicsBlit(g_Assets.getSurface("res/fire2.png"), entityDim);    
            }
        }
    }
    jadel::graphicsPopTargetSurface();
}

void renderGameObjects()
{
    for (int y = _yStart; y < _yEnd; ++y)
    {
        for (int x = _xStart; x < _xEnd; ++x)
        {
            jadel::graphicsPushTargetSurface(&_workingTileSurface);
            Sector currentSector = currentGame->currentWorld->sectors[x + y * currentGame->currentWorld->width];
            jadel::Recti entityDim = getSectorDimensions(x, y);
            const jadel::Surface *spriteToDraw = NULL;
            /*if (currentSector.occupant && !currentSector.occupant->transit.inTransit)
            {
                AnimFrames* frames
                    = &currentSector.occupant->gameObject.frames;
                spriteToDraw = frames->sprites[frames->currentFrameIndex];
            }*/
            jadel::Node<Item*>* currentItemNode = currentSector.items.head; 
            if (currentSector.occupant)
            {
                spriteToDraw = getCurrentFrame(currentSector.occupant);
            }
            else if (currentSector.numItems == 1)
            {
                spriteToDraw = getCurrentFrame(&currentItemNode->data->gameObject);
            }
            else if (currentSector.numItems > 1)
            {
                spriteToDraw = g_Assets.getSurface("res/clutter.png");
            }
            if (spriteToDraw)
            {
                // jadel::graphicsBlit(spriteToDraw, {0, 0, 256, 256});
                jadel::graphicsCopyEqualSizeSurface(spriteToDraw);
                jadel::Vec3 illumination = currentSector.illumination;
                jadel::graphicsMultiplyPixelValues(illumination.x, illumination.y, illumination.z);
            }

            jadel::graphicsPopTargetSurface();
            if (spriteToDraw)
            {
                jadel::graphicsBlit(&_workingTileSurface, entityDim);
            }
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

void renderBars()
{
    for (int y = _yStart; y < _yEnd; ++y)
    {
        for (int x = _xStart; x < _xEnd; ++x)
        {
            jadel::Recti entityDim = getSectorDimensions(x, y);
            entityDim.x1 = entityDim.x1 - entityDim.x0;
            entityDim.y1 = entityDim.y1 - entityDim.y0;
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
