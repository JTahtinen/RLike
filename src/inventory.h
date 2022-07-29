#pragma once
#include "gameobject.h"

struct Item
{
    GameObject gameObject;
    int value;
    uint32 effect;
};

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
};


ItemSlot* askInventorySlot(Inventory* inventory);
void printInventory(const Inventory* inventory);
void updateSubstateInventory();
