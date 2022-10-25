#include "item.h"
#include "actor.h"

void useItem(Item *item, Actor *actor)
{
    if (!item || !actor) return;
    GameObject *actorObj = &actor->gameObject;
    if (item->flags & ITEM_EFFECT_ADJUST_HEALTH)
    {
        actorObj->health += item->healthModifier;
        if (actorObj->health > actorObj->maxHealth)
            actorObj->health = actorObj->maxHealth;
    }
}

Item createItem(int x, int y, AnimFrames frames, const char *name, uint32 flags)
{
    Item result;
    result.gameObject = createGameObject(x, y, frames, name);
    result.flags = flags;
    return result;
}

Item createHealthItem(int x, int y, AnimFrames frames, const char *name, int healthModifier)
{
    Item item = createItem(x, y, frames, name, ITEM_EFFECT_ADJUST_HEALTH);
    item.healthModifier = healthModifier;
    return item;
}

Item createIlluminatorItem(int x, int y, AnimFrames frames, const char *name, float illumination)
{
    Item item = createItem(x, y, frames, name, ITEM_EFFECT_ILLUMINATE);
    item.illumination = illumination;
    item.distanceFromGround = 10;
    return item;
}

