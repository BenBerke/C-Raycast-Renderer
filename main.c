#include <SDL3/SDL.h>
#include <SDL3/SDL_render.h>

#include "config.h"
#include "Headers/Systems/Renderer.h"
#include "Headers/Systems/InputManager.h"

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

    InputManager inputManager;


    Wall w = {{100, 100}, {10, 10}, {0, 0, 0}};

    bool running = true;
    while (running) {
        input_manager_begin_frame(&inputManager);
        if (input_manager_get_key_down(&inputManager,SDL_SCANCODE_ESCAPE)) running = false;
        begin_frame(&renderer);

        render_wall(&renderer, &w);

        end_frame(&renderer);
    }

    destroy_renderer(&renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}