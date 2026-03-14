//
// Created by berke on 3/14/2026.
//

#ifndef RAYCAST_RENDERER_VECTOR4_H
#define RAYCAST_RENDERER_VECTOR4_H
typedef struct {
    float x, y, z, q;
} Vector4;

float vector4_length(Vector4 vector);
Vector4 vector4_add(Vector4 vector1, Vector4 vector2);
Vector4 vector4_subtract(Vector4 vector1, Vector4 vector2);

#endif //RAYCAST_RENDERER_VECTOR4_H