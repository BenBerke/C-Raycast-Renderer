//
// Created by berke on 3/12/2026.
//

#include "../../Headers/Systems/Raycast.h"

void raycast_move_ray(Raycast* ray) {
    ray->position = vector2_add(ray->position, ray->dir);
}