#pragma once
#include <jadel.h>
#include "screenobject.h"
#include "render.h"

enum ButtonContent
{
    BUTTON_GRAPHICS_TEXT = 1,
    BUTTON_GRAPHICS_IMAGE = 1 << 1,
    BUTTON_GRAPHICS_COLOR = 1 << 2
};

enum DialogBoxFlags
{
    DIALOG_BOX_HEADER = 1,
    DIALOG_BOX_MOVABLE = 1 << 1
};

struct Button
{
    uint32 contentFlags;
    jadel::String text;
    jadel::Surface* image;
    jadel::Color color;
    bool isHovered;
    bool isLeftClicked;
    bool isRightClicked;
    bool isLeftReleased;
    bool isRightReleased;
};

struct DialogBox
{
    
    uint32 contentFlags;
    ScreenObject screenObject;
    jadel::Vector<Button> buttons;
    jadel::Vec2 buttonDim;
    jadel::Vec2 dimensions;
    bool hooked;
    jadel::Vec2 hookPosition;
    jadel::Color headerIdleColor;
    jadel::Color headerHoverColor;
    jadel::Color headerClickedColor;
    jadel::Color headerCurrentColor;
    jadel::String headerString;
    float headerHeight;
    const jadel::Surface* background;
};

void dialogBoxUpdate(DialogBox* box);

void dialogBoxRender(DialogBox* box, RenderLayer* layer);

bool dialogBoxInit(DialogBox* target, jadel::Vec2 pos, jadel::Vec2 dimensions, 
                    const char* name, const jadel::Surface* background, uint32 flags);

bool dialogBoxAddButton(DialogBox *target, const char* text, jadel::Surface* image, jadel::Color color, uint32 flags);