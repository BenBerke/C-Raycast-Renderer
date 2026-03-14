//
// Created by berke on 3/14/2026.
//

#ifndef RAYCAST_RENDERER_LIGHT_H
#define RAYCAST_RENDERER_LIGHT_H

#include "../Tools/Vector2.h"

#include "../../Headers/Systems/Physics.h"

typedef struct {
    Vector2 position;
    float intensity;
    int rayCount;
} Light;



void light_update(Light* light, WallsList* walls);
#endif //RAYCAST_RENDERER_LIGHT_H