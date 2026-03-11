//
// Created by berke on 3/11/2026.
//


#ifndef RAYCAST_RENDERER_RENDERER_H
#define RAYCAST_RENDERER_RENDERER_H

#include <SDL3/SDL.h>

#include "../Objects/Wall.h"

typedef struct {
    SDL_Window *window;
    SDL_Renderer *renderer;
} Renderer;

Renderer create_renderer(SDL_Window *window, SDL_Renderer *renderer);

void begin_frame(const Renderer *renderer);
void end_frame(Renderer *renderer);
void destroy_renderer(Renderer *renderer);

void render_wall(const Renderer *renderer, Wall* wall);

#endif //RAYCAST_RENDERER_RENDERER_H