#include <SDL3/SDL.h>
#include <SDL3/SDL_render.h>
#include <stdbool.h>

#include "config.h"
#include "Headers/Systems/Renderer.h"

#include "Headers/Objects/Wall.h"

int main(int argc, char *argv[])
{
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("SDL Init failed: %s", SDL_GetError());
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("SDL3 Renderer", SCREEN_WIDTH, SCREEN_HEIGHT, 0);
    if (!window) {
        SDL_Log("Window creation failed: %s", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_Renderer* _renderer = SDL_CreateRenderer(window, NULL);
    if (!_renderer) {
        SDL_Log("Renderer creation failed: %s", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }
    Renderer renderer = create_renderer(window, _renderer);

    bool running = true;
    while (running) {

        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                running = false;
            }
            if (event.type == SDL_EVENT_KEY_DOWN) {
                if (event.key.key == SDLK_ESCAPE) { running = false; }
            }
        }

        Wall w = {{100, 100}, {10, 10}, {0, 0, 0}};

        begin_frame(&renderer);

        render_wall(&renderer, &w);

        end_frame(&renderer);
    }

    destroy_renderer(&renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}