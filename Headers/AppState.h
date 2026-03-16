//
// Created by berke on 3/16/2026.
//

#ifndef RAYCAST_RENDERER_APPSTATE_H
#define RAYCAST_RENDERER_APPSTATE_H
#include <SDL3/SDL_gpu.h>

#include "Systems/InputManager.h"

typedef struct {
    SDL_Window* window;
    SDL_GPUDevice* device;
    SDL_GPUGraphicsPipeline* pipeline;
    Uint32 numVertices;
    Uint32 numIndices;
    SDL_GPUBuffer* vertexBuffer;
    SDL_GPUBuffer* indexBuffer;
    InputManager inputManager;
} AppState;

#endif //RAYCAST_RENDERER_APPSTATE_H