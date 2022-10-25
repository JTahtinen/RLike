#include "actor.h"
#include "game.h"

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
    int diceRoll = (rand() % 20) + 1;
    if (diceRoll == 20) return true;
    if (diceRoll == 1) return false;
    int modifiedAttack = diceRoll + getStrengthModifier(attacker);
    bool result = (modifiedAttack >= getArmorClassRating(attackTarget));
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
    attrib->constitution = 10;
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

Actor createActor(int x, int y, AnimFrames frames, const char *name, Attributes *attrib)
{
    Actor result;
    result.attrib = *attrib;
    result.gameObject = createGameObject(x, y, frames, name);
    setMaxHealth(&result, calculateMaxHealth(&result));
    result.transit.inTransit = false;
    result.transit.startSector = NULL;
    result.transit.endSector = NULL;
    result.transit.progress = 0;
    result.commandInQueue = false;
    result.pathLength = 0;
    result.pathStepsTaken = 0;
    result.followingPath = false;
    for (int i = 0; i < 10; ++i)
    {
        result.inventory.itemSlots[i].hasItem = false;
    }
    return result;
}

Actor createActor(int x, int y, AnimFrames frames, const char *name)
{
    Attributes attrib;
    initDefaultAttributes(&attrib);
    Actor result = createActor(x, y, frames, name, &attrib);
    return result;
}