#pragma once
#include "Vec3.h"

class Box
{
public:
    Box(){};
    ~Box(){};

    Vec3 position;
    Vec3 size;
    Vec3 velocity;
    Vec3 colour; 

    void GenRandPos(Box box);
    void GenRandVel(Box box);
    void GenRandCol(Box box);
    void SetBoxSize(Box box, float x, float y, float z);
};
