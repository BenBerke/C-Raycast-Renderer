#include <SDL3/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <SDL3/SDL_main.h>

#include <string.h>

#include "config.h"
#include "Headers/Systems/InputManager.h"
#include "Headers/Systems/Renderer.h"
#include "Headers/AppState.h"

int main(int argc, char* argv[]) {
    if (!SDL_Init(SDL_INIT_VIDEO)) return -1;

    AppState state = {0};
    state.window = SDL_CreateWindow("Hello World", SCREEN_WIDTH, SCREEN_HEIGHT, 0);
    state.device = SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_SPIRV | SDL_GPU_SHADERFORMAT_DXIL | SDL_GPU_SHADERFORMAT_MSL, true, NULL);
    SDL_ClaimWindowForGPUDevice(state.device, state.window);

    if (!renderer_create_pipeline(&state)) return -1;
    Vertex vertices[] = {
        {-0.5f,  0.5f, 0.0f}, // 0: Top Left
        {-0.5f, -0.5f, 0.0f}, // 1: Bottom Left
        { 0.5f, -0.5f, 0.0f}, // 2: Bottom Right
        { 0.5f,  0.5f, 0.0f}, // 3: Top Right
    };

    Uint16 indices[] = {
        0, 1, 2, // First triangle
        0, 2, 3  // Second triangle
    };
    renderer_create_vertex_buffer(&state, vertices, sizeof(vertices)/sizeof(vertices[0]));
    renderer_create_index_buffer(&state, indices, sizeof(indices)/sizeof(indices[0]));

    bool running = true;
    SDL_Event event;

    while (running) {
        input_manager_begin_frame(&state.inputManager);
        if (input_manager_get_key_down(&state.inputManager, SDL_SCANCODE_ESCAPE)) running = false;

        SDL_GPUCommandBuffer* commandBuffer = SDL_AcquireGPUCommandBuffer(state.device);
        SDL_GPUTexture* swapChainTexture;
        if (SDL_WaitAndAcquireGPUSwapchainTexture(commandBuffer, state.window, &swapChainTexture, NULL, NULL)) {
            if (swapChainTexture != NULL) {
                SDL_GPUColorTargetInfo colorTargetInfo = {
                    .texture = swapChainTexture,
                    .clear_color = (SDL_FColor){1.0f, 0.4f, 1.0f, 1.0f},
                    .load_op = SDL_GPU_LOADOP_CLEAR,
                    .store_op = SDL_GPU_STOREOP_STORE,
                };
                SDL_GPURenderPass* renderPass = SDL_BeginGPURenderPass(commandBuffer, &colorTargetInfo, 1, NULL);

                SDL_BindGPUGraphicsPipeline(renderPass, state.pipeline);
                SDL_BindGPUVertexBuffers(renderPass, 0, &(SDL_GPUBufferBinding){.buffer = state.vertexBuffer}, 1);
                SDL_BindGPUIndexBuffer(renderPass, &(SDL_GPUBufferBinding){.buffer = state.indexBuffer}, SDL_GPU_INDEXELEMENTSIZE_16BIT);

                SDL_DrawGPUIndexedPrimitives(renderPass, state.numIndices, 1, 0, 0, 0);

                SDL_EndGPURenderPass(renderPass);
            }
        }
        SDL_SubmitGPUCommandBuffer(commandBuffer);
    }

    SDL_ReleaseWindowFromGPUDevice(state.device, state.window);
    SDL_DestroyWindow(state.window);
    SDL_Quit();
    return 0;
}