#pragma once
#include <jadel.h>
#include "item.h"
#include "screenobject.h"
#include "timer.h"

struct ItemSlot
{
    Item* item;
    bool hasItem;
    bool hovered;
};

struct InventoryRenderable
{
    float margin;
    ScreenObject screenObject;
    jadel::Color headerColor;
    jadel::Vec2 mainDimensions;
    jadel::Vec2 inventoryRadius;
    jadel::Vec2 inventoryCenter;
    uint32 targetOpeningTimeMS;
    uint32 elapsedTimeMS;
    Timer inventoryOpenTimer;
    bool hooked;
    bool inventoryOpened;
};


struct Inventory
{
    ItemSlot itemSlots[10];
    bool useMode;
    bool dropMode;
    bool opening;
    InventoryRenderable renderable;
};

bool initInventory(Inventory* in);
ItemSlot* askInventorySlot(Inventory* inventory);
void printInventory(const Inventory* inventory);
void updateSubstateInventory(Inventory* inventory);
void setInventoryPos(jadel::Vec2 pos, InventoryRenderable *renderable);
InventoryRenderable createInventoryRenderable(jadel::Vec2 pos, jadel::Vec2 dimensions);
void renderInventory(Inventory *inventory);
bool systemInitInventory();