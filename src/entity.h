#pragma once
#include <jadel.h>

struct Entity
{
    uint32 GUID;
    char name[20];
    jadel::Point2i pos;    
};
