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

enum MyButtonState
{
    BUTTON_STATE_HOVERED,
    BUTTON_STATE_CLICKED,
    BUTTON_STATE_HELD,
    BUTTON_STATE_RELEASED
};

struct Clickable
{
    bool hovered;
    bool clicked;
    bool held;
    bool released;
};

struct Button
{
    uint32 id;
    uint32 contentFlags;
    Clickable clickable;
    jadel::String text;
    const jadel::Surface* image;
    const jadel::Surface* pressedImage;
    jadel::Color color;
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
    Clickable headerClickable;
    jadel::Color headerIdleColor;
    jadel::Color headerHoverColor;
    jadel::Color headerClickedColor;
    jadel::Color headerCurrentColor;
    jadel::String headerString;
    float headerHeight;
    const jadel::Surface* background;
    uint32 lastButtonID;
};

void dialogBoxUpdate(DialogBox* box);

void dialogBoxRender(DialogBox* box, RenderLayer* layer);

bool dialogBoxInit(DialogBox* target, jadel::Vec2 pos, jadel::Vec2 dimensions, 
                    const char* name, const jadel::Surface* background, uint32 flags);

uint32 dialogBoxAddButton(DialogBox *target, const char* text, const jadel::Surface* buttonImage, 
                        const jadel::Surface* buttonPressedImage, jadel::Color color, uint32 flags);

bool isButtonState(uint32 id, uint32 state, DialogBox *box);