#pragma once
#include <cmath>

class Vec3
{
public:
    float x, y, z;

    //Initialises zero vector if no parameters are entered when calling constructor, otherwise initialises with passed in values
    Vec3() : x(0.0f), y(0.0f), z(0.0f) {}
    Vec3(float x, float y, float z) : x(x), y(y), z(z) {}
    ~Vec3() {};

    //Allows for vectors to be subtracted from each other
    Vec3 operator-(const Vec3& other) const
    {
        return Vec3(x - other.x, y - other.y, z - other.z);
    }

    void normalise();

    float length() const;
};