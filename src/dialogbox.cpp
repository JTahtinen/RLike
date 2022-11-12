#include "dialogbox.h"
#include "math.h"
#include "render.h"
#include "game.h"
#include <jadel.h>

static jadel::Vec2 mouseScreenPos(0, 0);
// static bool canLeftClickButton = true;
// static bool canRightClickButton = true;

static jadel::Vec2 initiallyClickedPoint(0, 0);

jadel::Rectf getPosOfButton(int index, const DialogBox *box)
{
    jadel::Rectf result;

    // TODO: Clean up magic numbers

    float startX;
    float startY;
    float endX;
    float endY;

    endY = box->dimensions.y - (index * 0.6f) - 0.2f;
    startY = endY - 0.6f;
    startX = 0.2f;
    endX = box->dimensions.x - 0.2f;
    result = {startX, startY, endX, endY};
    return result;
}

jadel::Rectf getScreenPosOfButton(int index, const DialogBox *box)
{
    jadel::Rectf result;
    jadel::Rectf relPos = getPosOfButton(index, box);
    jadel::Vec2 boxPos = box->screenObject.pos;
    result = {boxPos.x + relPos.x0, boxPos.y + relPos.y0, boxPos.x + relPos.x1, boxPos.y + relPos.y1};
    return result;
}

void updateClickable(Clickable *clickable, jadel::Vec2 start, jadel::Vec2 end)
{
    bool cursorOnClickable = pointInRectf(mouseScreenPos, start, end);

    if ((jadel::inputIsMouseLeftReleased() || jadel::inputIsMouseLeftHeld()) && !pointInRectf(initiallyClickedPoint, start, end))
    {
        clickable->hovered = false;
        clickable->clicked = false;
        clickable->held = false;
        clickable->released = false;
        return;
    }

    if (cursorOnClickable)
    {
        clickable->hovered = true;
        clickable->clicked = jadel::inputIsMouseLeftClicked();
        clickable->held = jadel::inputIsMouseLeftHeld();
        clickable->released = jadel::inputIsMouseLeftReleased();
    }
    else
    {
        clickable->hovered = false;
        clickable->clicked = false;
        clickable->held = false;
        clickable->released = false;
    }
}

void headerUpdate(DialogBox *box)
{
    if (!(box->contentFlags & DIALOG_BOX_HEADER) || !(box->contentFlags & DIALOG_BOX_MOVABLE))
        return;
    jadel::Vec2 pos = box->screenObject.pos;
    jadel::Vec2 headerStart(pos.x, pos.y + box->dimensions.y);
    jadel::Vec2 headerEnd(headerStart.x + box->dimensions.x, headerStart.y + box->headerHeight);
    jadel::Color *headerColor = &box->headerCurrentColor;

    Clickable *clickable = &box->headerClickable;
    updateClickable(clickable, headerStart, headerEnd);

    if (box->hooked)
    {
        *headerColor = box->headerClickedColor;
        pos = mouseScreenPos - box->hookPosition;
    }
    else if (clickable->clicked)
    {
        box->hooked = true;
        box->hookPosition = mouseScreenPos - pos;
    }
    else if (clickable->hovered)
    {
        *headerColor = box->headerHoverColor;
    }
    else
    {
        *headerColor = box->headerIdleColor;
    }

    if (!jadel::inputIsMouseLeftHeld())
        box->hooked = false;

    box->screenObject.pos = pos;
}

void dialogBoxUpdate(DialogBox *box)
{
    jadel::Vec2 pos = box->screenObject.pos;

    mouseScreenPos = jadel::inputGetMouseRelative();
    // TODO: multiply by matrix
    mouseScreenPos.x *= 16.0f;
    mouseScreenPos.y *= 9.0f;

    if (jadel::inputIsMouseLeftClicked())
    {
        initiallyClickedPoint = mouseScreenPos;
    }

    if (box->contentFlags && DIALOG_BOX_HEADER)
    {
        headerUpdate(box);
    }
    for (int i = 0; i < box->buttons.size; ++i)
    {
        {
            Button *button = &box->buttons[i];
            jadel::Rectf buttonPos = getScreenPosOfButton(i, box);
            jadel::Vec2 buttonStart(buttonPos.x1, buttonPos.y1);
            jadel::Vec2 buttonEnd(buttonPos.x0, buttonPos.y0);
            updateClickable(&button->clickable, buttonStart, buttonEnd);
        }
    }
}

void dialogBoxRender(DialogBox *box, RenderLayer *layer)
{
    if (!layer)
        return;
    ScreenObject *scrObj = &box->screenObject;
    screenObjectClear(scrObj);

    screenObjectPushScreenSurface(jadel::Vec2(0, 0), box->dimensions, box->background, scrObj);
    if (box->contentFlags && DIALOG_BOX_HEADER)
    {
        screenObjectPushRect(jadel::Vec2(0, box->dimensions.y), jadel::Vec2(box->dimensions.x, box->headerHeight), box->headerCurrentColor, scrObj);
        static jadel::Vec2 headerTextSize = getTextScreenSize(box->headerString.c_str(), 2.5f, &currentGame->font);
        submitText(jadel::Vec2((box->dimensions.x * 0.5f) - (headerTextSize.x * 0.5f), box->dimensions.y + 0.03f), 2.5f,
                   &currentGame->font, scrObj, box->headerString.c_str());
    }

    for (int i = 0; i < box->buttons.size; ++i)
    {
        const Button *button = &box->buttons[i];
        jadel::Rectf buttonScreenArea = getPosOfButton(i, box);
        jadel::Vec2 buttonStart = jadel::Vec2(buttonScreenArea.x0, buttonScreenArea.y0);
        jadel::Vec2 buttonDim = jadel::Vec2(buttonScreenArea.x1, buttonScreenArea.y1) - buttonStart;
        if (button->contentFlags & BUTTON_GRAPHICS_IMAGE)
        {
            const jadel::Surface *buttonImage = button->clickable.held ? button->pressedImage : button->image;
            if (buttonImage)
            {
                screenObjectPushScreenSurface(buttonStart, buttonDim, buttonImage, scrObj);
            }
        }
        if (button->contentFlags & BUTTON_GRAPHICS_COLOR)
        {
            screenObjectPushRect(buttonStart, buttonDim, button->color, scrObj);
        }
        if (button->contentFlags & BUTTON_GRAPHICS_TEXT)
        {
            jadel::Vec2 textScreenSize = getTextScreenSize(button->text.c_str(), 3.2f, &currentGame->font);
            jadel::Vec2 textPos(box->dimensions.x * 0.5f - textScreenSize.x * 0.5f, buttonStart.y + 0.16f);
            submitText(textPos, 3.2f, &currentGame->font, scrObj, button->text.c_str());
        }
    }
    submitRenderable(scrObj, layer);
}

bool dialogBoxInit(DialogBox *target, jadel::Vec2 pos, jadel::Vec2 dimensions, const char *name,
                   const jadel::Surface *background, uint32 flags)
{
    if (!target || !background)
        return false;

    target->contentFlags = flags;
    target->background = background;
    if (!jadel::vectorInit(20, &target->buttons))
        return false;
    initScreenObject(&target->screenObject);
    screenObjectSetPos(pos, &target->screenObject);
    target->dimensions = dimensions;
    target->buttonDim = jadel::Vec2(0.3f, 0.15f);
    if (flags & DIALOG_BOX_HEADER)
    {
        target->headerIdleColor = {0.6f, 0.2f, 0.3f, 0.8f};
        target->headerHoverColor = {0.6f, 0.3f, 0.4f, 1.0f};
        target->headerClickedColor = {0.6f, 0.8f, 0.6f, 1.0f};
        target->headerCurrentColor = target->headerIdleColor;
        target->headerHeight = 0.4f;
        target->headerString = jadel::String(name);
    }
    target->hooked = false;
    target->lastButtonID = 1;
    return true;
}

bool isButtonState(uint32 id, uint32 state, DialogBox *box)
{
    if (!box)
        return false;
    Button *button;
    for (int i = 0; i < box->buttons.size; ++i)
    {
        Button *currentButton = &box->buttons[i];
        if (currentButton->id == id)
        {
            button = currentButton;
            break;
        }
    }
    if (!button)
        return false;
    switch (state)
    {
    case BUTTON_STATE_HOVERED:
        return button->clickable.hovered;
    case BUTTON_STATE_CLICKED:
        return button->clickable.clicked;
    case BUTTON_STATE_HELD:
        return button->clickable.held;
    case BUTTON_STATE_RELEASED:
        return button->clickable.released;
    }
    return false;
}

uint32 dialogBoxAddButton(DialogBox *target, const char *text, const jadel::Surface *buttonImage, const jadel::Surface *buttonPressedImage, jadel::Color color, uint32 flags)
{
    if (!target)
        return 0;
    Button result;
    result.text = text;
    result.image = buttonImage;
    result.pressedImage = buttonPressedImage;
    result.color = color;
    result.contentFlags = flags;
    result.id = target->lastButtonID++;
    target->buttons.push(result);
    return result.id;
}