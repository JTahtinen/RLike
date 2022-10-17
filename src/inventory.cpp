#include "inventory.h"
#include "game.h"

void closeInventory(Inventory* inventory)
{
       currentGame->currentState = SUBSTATE_GAME;
       inventory->useMode = false;
       printf("Inventory closed\n");
}

void printInventory(const Inventory* inventory)
{
    printf("Inventory: \n");
    int j = 1;
    bool inventoryEmpty = true;
    for (int i = 0; i < 10; ++i)
    {
        if (currentGame->player.inventory.itemSlots[i].hasItem)
        {
            inventoryEmpty = false;
            printf("%d: %s\n", j++, inventory->itemSlots[i].item->gameObject.entity.name);
        }
    }
    if (inventoryEmpty)
    {
        printf("Empty\n");
    }
}

ItemSlot* askInventorySlot(Inventory* inventory)
{
    ItemSlot* slot = NULL;

    for (int i = 1; i < 10; ++i)
    {
        int j = 0;
        if (jadel::inputIsKeyTyped(jadel::KEY_0 + i))
        {               
            for (int itemIndex = 0; itemIndex < 10; ++itemIndex)
            {
                if (currentGame->player.inventory.itemSlots[itemIndex].hasItem)
                {
                    ++j;
                    if (j == i)
                    {
                        slot = &currentGame->player.inventory.itemSlots[itemIndex];
                        break;
                    }
                }
            }                   
                    
        }
    }
    return slot;
}

void updateSubstateInventory()
{
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
            printf("Use item (1-9)\n");
        }
    }
    if (jadel::inputIsKeyTyped(jadel::KEY_D))
    {
        currentGame->player.inventory.dropMode = !currentGame->player.inventory.dropMode;
        if (currentGame->player.inventory.dropMode)
        {
            printf("Drop item (1-9)\n");
        }
    }
    if (currentGame->player.inventory.useMode)
    {
        ItemSlot* slot = askInventorySlot(&currentGame->player.inventory);
        if (slot)
        {
            Item* item = slot->item;
            GameObject* pObj = &currentGame->player.gameObject;
            switch (item->effect)
            {
                case ADJUST_HEALTH:
                    pObj->health += item->value;
                    if (pObj->health > 100) pObj->health = pObj->maxHealth;
                    printf("You used %s\n", item->gameObject.entity.name);
                    printf("Health: %d / %d\n",
                           pObj->health,
                           pObj->maxHealth);                                        
            }
            slot->hasItem = false;
            printInventory(&currentGame->player.inventory);
 
        }
    }
    if (currentGame->player.inventory.dropMode)
    {
        ItemSlot* slot = askInventorySlot(&currentGame->player.inventory);
        if (slot)
        {
            currentGame->player.inventory.dropMode = false;
            slot->hasItem = false;
            Sector* currentSector = getSectorOfActor(&currentGame->player);
            addSectorItem(currentSector, slot->item);
            printf("Dropped %s\n", slot->item->gameObject.entity.name);
            printInventory(&currentGame->player.inventory);
        }
    }
}
