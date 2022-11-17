#pragma once
#include "render.h"
#include "game.h"

extern Renderer* gameRenderer;

void renderGame();

bool systemInitRenderGame(jadel::Window* window, Renderer* renderer);