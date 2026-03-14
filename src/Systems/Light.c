//
// Created by berke on 3/14/2026.
//

#include "../../Headers/Objects/Light.h"
#include "../../Headers/Systems/Raycast.h"

#include <math.h>

void light_update(Light* light, WallsList* walls) {
    for (int i = 0; i < walls->count; i++)
        for (int f = 0; f < 4; f++)
            walls->items[i].faceBrightness[f] = 0.0f;

    float angleStep = 2.0f * (float)M_PI / (float)light->rayCount;

    for (int r = 0; r < light->rayCount; r++) {
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