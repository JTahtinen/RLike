#include "inventory.h"
#include "game.h"
#include "render.h"
#include "math.h"

static const jadel::Surface* inventorySurface;
static jadel::Color idleHeaderColor = {0.6f, 0.2f, 0.3f, 0.8f};
static jadel::Color hoverHeaderColor = {0.6f, 0.3f, 0.4f, 1.0f};
static jadel::Color hookedHeaderColor = {0.6f, 0.8f, 0.6f, 1.0f};
static jadel::Color itemBGIdleColor = {1.0f, 0.2f, 0.3f, 0.8f};
static jadel::Color itemBGHoverColor = {1.0f, 0.3f, 0.4f, 1.0f};
static const float SpriteSize = 1.5f;

bool systemInitInventory()
{
    inventorySurface = currentGame->assets.getSurface("res/inventory3.png");
    if (!inventorySurface)
        return false;
    return true;
}

void closeInventory(Inventory *inventory)
{
    currentGame->currentState = SUBSTATE_GAME;
    inventory->useMode = false;
    inventory->opening = true;
    inventory->renderable.elapsedTimeMS = 0;
    jadel::message("Inventory closed\n");
}

void printInventory(const Inventory *inventory)
{
    jadel::message("Inventory: \n");
    int j = 1;
    bool inventoryEmpty = true;
    for (int i = 0; i < 10; ++i)
    {
        if (inventory->itemSlots[i].hasItem)
        {
            inventoryEmpty = false;
            jadel::message("%d: %s\n", j++, inventory->itemSlots[i].item->gameObject.entity.name.c_str());
        }
    }
    if (inventoryEmpty)
    {
        jadel::message("Empty\n");
    }
}

ItemSlot *askInventorySlot(Inventory *inventory)
{
    ItemSlot *slot = NULL;

    for (int i = 1; i < 10; ++i)
    {
        int j = 0;
        if (jadel::inputIsKeyTyped(jadel::KEY_0 + i))
        {
            for (int itemIndex = 0; itemIndex < 10; ++itemIndex)
            {
                if (inventory->itemSlots[itemIndex].hasItem)
                {
                    ++j;
                    if (j == i)
                    {
                        slot = &inventory->itemSlots[itemIndex];
                        break;
                    }
                }
            }
        }
    }
    return slot;
}

jadel::Rectf getPosOfItem(int index, Inventory *inventory)
{
    jadel::Rectf result;

    // TODO: Clean up magic numbers
    jadel::Vec2 start = inventory->renderable.screenObject.pos + jadel::Vec2(0.2f, inventory->renderable.mainDimensions.y + SpriteSize * (-index * 1.2f) - 0.2f);

    jadel::Vec2 end = start + jadel::Vec2(SpriteSize, -SpriteSize);
    result = {start.x, start.y, end.x, end.y};
    return result;
}

void useItemInSlot(ItemSlot *slot)
{
    Item *item = slot->item;
    useItem(item, &currentGame->player);
    if (item->flags & ITEM_EFFECT_CONSUMABLE)
        slot->hasItem = false;
}

void dropItemInSlot(ItemSlot *slot)
{
    slot->hasItem = false;
    if (slot->item->gameObject.entity.id == getPlayerWeaponID())
    {
        currentGame->player.equippedWeapon = NULL;
    }
    Sector *currentSector = getSectorOfActor(&currentGame->player);
    addSectorItem(currentSector, slot->item);
    jadel::message("Dropped %s\n", slot->item->gameObject.entity.name.c_str());
    printInventory(&currentGame->player.inventory);
}

void updateSubstateInventory(Inventory *inventory)
{
    InventoryRenderable *inRenderable = &inventory->renderable;
    jadel::Vec2 inPos = inRenderable->screenObject.pos;
    jadel::Vec2 headerStart = inPos + inRenderable->screenObject.screenRects[0].pos;
    jadel::Vec2 headerEnd = headerStart + inRenderable->screenObject.screenRects[0].dimensions;

    jadel::Vec2 mouseScreenPos = jadel::inputGetMouseRelative();
    jadel::Color *headerColor = &inRenderable->headerColor;
    // TODO: multiply by matrix
    mouseScreenPos.x *= 16.0f;
    mouseScreenPos.y *= 9.0f;
    static jadel::Vec2 hookPosition;
    if (pointInRectf(mouseScreenPos, headerStart, headerEnd))
    {
        *headerColor = hoverHeaderColor;
        if (!inRenderable->hooked && jadel::inputLButtonDown)
        {
            inRenderable->hooked = true;
            hookPosition = mouseScreenPos - inPos;
        }
    }
    else
        *headerColor = idleHeaderColor;
    if (inRenderable->hooked)
    {
        *headerColor = hookedHeaderColor;
        inPos = mouseScreenPos - hookPosition;
    }

    if (!jadel::inputLButtonDown)
        inRenderable->hooked = false;

    setInventoryPos(inPos, inRenderable);

    int foundItems = 0;
    static bool canLeftClickItem = true;
    if (!jadel::inputLButtonDown)
    {
        canLeftClickItem = true;
    }
    static bool canRightClickItem = true;
    if (!jadel::inputRButtonDown)
    {
        canRightClickItem = true;
    }
    for (int i = 0; i < 10; ++i)
    {
        ItemSlot *slot = &inventory->itemSlots[i];
        if (slot->hasItem)
        {
            jadel::Rectf itemPos = getPosOfItem(foundItems, inventory);
            if (pointInRectf(mouseScreenPos, jadel::Vec2(itemPos.x1, itemPos.y1), jadel::Vec2(itemPos.x0, itemPos.y0)))
            {
                // jadel::message("Hovering over item %d\n", foundItems);
                slot->hovered = true;
                if (canLeftClickItem && jadel::inputLButtonDown)
                {
                    useItemInSlot(slot);
                    canLeftClickItem = false;
                }
                else if (canRightClickItem && jadel::inputRButtonDown)
                {
                    dropItemInSlot(slot);
                    canRightClickItem = false;
                }
            }
            else
            {
                slot->hovered = false;
            }
            foundItems++;
        }
    }

    renderInventory(inventory);
    if (jadel::inputIsKeyTyped(jadel::KEY_I))
    {
        closeInventory(&currentGame->player.inventory);
        return;
    }
    if (jadel::inputIsKeyTyped(jadel::KEY_U))
    {
        currentGame->player.inventory.useMode = !currentGame->player.inventory.useMode;
        if (currentGame->player.inventory.useMode)
        {
            jadel::message("Use item (1-9)\n");
        }
    }
    if (jadel::inputIsKeyTyped(jadel::KEY_D))
    {
        currentGame->player.inventory.dropMode = !currentGame->player.inventory.dropMode;
        if (currentGame->player.inventory.dropMode)
        {
            jadel::message("Drop item (1-9)\n");
        }
    }
    if (currentGame->player.inventory.useMode)
    {
        ItemSlot *slot = askInventorySlot(&currentGame->player.inventory);
        if (slot)
        {
            useItemInSlot(slot);
            printInventory(&currentGame->player.inventory);
        }
    }
    if (currentGame->player.inventory.dropMode)
    {
        ItemSlot *slot = askInventorySlot(&currentGame->player.inventory);
        if (slot)
        {
            currentGame->player.inventory.dropMode = false;
            dropItemInSlot(slot);
        }
    }
}

bool initInventory(Inventory *in)
{
    if (!in)
        return false;
    for (int i = 0; i < 10; ++i)
    {
        in->itemSlots[i] = {0};
    }
    in->useMode = false;
    in->dropMode = false;
    in->renderable = createInventoryRenderable(jadel::Vec2(-15.0f, -8.0f), jadel::Vec2(15.0f, 16.0f));
    return true;
}

void setInventoryPos(jadel::Vec2 pos, InventoryRenderable *renderable)
{
    renderable->screenObject.pos = pos;
    renderable->inventoryCenter = pos + renderable->inventoryRadius;
}

InventoryRenderable createInventoryRenderable(jadel::Vec2 pos, jadel::Vec2 dimensions)
{
    InventoryRenderable renderable;
    initScreenObject(&renderable.screenObject);
    screenObjectSetPos(pos, &renderable.screenObject);
    renderable.mainDimensions = dimensions;
    renderable.inventoryRadius = jadel::Vec2(dimensions.x * 0.5f, dimensions.y * 0.5f);

    setInventoryPos(pos, &renderable);
    renderable.margin = 0.08f;

    renderable.targetOpeningTimeMS = 1000;
    renderable.elapsedTimeMS = 0;
    renderable.hooked = false;
    renderable.inventoryOpened = false;
    return renderable;
};

void renderInventory(Inventory *inventory)
{
    InventoryRenderable *renderable = &inventory->renderable;
    ScreenObject *scrObj = &renderable->screenObject;
    screenObjectClear(scrObj);

    if (inventory->opening)
    {
        renderable->inventoryOpened = false;
        renderable->inventoryOpenTimer.start();
        inventory->opening = false;
    }

    if (!renderable->inventoryOpened)
    {
        renderable->elapsedTimeMS += renderable->inventoryOpenTimer.getMillisSinceLastUpdate();

        float currentRadiusMod = {(float)renderable->elapsedTimeMS / (float)renderable->targetOpeningTimeMS};
        if (currentRadiusMod > 1.0f)
            currentRadiusMod = 1.0f;
        jadel::Vec2 currentRadius(currentRadiusMod * renderable->inventoryRadius.x, currentRadiusMod * renderable->inventoryRadius.y);
        jadel::Vec2 openingStart = renderable->inventoryRadius - currentRadius;
        jadel::Vec2 openingEnd = renderable->inventoryRadius + currentRadius;

        // renderSurface(&inventorySurface, openingStart, openingEnd);

        screenObjectPushScreenSurface(openingStart, openingEnd - openingStart, inventorySurface, scrObj);

        // pushRenderable(renderable->screenObject, &uiLayer);
        if (renderable->elapsedTimeMS >= renderable->targetOpeningTimeMS)
        {
            renderable->elapsedTimeMS = 0;
            renderable->inventoryOpened = true;
        }
        submitRenderable(scrObj, &uiLayer);
        return;
    }
    screenObjectPushScreenSurface(jadel::Vec2(0, 0), renderable->mainDimensions, inventorySurface, scrObj);
    screenObjectPushRect(jadel::Vec2(0, renderable->mainDimensions.y), jadel::Vec2(renderable->mainDimensions.x, 0.4f), renderable->headerColor, scrObj);
    
    static jadel::Vec2 headerTextSize = getTextScreenSize("INVENTORY", 2.5f, &currentGame->font);
    submitText(jadel::Vec2((renderable->mainDimensions.x * 0.5f) - (headerTextSize.x * 0.5f), renderable->mainDimensions.y + 0.03f), 2.5f, &currentGame->font, scrObj, "INVENTORY");
    float itemY = -1.0f;
    int currentItemNumber = 1;
    for (int i = 0; i < 10; ++i)
    {
        jadel::Vec2 itemDim(SpriteSize, SpriteSize);
        if (!inventory->itemSlots[i].hasItem)
            continue;
        Item *item = inventory->itemSlots[i].item;
        const jadel::Surface *sprite = NULL;
        if (item->gameObject.frames.numFrames > 0)
            sprite = getCurrentFrame(&item->gameObject);

        jadel::Color itemColor;
        if (item->gameObject.entity.id == getPlayerWeaponID())
            itemColor = jadel::Color{1.0f, 1.0f, 0.2f, 1.0f};
        else
        {
            if (inventory->itemSlots[i].hovered)
                itemColor = itemBGHoverColor;
            else
                itemColor = itemBGIdleColor;
        }
        jadel::Vec2 itemPos(0.2f, renderable->mainDimensions.y + SpriteSize * itemY - 0.2f);
        screenObjectPushRect(itemPos, itemDim, itemColor, scrObj);
        if (sprite)
            screenObjectPushScreenSurface(itemPos, itemDim, sprite, scrObj);

        submitText(itemPos + jadel::Vec2(SpriteSize + 0.4f, SpriteSize * 0.5f - 0.1f), 4, &currentGame->font, scrObj, (jadel::toString(currentItemNumber++) + ": " + item->gameObject.entity.name).c_str());
        itemY -= 1.2f;
    }
    submitRenderable(scrObj, &uiLayer);

}