#pragma once
#include <jadel.h>
#include "game.h"
#include "dialogbox.h"
#include "render.h"

enum ApplicationState
{
    APP_STATE_GAME,
    APP_STATE_MENU,
    APP_STATE_LOAD_GAME
};

struct MainMenu
{
    DialogBox menuBox;
    uint32 newGameButton;
    uint32 quitButton;
    Renderer renderer;
    uint32 renderLayer;
};

struct Application
{
    jadel::Window* window;
    uint32 appState;
    Game _game;
    MainMenu mainMenu;
    bool gameInitialized;
    bool init(jadel::Window* window);
    void update();
    jadel::Surface* getScreenBuffer();
};