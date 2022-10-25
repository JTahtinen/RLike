#pragma once
#include "item.h"

struct ItemSlot
{
    Item* item;
    bool hasItem;
};

struct Inventory
{
    ItemSlot itemSlots[10];
    bool useMode = false;
    bool dropMode = false;
    bool opening = false;
};


ItemSlot* askInventorySlot(Inventory* inventory);
void printInventory(const Inventory* inventory);
void updateSubstateInventory();
