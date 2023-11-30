#include "Vec3.h"

void Vec3::normalise()
{
    //Normalises this vector
    float length = std::sqrt(x * x + y * y + z * z);
        if (length != 0) {
            x /= length;
            y /= length;
            z /= length;
        }
}

float Vec3::length() const
{
    //Returns length of vector
    return std::sqrt(x * x + y * y + z * z);
}