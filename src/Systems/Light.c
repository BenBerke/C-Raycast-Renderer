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
    // 1. Reset brightness
    for (int i = 0; i < walls->count; i++)
        for (int f = 0; f < 4; f++)
            walls->items[i].faceBrightness[f] = 0.0f;

    for (int i = 0; i < walls->count; i++) {
        Wall* wall = &walls->items[i];
        float distToWall = vector2_distance(light->position, wall->position);
        float effectiveRange = light->distance * light->intensity;

        if (distToWall > effectiveRange) continue;

        // 2. Project the wall's corners into angles relative to the light
        float angleToCorner1 = atan2f(wall->position.y - light->position.y,
                                      wall->position.x - light->position.x);
        // Note: For a precise rectangle, use its actual vertices instead of wall->position

        // 3. Instead of casting random rays, cast rays specifically across the wall's width
        int raysForWall = 16;
        float wallAngularWidth = 0.5f; // This should be calculated based on the wall's size
        float startAngle = angleToCorner1 - (wallAngularWidth / 2.0f);

        for (int r = 0; r < raysForWall; r++) {
            float angle = startAngle + (wallAngularWidth * ((float)r / (float)raysForWall));
            Vector2 dir = { cosf(angle), sinf(angle) };

            float t = 0.0f;
            int side = -1;

            // Check if this specific ray hits the current wall
            if (ray_intersect_wall(light->position, dir, wall, &t, &side)) {
                if (t > 0.0f && t < effectiveRange) {
                    float distanceFactor = 1.0f - (t / effectiveRange);
                    wall->faceBrightness[side] += distanceFactor;
                }
            }
        }
    }

    // 4. Clamp
    for (int i = 0; i < walls->count; i++)
        for (int f = 0; f < 4; f++)
            if (walls->items[i].faceBrightness[f] > 1.0f)
                walls->items[i].faceBrightness[f] = 1.0f;
}

void light_update(LightsList* lights, WallsList* walls) {
    for (int i = 0; i < lights->count; i++) individual_light_update(&lights->items[i], walls);
}