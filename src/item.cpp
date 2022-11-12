#include "item.h"
#include "actor.h"
#include "dice.h"
#include "game.h"

uint32 calculateDamage(DamageRange damageRange)
{
    uint32 result = rollDice(damageRange.numDice, damageRange.numFaces);
    return result;
}

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
    if (item->flags & ITEM_EFFECT_EQUIPPABLE)
    {
        if (item->flags & ITEM_EFFECT_WEAPON)
        {
            if (getPlayerWeaponID() == item->gameObject.entity.id)
                actor->equippedWeapon = NULL;
            else
                actor->equippedWeapon = item;
        }
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
    Item item = createItem(x, y, frames, name, ITEM_EFFECT_ADJUST_HEALTH | ITEM_EFFECT_CONSUMABLE);
    item.healthModifier = healthModifier;
    return item;
}

Item createHealthItem(int x, int y, const GameObjectTemplate* obj, int healthModifier)
{
    Item item = createItem(x, y, obj->frames, obj->name.c_str(), ITEM_EFFECT_ADJUST_HEALTH | ITEM_EFFECT_CONSUMABLE);
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

Item createWeaponItem(int x, int y, AnimFrames frames, const char* name, uint32 numDice, uint32 numFaces)
{
    Item item = createItem(x, y, frames, name, ITEM_EFFECT_WEAPON | ITEM_EFFECT_EQUIPPABLE);
    item.damage = {numDice, numFaces};
    return item;
}