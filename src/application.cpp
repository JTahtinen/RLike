#include "application.h"
#include "globals.h"
#include "imageload.h"

bool Application::init(jadel::Window *window)
{
    systemInitLoadImage();
    gameInitialized = false;
    this->window = window;
    AssetCollection &assets = g_Assets;
    // assets.surfaces = jadel::Vector<jadel::Surface>(50);
    jadel::vectorInit(50, &assets.surfaces);
    jadel::vectorInit(50, &assets.surfaceNames);
    jadel::vectorInit(50, &assets.fonts);
    jadel::vectorInit(50, &assets.fontNames);
    assets.loadSurface("res/missing.png");
    assets.loadSurface("res/loading.png");
    assets.loadSurface("res/bup.png");
    assets.loadSurface("res/bdown.png");
    assets.loadSurface("res/dude1.png");
    assets.loadSurface("res/dude1l.png");
    assets.loadSurface("res/clutter.png");
    assets.loadSurface("res/grass.png");
    assets.loadSurface("res/wall.png");
    assets.loadSurface("res/poison.png");
    assets.loadSurface("res/poison2.png");
    assets.loadSurface("res/poison3.png");
    assets.loadSurface("res/hpotion.png");
    assets.loadSurface("res/hpotion2.png");
    assets.loadSurface("res/hpotion3.png");
    assets.loadSurface("res/portal.png");
    assets.loadSurface("res/dagger.png");
    assets.loadSurface("res/inventory3.png");
    assets.loadSurface("res/villasukat.png");
    assets.loadSurface("res/fire.png");
    assets.loadSurface("res/fire2.png");
    assets.loadFont("res/fonts/arial.fnt");
    g_currentFont = assets.getFont("res/fonts/arial.fnt");
    appState = APP_STATE_MENU;
    dialogBoxInit(&mainMenu.menuBox, jadel::Vec2(-16.0f, -9.0f), jadel::Vec2(32.0f, 18.0f),
                  "MAIN MENU", assets.getSurface("res/inventory3.png"), 0);
    mainMenu.newGameButton = dialogBoxAddButton(&mainMenu.menuBox, "New Game", assets.getSurface("res/bup.png"), assets.getSurface("res/bdown.png"), {0}, BUTTON_GRAPHICS_IMAGE | BUTTON_GRAPHICS_TEXT);
    mainMenu.quitButton = dialogBoxAddButton(&mainMenu.menuBox, "Quit Game", assets.getSurface("res/bup.png"), assets.getSurface("res/bdown.png"), {0}, BUTTON_GRAPHICS_IMAGE | BUTTON_GRAPHICS_TEXT);
    setGame(&_game);

    RendererInfo rInfo;
    rInfo.screenObjectPoolSize = 10;
    rInfo.stringBufferSize = 100;
    rInfo.workingBufferDimensions = {window->width, window->height};
    mainMenu.renderer.init(rInfo);
    mainMenu.renderLayer = mainMenu.renderer.createLayer();
    return true;
}

void Application::update()
{
    switch (appState)
    {
    case APP_STATE_MENU:
    {
        dialogBoxUpdate(&mainMenu.menuBox);
        if (isButtonState(mainMenu.newGameButton, BUTTON_STATE_RELEASED, &mainMenu.menuBox))
        {
            if (!gameInitialized)
            {
                appState = APP_STATE_LOAD_GAME;
                mainMenu.renderer.submitRenderable(g_Assets.getSurface("res/loading.png"), jadel::Vec2(-16.0f, -9.0f), jadel::Vec2(32.0f, 18.0f), mainMenu.renderLayer);
                mainMenu.renderer.render();
                break;
            }
        }
        if (isButtonState(mainMenu.quitButton, BUTTON_STATE_RELEASED, &mainMenu.menuBox))
        {
            exit(0);
        }
        dialogBoxRender(&mainMenu.menuBox, mainMenu.renderLayer, &mainMenu.renderer);
        mainMenu.renderer.render();
    }
    break;
    case APP_STATE_GAME:
    {
        updateGame();
    }
    break;
    case APP_STATE_LOAD_GAME:
    {
        gameInitialized = initGame(window);
        if (!gameInitialized)
        {
            exit(0);
        }
        if (gameInitialized)
        {
            appState = APP_STATE_GAME;
        }
    }
    break;
    }
}

jadel::Surface *Application::getScreenBuffer()
{
    switch (appState)
    {
    case APP_STATE_MENU:
    {
        return mainMenu.renderer.getScreenBuffer();
    }
    case APP_STATE_GAME:
    {
        return _game.gameRenderer.getScreenBuffer();
    }
    case APP_STATE_LOAD_GAME:
    {
        return mainMenu.renderer.getScreenBuffer();
    }
    default:
    {
        return mainMenu.renderer.getScreenBuffer();
    }
    }
    return NULL;
}
