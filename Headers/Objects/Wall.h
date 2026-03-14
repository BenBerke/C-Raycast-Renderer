//
// Created by berke on 3/11/2026.
//

#ifndef RAYCAST_RENDERER_WALL_H
#define RAYCAST_RENDERER_WALL_H

#include "../Tools/Vector2.h"
#include "../Tools/Vector3.h"

typedef struct {
    Vector2 position;
    Vector2 scale;
    Vector3 color;
    int texture;
    float height;
} Wall;

#endif //RAYCAST_RENDERER_WALL_H