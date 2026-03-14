//
// Created by berke on 3/14/2026.
//

#include "../../Headers/Tools/Vector4.h"

#include <math.h>

Vector4 vector4_add(const Vector4 vector1, const Vector4 vector2) {
    Vector4 result = {vector1.x + vector2.x, vector1.y + vector2.y, vector1.z + vector2.z};
    return result;
}
Vector4 vector4_subtract(Vector4 vector1, Vector4 vector2) {
    Vector4 result = {vector1.x - vector2.x, vector1.y - vector2.y, vector1.z - vector2.z};
    return result;
}
