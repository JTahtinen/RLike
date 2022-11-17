#pragma once
#include <jadel.h>
#include <vector>
#include "screenobject.h"
#include "font.h"

#define INVALID_RENDERLAYER_HANDLE (0)

struct RenderLayer
{
    jadel::Vector<ScreenObject *> screenObjects;
};

struct RendererInfo
{
    uint32 screenObjectPoolSize;
    uint32 stringBufferSize;
    jadel::Point2i workingBufferDimensions;
};

struct Renderer
{
    RenderLayer* _layers;
    uint32* _layerHandles;
    uint32 _numLayers;

    jadel::Surface _workingBuffer;

    char *_stringBuffer;
    uint32 _stringBufferSize;
    ScreenObject *_screenObjectPool;
    uint32 _screenObjectPoolSize;
    uint32 _numReservedScreenObjects;

    jadel::Stack<jadel::Mat3> _transformationStack;

public:
    bool init(RendererInfo info);
    void render();
    jadel::Surface *getScreenBuffer();
    void submitRenderable(ScreenObject *scrObj, uint32 layer);
    void submitRenderable(const jadel::Surface *surface, jadel::Vec2 pos, jadel::Vec2 dimensions, uint32 layer);
    void submitRenderable(const jadel::Color color, jadel::Vec2 pos, jadel::Vec2 dimensions, uint32 layer);

    bool submitText(jadel::Vec2 pos, float scale, const Font *font, ScreenObject *target, const char *content, ...);
    bool submitText(jadel::Vec2 pos, float scale, const Font *font, uint32 layer, const char *content, ...);

    uint32 createLayer();
    // bool submitText(const char *content, jadel::Vec2 pos, float scale, const Font* font, ScreenObject* target);
    // bool submitText(const char *content, jadel::Vec2 pos, float scale, const Font* font, RenderLayer* layer);
    // void renderText(const char *text, jadel::Vec2 pos, float scale, const Font *font, ScreenObject* target);

private:
    RenderLayer* getRenderLayer(uint32 handle);
    void pushMatrix(jadel::Mat3 matrix);
    bool popMatrix();
    ScreenObject *reserveScreenObject();
    void renderSurface(const jadel::Surface *surface, jadel::Vec2 start, jadel::Vec2 end, jadel::Rectf sourceRect);
    void renderSurface(const jadel::Surface *surface, jadel::Vec2 start, jadel::Vec2 end);
    void renderRect(float a, float r, float g, float b, jadel::Vec2 start, jadel::Vec2 end);
    void renderRect(jadel::Color color, jadel::Vec2 start, jadel::Vec2 end);
    void submitRenderable(const jadel::Surface *surface, jadel::Vec2 pos, jadel::Vec2 dimensions, jadel::Rectf sourceRect, uint32 layerHandle);
    void flushLayer(uint32 layer);
    void flush();
    void renderText(const char *text, jadel::Vec2 pos, float scale, const Font *font, ScreenObject *target);
};

jadel::Vec2 getTextScreenSize(const char *text, float scale, const Font *font);

bool systemInitRender(jadel::Window *window);
