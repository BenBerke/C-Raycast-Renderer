//
// Created by berke on 3/12/2026.
//

#include "../../Headers/Objects/DebugSquare.h"

void debugSquare_set_position(DebugSquare* square, const Vector2 position) {
    square->position = position;
}
void debugSquare_set_scale(DebugSquare* square, const Vector2 scale) {
    square->scale = scale;
}
void debugSquare_set_color(DebugSquare* square, const Vector3 color) {
    square->color = color;
}
