#include "Vec3.h"

void Vec3::normalise()
{
    float length = std::sqrt(x * x + y * y + z * z);
        if (length != 0) {
            x /= length;
            y /= length;
            z /= length;
        }
}

float Vec3::length() const
{
    return std::sqrt(x * x + y * y + z * z);
}