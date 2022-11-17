#include "render.h"
#include "game.h"
#include <jadel.h>
#include <stack>
#include <vector>
#include "screenobject.h"
#include "font.h"
#include <stdio.h>
#include "globals.h"

#define NUM_RENDERABLE_TYPES (2)
#define MAX_RENDERABLES_PER_TYPE (250)
#define SCREEN_OBJECT_POOL_SIZE (NUM_RENDERABLE_TYPES * MAX_RENDERABLES_PER_TYPE)

#define MAX_LAYERS_PER_RENDERER (5)

#define STRING_BUFFER_SIZE (500)

#define FORMAT_SCREEN_STRING(buffer, bufferSize)                                                      \
    {                                                                                                 \
        char *stringBufferPointer = buffer;                                                           \
        char *stringBufferEnd = buffer + bufferSize;                                                  \
        va_list ap; /* points to each unnamed arg in turn */                                          \
        const char *p, *sval;                                                                         \
        int ival;                                                                                     \
        double dval;                                                                                  \
        va_start(ap, content); /* make ap point to 1st unnamed arg */                                 \
        for (p = content; *p; p++)                                                                    \
        {                                                                                             \
            if (*p != '%')                                                                            \
            {                                                                                         \
                *stringBufferPointer = *p;                                                            \
                ++stringBufferPointer;                                                                \
                continue;                                                                             \
            }                                                                                         \
            switch (*++p)                                                                             \
            {                                                                                         \
            case 'd':                                                                                 \
                ival = va_arg(ap, int);                                                               \
                stringBufferPointer +=                                                                \
                    snprintf(stringBufferPointer, stringBufferEnd - stringBufferPointer, "%d", ival); \
                break;                                                                                \
            case 'f':                                                                                 \
                dval = va_arg(ap, double);                                                            \
                stringBufferPointer +=                                                                \
                    snprintf(stringBufferPointer, stringBufferEnd - stringBufferPointer, "%f", dval); \
                break;                                                                                \
            case 's':                                                                                 \
                for (sval = va_arg(ap, char *); *sval; sval++)                                        \
                {                                                                                     \
                    snprintf(stringBufferPointer, stringBufferEnd - stringBufferPointer, "%s", sval); \
                    ++stringBufferPointer;                                                            \
                }                                                                                     \
                break;                                                                                \
            default:                                                                                  \
                *stringBufferPointer = *p;                                                            \
                ++stringBufferPointer;                                                                \
                break;                                                                                \
            }                                                                                         \
        }                                                                                             \
        *stringBufferPointer = '\0';                                                                  \
        va_end(ap);                                                                                   \
    }
/*
RenderLayer gameLayer;
RenderLayer uiLayer;

static jadel::Surface workingBuffer = {0};
static jadel::Surface worldBuffer = {0};
static jadel::Surface workingTileSurface = {0};

static int xStart;
static int yStart;
static int xEnd;
static int yEnd;

static char *stringBuffer;

static ScreenObject *screenObjectPool;
static size_t numReservedScreenObjects = 0;

static std::stack<jadel::Mat3> transformationStack;
*/
static uint32 freeLayerHandle = 1;

static jadel::Mat3 identityMatrix(1.0f, 0, 0,
                                  0, 1.0f, 0,
                                  0, 0, 1.0f);

static jadel::Mat3 viewMatrix(1.0f / 16.0f, 0.0f, 0.0f,
                              0.0f, 1.0f / 9.0f, 0.0f,
                              0.0f, 0.0f, 1.0f);

bool Renderer::init(RendererInfo info)
{
    
    if (!jadel::graphicsCreateSurface(info.workingBufferDimensions.x, info.workingBufferDimensions.y, &_workingBuffer))
        return false;

    _layers = (RenderLayer*)jadel::memoryReserve(MAX_LAYERS_PER_RENDERER * sizeof(RenderLayer));
    _layerHandles = (uint32*)jadel::memoryReserve(MAX_LAYERS_PER_RENDERER * sizeof(uint32));
    _numLayers = 0;

    for (int i = 0; i < MAX_LAYERS_PER_RENDERER; ++i)
    {
        jadel::vectorInit<ScreenObject*>(MAX_RENDERABLES_PER_TYPE, &_layers[i].screenObjects);
        //_layers[i].screenObjects.reserve(MAX_RENDERABLES_PER_TYPE);
    }
    _transformationStack.init(50);
    _transformationStack.push(identityMatrix);
    _screenObjectPool = (ScreenObject *)jadel::memoryReserve(info.screenObjectPoolSize * sizeof(ScreenObject));
    _screenObjectPoolSize = info.screenObjectPoolSize;
    for (int i = 0; i < info.screenObjectPoolSize; ++i)
    {
        initScreenObject(&_screenObjectPool[i]);
    }
    _stringBuffer = (char *)jadel::memoryReserve(info.stringBufferSize);
    _stringBufferSize = info.stringBufferSize;
    //_gameLayer.screenObjects.reserve(MAX_RENDERABLES_PER_TYPE);
   // _uiLayer.screenObjects.reserve(MAX_RENDERABLES_PER_TYPE);
    jadel::graphicsPushTargetSurface(&_workingBuffer);
    jadel::graphicsSetClearColor(0);
    jadel::graphicsClearTargetSurface();
    jadel::graphicsPopTargetSurface();

    return true;
}

RenderLayer* Renderer::getRenderLayer(uint32 layerHandle)
{
    if (layerHandle == INVALID_RENDERLAYER_HANDLE)
    {
        return NULL;
    }
    for (int i = 0; i < _numLayers; ++i)
    {
        if (_layerHandles[i] == layerHandle)
        {
            return &_layers[i];
        }
    }
    return NULL;
}

uint32 Renderer::createLayer()
{
    if (_numLayers == MAX_LAYERS_PER_RENDERER)
    {
        return INVALID_RENDERLAYER_HANDLE;
    }
    uint32 handle = freeLayerHandle++;
    RenderLayer* layer = &_layers[_numLayers];
    _layerHandles[_numLayers++] = handle;
    return handle;
}

void Renderer::pushMatrix(jadel::Mat3 matrix)
{
    jadel::Mat3 multipliedMatrix = matrix.mul(_transformationStack.top());
    _transformationStack.push(multipliedMatrix);
}

bool Renderer::popMatrix()
{
    if (_transformationStack.size() <= 1)
        return false;
    _transformationStack.pop();
    return true;
}

ScreenObject *Renderer::reserveScreenObject()
{
    if (_numReservedScreenObjects == _screenObjectPoolSize)
        return NULL;
    ScreenObject *result = &_screenObjectPool[_numReservedScreenObjects++];
    screenObjectClear(result);
    return result;
}

bool Renderer::submitText(jadel::Vec2 pos, float scale, const Font *font, ScreenObject *target, const char *content, ...)
{
    if (!content || *content == '\0' || !font || !target)
        return false;
    size_t stringLength = strlen(content);
    if (stringLength >= _stringBufferSize)
        return false;
    FORMAT_SCREEN_STRING(_stringBuffer, _stringBufferSize);
    //    snprintf(stringBuffer, STRING_BUFFER_SIZE, content);
    renderText(_stringBuffer, pos, scale, font, target);
    return true;
}

bool Renderer::submitText(jadel::Vec2 pos, float scale, const Font *font, uint32 layerHandle, const char *content, ...)
{
    if (!content || *content == '\0' || !font || layerHandle == INVALID_RENDERLAYER_HANDLE)
    {
        return false;
    }
    size_t stringLength = strlen(content);
    if (stringLength >= _stringBufferSize)
    {
        return false;
    }
    ScreenObject *scrObj = reserveScreenObject();
    screenObjectSetPos(jadel::Vec2(0, 0), scrObj);
    FORMAT_SCREEN_STRING(_stringBuffer, _stringBufferSize);
    renderText(_stringBuffer, pos, scale, font, scrObj);
    submitRenderable(scrObj, layerHandle);
    return true;
}
/*
bool submitText(const char *content, jadel::Vec2 pos, float scale, const Font *font, ScreenObject *target)
{
    if (!content || *content == '\0' || !font || !target)
        return false;
    size_t stringLength = strlen(content);
    if (stringLength >= STRING_BUFFER_SIZE)
        return false;
    snprintf(stringBuffer, STRING_BUFFER_SIZE, content);
    renderText(stringBuffer, pos, scale, font, target);
    return true;
}

bool submitText(const char *content, jadel::Vec2 pos, float scale, const Font *font, uint32 layerHandle)
{
    if (!layer)
        return false;
    ScreenObject *scrObj = reserveScreenObject();
    initScreenObject(pos, scrObj);
    if (!submitText(content, jadel::Vec2(0, 0), scale, font, scrObj))
        return false;
    submitRenderable(scrObj, layer);
    return true;

}*/

void Renderer::renderSurface(const jadel::Surface *surface, jadel::Vec2 start, jadel::Vec2 end, jadel::Rectf sourceRect)
{
    if (!surface)
        return;
    jadel::Mat3 matrix = _transformationStack.top();
    jadel::Vec2 screenStart = matrix.mul(start);
    jadel::Vec2 screenEnd = matrix.mul(end);
    jadel::graphicsBlitRelative(surface, jadel::Rectf{screenStart.x, screenStart.y, screenEnd.x, screenEnd.y}, sourceRect);
}

void Renderer::renderSurface(const jadel::Surface *surface, jadel::Vec2 start, jadel::Vec2 end)
{
    renderSurface(surface, start, end, jadel::Rectf(0, 0, 1.0f, 1.0f));
}

void Renderer::renderRect(float a, float r, float g, float b, jadel::Vec2 start, jadel::Vec2 end)
{
    jadel::Mat3 matrix = _transformationStack.top();
    jadel::Vec2 screenStart = matrix.mul(start);
    jadel::Vec2 screenEnd = matrix.mul(end);
    jadel::graphicsDrawRectRelative(jadel::Rectf{screenStart.x, screenStart.y, screenEnd.x, screenEnd.y},
                                    jadel::Color{a, r, g, b});
}

void Renderer::renderRect(jadel::Color color, jadel::Vec2 start, jadel::Vec2 end)
{
    renderRect(color.a, color.r, color.g, color.b, start, end);
}

void Renderer::submitRenderable(ScreenObject *scrObj, uint32 layerHandle)
{
    RenderLayer* layer = getRenderLayer(layerHandle);
    if (!layer)
    {
        return;
    }
    layer->screenObjects.push(scrObj);
}

void Renderer::submitRenderable(const jadel::Surface *surface, jadel::Vec2 pos, jadel::Vec2 dimensions, jadel::Rectf sourceRect, uint32 layerHandle)
{
    ScreenObject *scrObj = reserveScreenObject();
    screenObjectSetPos(pos, scrObj);
    ScreenSurface scrSurf = createScreenSurface(jadel::Vec2(0, 0), dimensions, sourceRect, surface);
    screenObjectPushScreenSurface(scrSurf, scrObj);
    submitRenderable(scrObj, layerHandle);
}

void Renderer::submitRenderable(const jadel::Surface *surface, jadel::Vec2 pos, jadel::Vec2 dimensions, uint32 layerHandle)
{
    submitRenderable(surface, pos, dimensions, jadel::Rectf{0, 0, 1.0f, 1.0f}, layerHandle);
}

void Renderer::submitRenderable(const jadel::Color color, jadel::Vec2 pos, jadel::Vec2 dimensions, uint32 layerHandle)
{
    ScreenObject *scrObj = reserveScreenObject();
    screenObjectSetPos(pos, scrObj);
    ScreenRect scrRect = createScreenRect(jadel::Vec2(0, 0), dimensions, color);
    screenObjectPushRect(scrRect, scrObj);
    submitRenderable(scrObj, layerHandle);
}

void Renderer::flushLayer(uint32 layerHandle)
{
    RenderLayer* layer = getRenderLayer(layerHandle);
    if (!layer)
    {
        return;
    }
    pushMatrix(viewMatrix);
    for (int i = 0; i < layer->screenObjects.size; ++i) 
    {
        ScreenObject *scrObj = layer->screenObjects[i];
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

void Renderer::flush()
{
    for (int i = 0; i < _numLayers; ++i)
    {
        flushLayer(_layerHandles[i]);
    }
    //flushLayer(&_gameLayer);
    //flushLayer(&_uiLayer);
    _numReservedScreenObjects = 0;
}

bool systemInitRender(jadel::Window *window)
{
    /*if (!jadel::graphicsCreateSurface(window->width, window->height, &workingBuffer))
        return false;
    if (!jadel::graphicsCreateSurface(window->width, window->height, &worldBuffer))
        return false;
    if (!jadel::graphicsCreateSurface(256, 256, &workingTileSurface))
        return false;

    transformationStack.emplace(identityMatrix);
    screenObjectPool = (ScreenObject *)jadel::memoryReserve(SCREEN_OBJECT_POOL_SIZE * sizeof(ScreenObject));
    for (int i = 0; i < SCREEN_OBJECT_POOL_SIZE; ++i)
    {
        initScreenObject(&screenObjectPool[i]);
    }
    stringBuffer = (char *)jadel::memoryReserve(STRING_BUFFER_SIZE);
    gameLayer.screenObjects.reserve(MAX_RENDERABLES_PER_TYPE);
    uiLayer.screenObjects.reserve(MAX_RENDERABLES_PER_TYPE);
    jadel::graphicsPushTargetSurface(&workingBuffer);
    jadel::graphicsSetClearColor(0);
    jadel::graphicsClearTargetSurface();
    jadel::graphicsPopTargetSurface();
*/
    return true;
}

void Renderer::renderText(const char *text, jadel::Vec2 pos, float scale, const Font *font, ScreenObject *target)
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

jadel::Vec2 getTextScreenSize(const char *text, float scale, const Font *font)
{
    if (!text)
        return jadel::Vec2(0, 0);
    jadel::Vec2 result(0, 0);
    const char *c = text;
    while (*c)
    {
        const Letter *letter = &font->letters[(int)(*c)];
        result.x += letter->xAdvance * scale;
        if (letter->height * scale > result.y)
            result.y = letter->height * scale;
        ++c;
    }
    return result;
}

void Renderer::render()
{
    jadel::graphicsPushTargetSurface(&_workingBuffer);
    jadel::graphicsSetClearColor(0);
    jadel::graphicsClearTargetSurface();

   /* if (currentGame->screenPos.x > -screenTilemapW && currentGame->screenPos.y > -screenTilemapH && currentGame->screenPos.x < currentGame->currentWorld->width && currentGame->screenPos.y < currentGame->currentWorld->height)
    {
        _xStart = currentGame->screenPos.x < 0 ? 0 : currentGame->screenPos.x;
        _yStart = currentGame->screenPos.y < 0 ? 0 : currentGame->screenPos.y;
        _xEnd = currentGame->screenPos.x + screenTilemapW <= currentGame->currentWorld->width
                   ? (currentGame->screenPos.x + screenTilemapW)
                   : currentGame->currentWorld->width;
        _yEnd = currentGame->screenPos.y + screenTilemapH <= currentGame->currentWorld->height
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
 /*       renderWorld();

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
*/
    flush();
    jadel::graphicsPopTargetSurface();
}

jadel::Surface* Renderer::getScreenBuffer()
{
    return &_workingBuffer;
}