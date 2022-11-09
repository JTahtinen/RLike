#include <jadel.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "game.h"
//#include "file.h"
#include <thread>
#include "timer.h"
#include "render.h"

static int windowWidth = 1280;
static int windowHeight = 720;

int JadelMain()
{
    if (!JadelInit())
    {
        jadel::message("Jadel init failed!\n");
        return 0;
    }
    jadel::allocateConsole();
    srand(time(NULL));
    jadel::Window window;
    jadel::windowCreate(&window, "Rlike", windowWidth, windowHeight);     
    jadel::Surface winSurface;
    jadel::graphicsCreateSurface(windowWidth, windowHeight, &winSurface);
    uint32* winPixels = (uint32*)winSurface.pixels;
    jadel::memoryInit(MB(50));
    Game game;
    setGame(&game);
    if (!initGame(&window))
    {
        jadel::message("Game init failed!\n");
        exit(0);
    }
    Timer frameTimer;
    frameTimer.start();
    uint32 elapsedInMillis = 0;
    uint32 minFrameTime = 1000 / 60;
    while (true)
    {
        JadelUpdate();
        updateGame();
        jadel::windowUpdate(&window, getScreenBuffer());
        
        elapsedInMillis = frameTimer.getMillisSinceLastUpdate();
        if (elapsedInMillis < minFrameTime)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(minFrameTime - elapsedInMillis));
        }
        if (elapsedInMillis > 0)
        {
            frameTime = (float)frameTimer.getMillisSinceLastUpdate() * 0.001f;
        }

        //uint32 debugTime = frameTimer.getMillisSinceLastUpdate();
//        jadel::message("%f\n", frameTime);
        
        frameTimer.update();
    }
    return 0;
}
