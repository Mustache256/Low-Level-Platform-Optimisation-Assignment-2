#pragma once
#include <cmath>

class Vec3
{
public:
    float x, y, z;

    Vec3() : x(0.0f), y(0.0f), z(0.0f) {}
    Vec3(float x, float y, float z) : x(x), y(y), z(z) {}
    ~Vec3() {};

    Vec3 operator-(const Vec3& other) const
    {
        return Vec3(x - other.x, y - other.y, z - other.z);
    }

    void normalise();

    float length() const;
};