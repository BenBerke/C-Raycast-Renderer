#ifndef RAYCAST_RENDERER_RENDERER_H
#define RAYCAST_RENDERER_RENDERER_H

#include <SDL3/SDL.h>

#include "../Objects/Player.h"
#include "../Objects/DebugSquare.h"
#include "../Objects/Object.h"
#include "Physics.h"
#include "TextureManager.h"

typedef struct {
    SDL_Window* window;
    SDL_Renderer* renderer;
} Renderer;


Renderer create_renderer(SDL_Window* window, SDL_Renderer* renderer);
void destroy_renderer(const Renderer* renderer);

void begin_frame(const Renderer* renderer, SDL_Texture* skyBox, const SDL_FRect* skyDst);
void end_frame(const Renderer* renderer);

void renderer_draw(
    const TexturesList* texturesList,
    const Player* player,
    const WallsList* walls,
    const Renderer* renderer,
    ObjectsList* objects
);

void renderer_draw_objects(ObjectsList* objectList, Player* p);

#endif