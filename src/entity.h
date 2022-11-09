#pragma once
#include <jadel.h>

struct Entity
{
    uint32 id;
//    char name[20];
    jadel::String name;
    jadel::Point2i pos;    
};

Entity createEntity(int x, int y);