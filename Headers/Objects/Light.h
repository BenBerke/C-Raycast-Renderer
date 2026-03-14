//
// Created by berke on 3/14/2026.
//

#ifndef RAYCAST_RENDERER_LIGHT_H
#define RAYCAST_RENDERER_LIGHT_H

#include "../Tools/Vector2.h"

#include "../../Headers/Systems/Physics.h"

typedef struct {
    Vector2 position;
    float distance;
    float intensity;
} Light;

typedef struct {
    Light* items;
    int count;
    int size;
    int chunkSize;
} LightsList;


void lights_create_lights_list(LightsList* list, int chunkSize);
void light_push_lights_list(LightsList* list, const Light* value);
void light_pop_lights_list(LightsList* list);
void light_free_lights_list(LightsList* list);

void light_update(LightsList* light, WallsList* walls);
#endif //RAYCAST_RENDERER_LIGHT_H