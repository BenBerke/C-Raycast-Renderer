//
// Created by berke on 3/12/2026.
//

#ifndef RAYCAST_RENDERER_DEBUGSQUARE_H
#define RAYCAST_RENDERER_DEBUGSQUARE_H

#include "../Tools/Vector2.h"
#include "../Tools/Vector3.h"

typedef struct {
    Vector2 position;
    Vector2 scale;
    Vector3 color;
} DebugSquare;

void debugSquare_set_position(DebugSquare* square, Vector2 position);
void debugSquare_set_scale(DebugSquare* square, Vector2 scale);
void debugSquare_set_color(DebugSquare* square, Vector3 color);

#endif //RAYCAST_RENDERER_DEBUGSQUARE_H