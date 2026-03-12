//
// Created by berke on 3/11/2026.
//


#ifndef RAYCAST_RENDERER_RENDERER_H
#define RAYCAST_RENDERER_RENDERER_H

#include <SDL3/SDL.h>

#include "../Objects/Player.h"

#include "Physics.h"

typedef struct {
    SDL_Window *window;
    SDL_Renderer *renderer;
} Renderer;

Renderer create_renderer(SDL_Window *window, SDL_Renderer *renderer);

void begin_frame(const Renderer *renderer);
void end_frame(const Renderer *renderer);
void destroy_renderer(const Renderer *renderer);

void render_walls(const Renderer *renderer, const WallsList* walls);
void render_player(const Renderer *renderer, const Player* player);

void render_draw_grid_line(const Renderer *renderer);

#endif //RAYCAST_RENDERER_RENDERER_H