//
// Created by berke on 3/14/2026.
//

#ifndef RAYCAST_RENDERER_DEBUGGER_H
#define RAYCAST_RENDERER_DEBUGGER_H

#include "Renderer.h"
#include "../Objects/Light.h"


void render_debug_walls(const Renderer* renderer, const WallsList* walls);
void render_debug_lights(const Renderer* renderer, const LightsList* lights, const WallsList* walls);
void render_debug_player(const Renderer* renderer, const Player* player);
void render_debug_lineOfSight(const Renderer* renderer, const Player* player, const WallsList* walls);


#endif //RAYCAST_RENDERER_DEBUGGER_H