//
// Created by berke on 3/11/2026.
//

#ifndef RAYCAST_RENDERER_VECTOR3_H
#define RAYCAST_RENDERER_VECTOR3_H
typedef struct {
    float x, y, z;
} Vector3;

float vector3_length(Vector3 vector);
Vector3 vector3_add(Vector3 vector1, Vector3 vector2);
Vector3 vector3_subtract(Vector3 vector1, Vector3 vector2);

#endif //RAYCAST_RENDERER_VECTOR3_H