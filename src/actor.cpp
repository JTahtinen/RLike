#include "actor.h"
#include "game.h"
#include "dice.h"

void Actor::clearPath()
{
    pathLength = 0;
    pathStepsTaken = 0;
}

int getStrengthModifier(const Actor* actor)
{
    int result = ATTRIB_MODIFIER(actor->attrib.strength);
    return result;
}

int getArmorClassRating(const Actor* actor)
{
    int result = 10 + ATTRIB_MODIFIER(actor->attrib.dexterity);
    return result;
}

bool rollAttackHit(const Actor* attacker, const Actor* attackTarget)
{
    int diceRoll = rollDie(20);
    if (diceRoll == 20) return true;
    if (diceRoll == 1) return false;
    int modifiedAttack = diceRoll + getStrengthModifier(attacker);
    bool result = (modifiedAttack >= getArmorClassRating(attackTarget));
    return result;
}

int rollInitiative(Actor* actor)
{
    int result = rollDie(20) + ATTRIB_MODIFIER(actor->attrib.dexterity);
    return result;
}

int calculateMaxHealth(const Actor* actor)
{
    int classHitPoints = 8; // TODO: define classes to determine this value
    int result = classHitPoints + ATTRIB_MODIFIER(actor->attrib.constitution);
    return result;
}

void initDefaultAttributes(Attributes *attrib)
{
    attrib->strength = 10;
    attrib->dexterity = 10;
    attrib->constitution = 30;
    attrib->intelligence = 10;
    attrib->wisdom = 10;
    attrib->charisma = 10;
}

void setMaxHealth(Actor *actor, int health)
{
    if (health < 1)
        return;
    actor->gameObject.maxHealth = health;
    actor->gameObject.health = health;
}

const jadel::Surface* getCurrentFrame(const Actor* actor)
{
    if (!actor) return NULL;
    const jadel::Surface* result = getCurrentFrame(&actor->gameObject);
    return result;
}

bool setFrame(int index, Actor* actor)
{
    if (!actor) return false;
    setFrame(index, &actor->gameObject);
    return true;
}

void setNextFrame(Actor* actor)
{
    if (!actor) return;
    setNextFrame(&actor->gameObject);
}

Actor createActor(int x, int y, AnimFrames frames, const char *name, Attributes *attrib)
{
    Actor result;
    result.attrib = *attrib;
    result.gameObject = createGameObject(x, y, frames, name);
    setMaxHealth(&result, calculateMaxHealth(&result));
    result.equippedWeapon = NULL;
    result.transit.inTransit = false;
    result.transit.startSector = NULL;
    result.transit.endSector = NULL;
    result.transit.progress = 0;
    result.commandInQueue = false;
    result.pathLength = 0;
    result.pathStepsTaken = 0;
    result.followingPath = false;
    initInventory(&result.inventory);
    return result;
}

Actor createActor(int x, int y, AnimFrames frames, const char *name)
{
    Attributes attrib;
    initDefaultAttributes(&attrib);
    Actor result = createActor(x, y, frames, name, &attrib);
    return result;
}