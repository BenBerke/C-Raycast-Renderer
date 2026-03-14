#ifndef RAYCAST_RENDERER_RAYCAST_H
#define RAYCAST_RENDERER_RAYCAST_H

#include <stdbool.h>

#include "../Tools/Vector2.h"
#include "../Objects/Player.h"
#include "../Objects/Ray.h"
#include "Physics.h"

typedef struct {
    float distance;
    unsigned char r, g, b, a;
    char side;      // 0 top, 1 bottom, 2 left, 3 right
    float u;        // 0.0 -> 1.0 across the wall face
    int textures[4];
    float height;
    float faceBrightness[4];
} RayReturn;

/*
 * Collects up to maxHits wall intersections for one ray.
 * Returns the number of valid hits written into outHits.
 * outHits are sorted from farthest to nearest so you can render in that order.
 * nearestRay is set to the nearest hit point for debug drawing.
 */
int raycast_collect_hits(
    Ray* nearestRay,
    const Player* player,
    Vector2 dir,
    const WallsList* list,
    RayReturn* outHits,
    int maxHits
);

bool ray_intersect_wall(Vector2 origin, Vector2 dir, const Wall* wall, float* outT, int* outSide);

#endif