#include "dice.h"

uint32 rollDie(uint32 numFaces)
{
    uint32 result = rand() % numFaces + 1;
    return result;
}

uint32 rollDice(uint32 numDice, uint32 numFaces)
{
    uint32 result = 0;
    for (uint32 i = 0; i < numDice; ++i)
    {
        result += rollDie(numDice);
    }
    return result; 
}