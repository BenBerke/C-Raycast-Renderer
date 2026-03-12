#ifndef RAYCAST_RENDERER_RAYCAST_H
#define RAYCAST_RENDERER_RAYCAST_H

#include "../Tools/Vector2.h"
#include "../Objects/Player.h"
#include "../Objects/Ray.h"

typedef struct WallsList WallsList;

float raycast_create_ray(Ray* r, Player* p, Vector2 dir, const WallsList* list);
void raycast_move_ray(Ray* r, Vector2 dir, float speed);

#endif