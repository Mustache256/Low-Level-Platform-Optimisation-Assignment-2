#include "Box.h"

void Box::GenRandPos(Box box)
{
    box.position.x = static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / 20.0f));
    box.position.y = 10.0f + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / 1.0f));
    box.position.z = static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / 20.0f));
}

void Box::GenRandVel(Box box)
{
    float randomXVelocity = -1.0f + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / 2.0f));
    box.velocity = {randomXVelocity, 0.0f, 0.0f};
}

void Box::GenRandCol(Box box)
{
    box.colour.x = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
    box.colour.y = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
    box.colour.z = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
}

void Box::SetBoxSize(Box box, float x, float y, float z)
{
    box.size = {x, y, z};
}