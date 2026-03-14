//
// Created by berke on 3/11/2026.
//

#include "../../Headers/Tools/Vector3.h"
#include <math.h>

float vector3_length(const Vector3 vector) {
    return sqrt(vector.x * vector.x + vector.y * vector.y + vector.z * vector.z);
}

Vector3 vector3_add(const Vector3 vector1, const Vector3 vector2) {
    Vector3 result = {vector1.x + vector2.x, vector1.y + vector2.y, vector1.z + vector2.z};
    return result;
}
Vector3 vector3_subtract(Vector3 vector1, Vector3 vector2) {
    Vector3 result = {vector1.x - vector2.x, vector1.y - vector2.y, vector1.z - vector2.z};
    return result;
}