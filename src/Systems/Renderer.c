//
// Created by berke on 3/11/2026.
//

#include "../../config.h"
#include "../../Headers/Systems/Renderer.h"

Renderer create_renderer(SDL_Window *window, SDL_Renderer *renderer) {
    Renderer r;
    r.window = window;
    r.renderer = renderer;
    return r;
}

void begin_frame(const Renderer *renderer) {
    SDL_Renderer* r = renderer->renderer;
    SDL_SetRenderDrawColor(r, 255, 255, 255, 255);
    SDL_RenderClear(r);
}

void end_frame(Renderer *renderer) {
    SDL_Renderer* r = renderer->renderer;
    SDL_RenderPresent(r);
}

void destroy_renderer(Renderer *renderer) {
    SDL_DestroyRenderer(renderer->renderer);
}

void render_wall(const Renderer *renderer, Wall* wall) {
    SDL_SetRenderDrawColor(renderer->renderer, (Uint8)wall->color.x, (Uint8)wall->color.y, (Uint8)wall->color.z, 255);
    SDL_FRect rect = {wall->position.x, wall->position.y, wall->scale.x, wall->scale.y};
    SDL_RenderFillRect(renderer->renderer, &rect);
}