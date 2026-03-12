//
// Created by berke on 3/12/2026.
//

#ifndef RAYCAST_RENDERER_RAYCAST_H
#define RAYCAST_RENDERER_RAYCAST_H
#include <stdbool.h>

#include "../Tools/Vector2.h"

typedef struct {
    Vector2 position;
    Vector2 dir;

    float distance;
} Raycast;

void raycast_move_ray();

#endif //RAYCAST_RENDERER_RAYCAST_H