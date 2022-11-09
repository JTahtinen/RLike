#include "render.h"
#include "game.h"
#include <jadel.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <stack>
#include <vector>
#include "screenobject.h"
#include "font.h"

#define NUM_RENDERABLE_TYPES (2)
#define MAX_RENDERABLES_PER_TYPE (250)
#define SCREEN_OBJECT_POOL_SIZE (NUM_RENDERABLE_TYPES * MAX_RENDERABLES_PER_TYPE)

#define STRING_POOL_SIZE (200)
RenderLayer gameLayer;
RenderLayer uiLayer;

static jadel::Surface workingBuffer = {0};
static jadel::Surface workingTileSurface = {0};

static int xStart;
static int yStart;
static int xEnd;
static int yEnd;


static jadel::String *stringPool;
static size_t numReservedStrings = 0;

static ScreenObject *screenObjectPool;
static size_t numReservedScreenObjects = 0; 

static std::stack<jadel::Mat3> transformationStack;

static jadel::Mat3 identityMatrix(1.0f, 0, 0,
                                  0, 1.0f, 0,
                                  0, 0, 1.0f);

static jadel::Mat3 viewMatrix(1.0f / 16.0f, 0.0f, 0.0f,
                              0.0f, 1.0f / 9.0f, 0.0f,
                              0.0f, 0.0f, 1.0f);

static void pushMatrix(jadel::Mat3 matrix)
{
    jadel::Mat3 multipliedMatrix = matrix.mul(transformationStack.top());
    transformationStack.emplace(multipliedMatrix);
}

static bool popMatrix()
{
    if (transformationStack.size() <= 1)
        return false;
    transformationStack.pop();
    return true;
}

jadel::String* reserveString()
{
    if (numReservedStrings == STRING_POOL_SIZE) return NULL;
    jadel::String* result = &stringPool[numReservedStrings++];
    return result;
}

ScreenObject* reserveScreenObject()
{
    if (numReservedScreenObjects == SCREEN_OBJECT_POOL_SIZE) return NULL;
    ScreenObject* result = &screenObjectPool[numReservedScreenObjects++];
    return result;
}

void renderSurface(const jadel::Surface *surface, jadel::Vec2 start, jadel::Vec2 end, jadel::Rectf sourceRect)
{
    if (!surface)
        return;
    jadel::Mat3 matrix = transformationStack.top();
    jadel::Vec2 screenStart = matrix.mul(start);
    jadel::Vec2 screenEnd = matrix.mul(end);
    jadel::graphicsBlitRelative(surface, jadel::Rectf{screenStart.x, screenStart.y, screenEnd.x, screenEnd.y}, sourceRect);
}

void renderSurface(const jadel::Surface *surface, jadel::Vec2 start, jadel::Vec2 end)
{
    renderSurface(surface, start, end, jadel::Rectf(0, 0, 1.0f, 1.0f));
}

void renderRect(float a, float r, float g, float b, jadel::Vec2 start, jadel::Vec2 end)
{
    jadel::Mat3 matrix = transformationStack.top();
    jadel::Vec2 screenStart = matrix.mul(start);
    jadel::Vec2 screenEnd = matrix.mul(end);
    jadel::graphicsDrawRectRelative(jadel::Rectf{screenStart.x, screenStart.y, screenEnd.x, screenEnd.y},
                                    jadel::Color{a, r, g, b});
}

void renderRect(jadel::Color color, jadel::Vec2 start, jadel::Vec2 end)
{
    renderRect(color.a, color.r, color.g, color.b, start, end);
}

void submitRenderable(ScreenObject* scrObj, RenderLayer *layer)
{
    layer->screenObjects.emplace_back(scrObj);
}

void submitRenderable(const jadel::Surface *surface, jadel::Vec2 pos, jadel::Vec2 dimensions, jadel::Rectf sourceRect, RenderLayer *layer)
{
    ScreenObject* scrObj = reserveScreenObject();
    initScreenObject(pos, scrObj);
    ScreenSurface scrSurf = createScreenSurface(jadel::Vec2(0, 0), dimensions, sourceRect, surface);
    screenObjectPushScreenSurface(scrSurf, scrObj);
    submitRenderable(scrObj, layer);
}

void submitRenderable(const jadel::Surface *surface, jadel::Vec2 pos, jadel::Vec2 dimensions, RenderLayer *layer)
{
    submitRenderable(surface, pos, dimensions, jadel::Rectf{0, 0, 1.0f, 1.0f}, layer);
}

void submitRenderable(const jadel::Color color, jadel::Vec2 pos, jadel::Vec2 dimensions, RenderLayer *layer)
{
    ScreenObject* scrObj = reserveScreenObject();
    initScreenObject(pos, scrObj);
    ScreenRect scrRect = createScreenRect(jadel::Vec2(0, 0), dimensions, color);
    screenObjectPushRect(scrRect, scrObj);
    submitRenderable(scrObj, layer);
}

void flushLayer(RenderLayer *layer)
{
    pushMatrix(viewMatrix);
    for (ScreenObject* scrObj : layer->screenObjects)
    {
        uint32 surfacesRendered = 0;
        uint32 rectsRendered = 0;
        for (int i = 0; i < scrObj->numElements; ++i)
        {
            uint32 type = scrObj->typeQueue[i];
            switch (type)
            {
            case RENDERABLE_TYPE_SURFACE:
            {
                ScreenSurface scrSurf = scrObj->screenSurfaces[surfacesRendered++];
                renderSurface(scrSurf.surface, scrObj->pos + scrSurf.pos, scrObj->pos + scrSurf.pos + scrSurf.dimensions, scrSurf.sourceRect);
            }
            break;
            case RENDERABLE_TYPE_RECT:
            {
                ScreenRect scrRect = scrObj->screenRects[rectsRendered++];
                renderRect(scrRect.color, scrObj->pos + scrRect.pos, scrObj->pos + scrRect.pos + scrRect.dimensions);
            }
            break;
            }
        }
    }
    layer->screenObjects.clear();
    popMatrix();
}

void flush()
{
    flushLayer(&gameLayer);
    flushLayer(&uiLayer);
    numReservedScreenObjects = 0;
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

bool systemInitRender(jadel::Window *window)
{
    stbi_set_flip_vertically_on_load(true);
    if (!jadel::graphicsCreateSurface(window->width, window->height, &workingBuffer))
        return false;
    if (!jadel::graphicsCreateSurface(256, 256, &workingTileSurface))
        return false;

    transformationStack.emplace(identityMatrix);
    screenObjectPool = (ScreenObject*)jadel::memoryReserve(SCREEN_OBJECT_POOL_SIZE * sizeof(ScreenObject));
    stringPool = (jadel::String*)jadel::memoryReserve(STRING_POOL_SIZE * sizeof(jadel::String));
    gameLayer.screenObjects.reserve(MAX_RENDERABLES_PER_TYPE);
    uiLayer.screenObjects.reserve(MAX_RENDERABLES_PER_TYPE);
    jadel::graphicsPushTargetSurface(&workingBuffer);
    jadel::graphicsSetClearColor(0);
    jadel::graphicsClearTargetSurface();
    jadel::graphicsPopTargetSurface();

    return true;
}

static jadel::Recti getSectorDimensions(int x, int y)
{
    jadel::Recti sectorPos = getSectorScreenPos(x, y);
    jadel::Recti result = {.x0 = sectorPos.x0,
                           .y0 = sectorPos.y0,
                           .x1 = sectorPos.x0 + currentGame->tileScreenW,
                           .y1 = sectorPos.y0 + currentGame->tileScreenH};
    return result;
}

static void renderBars()
{
    for (int y = yStart; y < yEnd; ++y)
    {
        for (int x = xStart; x < xEnd; ++x)
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
                spriteToDraw = getCurrentFrame(currentSector.occupant);
            }
            else if (currentSector.numItems == 1)
            {
                spriteToDraw = getCurrentFrame(&currentSector.items[currentSector.numItems - 1]->gameObject);
            }
            else if (currentSector.numItems > 1)
            {
                spriteToDraw = currentGame->assets.getSurface("res/clutter.png");
                ;
            }
            if (spriteToDraw)
            {
                jadel::graphicsBlit(spriteToDraw, {0, 0, 256, 256});
            }
            jadel::graphicsMultiplyPixelValues(currentSector.illumination);
            jadel::graphicsPopTargetSurface();

            jadel::graphicsBlit(&workingTileSurface, entityDim);

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

void renderText(const char *text, jadel::Vec2 pos, float scale, const Font *font, ScreenObject* target)
{
    const jadel::Surface *atlas = &font->fontAtlas;

    if (!text)
        return;
    const char *c = text;
    float xAdvance = 0;
    while (*c)
    {
        const Letter *letter = &font->letters[(int)(*c)];
        jadel::Vec2 letterPos = jadel::Vec2((xAdvance + letter->xOffset) * scale,
                                      font->info.base * scale - (letter->yOffset + letter->height) * scale);
        jadel::Rectf sourceRect = {letter->x, 1.0f - letter->y, letter->x + letter->width, 1.0f - letter->y - letter->height};
        screenObjectPushScreenSurface(pos + letterPos, jadel::Vec2(letter->width * scale, letter->height * scale), atlas, sourceRect, target);
        xAdvance += letter->xAdvance;
        ++c;
    }
}

void renderText(const char *text, jadel::Vec2 pos, float scale, const Font *font, RenderLayer* layer)
{
    ScreenObject* scrObj = reserveScreenObject();
    initScreenObject(pos, scrObj);
    renderText(text, jadel::Vec2(0, 0), scale, font, scrObj);
    submitRenderable(scrObj, layer);
}

jadel::Vec2 getTextScreenSize(const char* text, float scale, const Font* font)
{
    if (!text) return jadel::Vec2(0, 0);
    jadel::Vec2 result(0, 0);
    const char *c = text;
    while (*c)
    {
        const Letter* letter = &font->letters[(int)(*c)];
        result.x += letter->xAdvance * scale;
        if (letter->height * scale > result.y) result.y = letter->height * scale; 
        ++c;
    }
    return result;
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

        /*
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
        */
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
    }
 
    flush();
    jadel::graphicsPopTargetSurface();
}

jadel::Surface *getScreenBuffer()
{
    return &workingBuffer;
}