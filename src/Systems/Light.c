//
// Created by berke on 3/14/2026.
//

#include "../../Headers/Objects/Light.h"
#include "../../Headers/Systems/Raycast.h"

#include <math.h>
#include <stdlib.h>

void lights_create_lights_list(LightsList* list, int chunkSize) {
    list->count = 0;
    list->size = chunkSize;
    list->chunkSize = chunkSize;
    list->items = malloc((size_t)list->size * sizeof(*list->items));
}

void light_push_lights_list(LightsList* list, const Light* value) {
    if (++list->count > list->size) {
        list->size += list->chunkSize;
        list->items = realloc(list->items, (size_t)list->size * sizeof(*list->items));
    }
    list->items[list->count - 1] = *value;
}

void light_pop_lights_list(LightsList* list) {
    if (list->count <= 0) {
        return;
    }

    list->count--;

    if (list->size - list->count >= list->chunkSize && list->size > list->chunkSize) {
        list->size -= list->chunkSize;
        list->items = realloc(list->items, (size_t)list->size * sizeof(*list->items));
    }
}

void light_free_lights_list(LightsList* list) {
    free(list->items);
    list->items = NULL;
    list->count = 0;
    list->size = 0;
    list->chunkSize = 0;
}

void individual_light_update(Light* light, WallsList* walls) {
    for (int i = 0; i < walls->count; i++)
        for (int f = 0; f < 4; f++)
            walls->items[i].faceBrightness[f] = 0.0f;

    int rayCount = 0;
    switch (light->detailLevel) {
        case MINIMAL: rayCount = 4; break;
        case LOW: rayCount = 90; break;
        case MEDIUM: rayCount = 180; break;
        case HIGH: rayCount = 270; break;
            default: return;
    }

    float angleStep = 2.0f * (float)M_PI / (float)rayCount;

    for (int r = 0; r < rayCount; r++) {
        float angle = r * angleStep;
        Vector2 dir = { cosf(angle), sinf(angle) };

        float nearestT = 1e9f;
        Wall* nearestWall = NULL;
        int nearestSide = -1;

        for (int i = 0; i < walls->count; i++) {
            float t = 0.0f;
            int side = -1;
            if (ray_intersect_wall(light->position, dir, &walls->items[i], &t, &side)) {
                if (t > 0.0f && t < nearestT) {
                    nearestT = t;
                    nearestWall = &walls->items[i];
                    nearestSide = side;
                }
            }
        }

        if (nearestWall != NULL && nearestSide >= 0) {
            float falloff = 1.0f - (nearestT / light->intensity);
            if (falloff > 0.0f)
                nearestWall->faceBrightness[nearestSide] += falloff;
        }
    }

    // normalize by max so closest wall = 1.0, others scale relative to it
    float maxB = 0.0f;
    for (int i = 0; i < walls->count; i++)
        for (int f = 0; f < 4; f++)
            if (walls->items[i].faceBrightness[f] > maxB)
                maxB = walls->items[i].faceBrightness[f];

    if (maxB > 0.0f)
        for (int i = 0; i < walls->count; i++)
            for (int f = 0; f < 4; f++)
                walls->items[i].faceBrightness[f] /= maxB;
}

void light_update(LightsList* lights, WallsList* walls) {
    for (int i = 0; i < lights->count; i++) individual_light_update(&lights->items[i], walls);
}