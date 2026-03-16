#include <SDL3/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <SDL3/SDL_main.h>

#include <string.h>

#include "config.h"
#include "Headers/Systems/InputManager.h"

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

typedef struct {
    float x, y, z;
} Vertex;

SDL_GPUShader* load_shader(SDL_GPUDevice* device, const char* fileName) {
    SDL_GPUShaderStage stage;
    if (strstr(fileName, ".vert") != NULL) stage = SDL_GPU_SHADERSTAGE_VERTEX;
    else if (strstr(fileName, ".frag") != NULL) stage = SDL_GPU_SHADERSTAGE_FRAGMENT;
    else {
        SDL_Log("Unable to load shader: %s", SDL_GetError());
        return NULL;
    }
    char fullPath[512];
    snprintf(fullPath, 512, "%s/%s", SDL_GetBasePath(), fileName);
    SDL_GPUShaderFormat format = SDL_GPU_SHADERFORMAT_INVALID;
    const char* entryPoint;
    SDL_GPUShaderFormat backEndFormats = SDL_GetGPUShaderFormats(device);
    if (backEndFormats & SDL_GPU_SHADERFORMAT_SPIRV) {
        snprintf(fullPath, sizeof(fullPath), "%sshaders/%s.spv", SDL_GetBasePath(), fileName);
        format = SDL_GPU_SHADERFORMAT_SPIRV;
        entryPoint = "main";
    }
    else if (backEndFormats & SDL_GPU_SHADERFORMAT_MSL) {
        snprintf(fullPath, sizeof(fullPath), "%sshaders/%s.msl", SDL_GetBasePath(), fileName);
        format = SDL_GPU_SHADERFORMAT_MSL;
        entryPoint = "main0";
    }
    else if (backEndFormats & SDL_GPU_SHADERFORMAT_DXIL) {
        snprintf(fullPath, sizeof(fullPath), "%sshaders/%s.dxil", SDL_GetBasePath(), fileName);
        format = SDL_GPU_SHADERFORMAT_DXIL;
        entryPoint = "main1";
    }
    else {
        SDL_Log("Couldn't find shader %s", SDL_GetGPUDeviceDriver(device));
        return NULL;
    }
    size_t fileSize;
    const void* code = SDL_LoadFile(fullPath, &fileSize);
    if (code == NULL) {
        SDL_Log("Couldn't load file: %s", SDL_GetError());
        return NULL;
    }
    const SDL_GPUShaderCreateInfo shaderInfo = {
        .code_size = fileSize,
        .code = (Uint8*)code,
        .entrypoint = entryPoint,
        .format = format,
        .stage = stage,
    };
    SDL_GPUShader* shader = SDL_CreateGPUShader(device, &shaderInfo);
    if (shader == NULL) {
        SDL_Log("Couldn't create shader from file %s: %s", fullPath, SDL_GetError());
        return NULL;
    }
    SDL_free((void*)code);
    return shader;
}

bool create_pipeline(AppState* state) {
    SDL_GPUShader* vertexShader = load_shader(state->device, "OnlyPosition.vert");
    if (vertexShader == NULL) {
        SDL_Log("Unable to load vertex shader: %s", SDL_GetError());
        return false;
    }
    SDL_GPUShader* fragmentShader = load_shader(state->device, "SolidColor.frag");
    if (fragmentShader == NULL) {
        SDL_Log("Unable to load fragment shader: %s", SDL_GetError());
        return false;
    }
    SDL_GPUVertexBufferDescription vertexDescriptions[] = {
        {.slot = 0,.pitch = sizeof(Vertex), .input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX, .instance_step_rate = 0,},
    };
    SDL_GPUVertexAttribute vertexAttributes[] = {
        {.location = 0, .buffer_slot = 0, .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT, .offset = 0 * sizeof(float)},
        {.location = 1, .buffer_slot = 0, .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT, .offset = 1 * sizeof(float)},
        {.location = 2, .buffer_slot = 0, .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT, .offset = 2 * sizeof(float)},
    };
    SDL_GPUColorTargetDescription colorTargetDescriptions[] = {
        {.format = SDL_GetGPUSwapchainTextureFormat(state->device, state->window)},
    };

    SDL_GPUGraphicsPipelineCreateInfo pipelineCreateInfo = {
        .vertex_shader = vertexShader,
        .fragment_shader = fragmentShader,
        .vertex_input_state = {
            .vertex_buffer_descriptions = vertexDescriptions,
            .num_vertex_buffers = sizeof(vertexDescriptions) / sizeof(vertexDescriptions[0]),
            .vertex_attributes = vertexAttributes,
            .num_vertex_attributes = sizeof(vertexAttributes) / sizeof(vertexAttributes[0]),
        },
        .primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST,
        .rasterizer_state = {
            .fill_mode = SDL_GPU_FILLMODE_FILL,
            .cull_mode = SDL_GPU_CULLMODE_BACK,
            .front_face = SDL_GPU_FRONTFACE_COUNTER_CLOCKWISE,
        },
        .target_info = {
            .color_target_descriptions = colorTargetDescriptions,
            .num_color_targets = sizeof(colorTargetDescriptions) / sizeof(colorTargetDescriptions[0]),
        },
    };
    state->pipeline = SDL_CreateGPUGraphicsPipeline(state->device, &pipelineCreateInfo);
    if (state->pipeline == NULL) {
        SDL_Log("Couldn't create graphics pipeline %s", SDL_GetError());
        return false;
    }
    SDL_ReleaseGPUShader(state->device, vertexShader);
    SDL_ReleaseGPUShader(state->device, fragmentShader);
    return true;
}

bool create_index_buffer(AppState* state, Uint16* indices, int indexCount) {
    state->numIndices = indexCount;
    Uint32 indicesSize = indexCount * sizeof(Uint16);

    SDL_GPUBufferCreateInfo vertexBufferCreateInfo = {
        .usage = SDL_GPU_BUFFERUSAGE_INDEX,
        .size = indicesSize,
    };
    state->indexBuffer = SDL_CreateGPUBuffer(state->device, &vertexBufferCreateInfo);
    if (state->indexBuffer == 0) {
        SDL_Log("Couldn't create index buffer %s", SDL_GetError());
        return false;
    }
    const SDL_GPUTransferBufferCreateInfo transferBufferCreateInfo = {
        .usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
        .size = indicesSize,
    };
    SDL_GPUTransferBuffer* transferBuffer = SDL_CreateGPUTransferBuffer(state->device, &transferBufferCreateInfo);
    if (transferBuffer == NULL) {
        SDL_Log("Couldn't create transfer buffer %s", SDL_GetError());
        return false;
    }
    Uint16* transferData = (Uint16*)SDL_MapGPUTransferBuffer(state->device, transferBuffer, false);
    if (transferData == NULL) {
        SDL_Log("Couldn't create transfer buffer %s", SDL_GetError());
        return false;
    }
    memcpy(transferData, indices, indicesSize);
    SDL_UnmapGPUTransferBuffer(state->device, transferBuffer);

    SDL_GPUCommandBuffer* uploadCmdBuf = SDL_AcquireGPUCommandBuffer(state->device);
    if (uploadCmdBuf == NULL) {
        SDL_Log("Couldn't create command buffer %s", SDL_GetError());
        return false;
    }
    SDL_GPUCopyPass* copyPass =  SDL_BeginGPUCopyPass(uploadCmdBuf);
    SDL_GPUTransferBufferLocation bufferLocation = {
        .transfer_buffer = transferBuffer,
        .offset =  0,
    };
    SDL_GPUBufferRegion bufferRegion = {
        .buffer = state->indexBuffer,
        .offset = 0,
        .size = indicesSize,
    };
    SDL_UploadToGPUBuffer(copyPass, &bufferLocation, &bufferRegion, false);
    SDL_EndGPUCopyPass(copyPass);
    if (!SDL_SubmitGPUCommandBuffer(uploadCmdBuf)) {
        SDL_Log("Couldn't submit command buffer %s", SDL_GetError());
        return false;
    }
    SDL_ReleaseGPUTransferBuffer(state->device, transferBuffer);
    return true;
}

bool create_vertex_buffer(AppState* state, Vertex* vertices, int vertexCount) {
    state->numVertices = vertexCount;
    Uint32 verticesSize = state->numVertices * sizeof(Vertex);
    SDL_GPUBufferCreateInfo vertexBufferCreateInfo = {
        .usage = SDL_GPU_BUFFERUSAGE_VERTEX,
        .size = verticesSize,
    };
    state->vertexBuffer = SDL_CreateGPUBuffer(state->device, &vertexBufferCreateInfo);
    if (state->vertexBuffer == 0) {
        SDL_Log("Couldn't create vertex buffer %s", SDL_GetError());
        return false;
    }
    const SDL_GPUTransferBufferCreateInfo transferBufferCreateInfo = {
        .usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
        .size = verticesSize,
    };
    SDL_GPUTransferBuffer* transferBuffer = SDL_CreateGPUTransferBuffer(state->device, &transferBufferCreateInfo);
    if (transferBuffer == NULL) {
        SDL_Log("Couldn't create transfer buffer %s", SDL_GetError());
        return false;
    }
    Vertex* transferData = (Vertex*)SDL_MapGPUTransferBuffer(state->device, transferBuffer, false);
    if (transferData == NULL) {
        SDL_Log("Couldn't create transfer buffer %s", SDL_GetError());
        SDL_ReleaseGPUTransferBuffer(state->device, transferBuffer);
        return false;
    }
    memcpy(transferData, vertices, verticesSize);
    SDL_UnmapGPUTransferBuffer(state->device, transferBuffer);

    SDL_GPUCommandBuffer* uploadCmdBuf = SDL_AcquireGPUCommandBuffer(state->device);
    if (uploadCmdBuf == NULL) {
        SDL_Log("Couldn't create command buffer %s", SDL_GetError());
        return false;
    }
    SDL_GPUCopyPass* copyPass = SDL_BeginGPUCopyPass(uploadCmdBuf);
    SDL_GPUTransferBufferLocation bufferLocation = {
        .transfer_buffer = transferBuffer,
        .offset = 0,
    };
    SDL_GPUBufferRegion bufferRegion = {
        .buffer = state->vertexBuffer,
        .offset = 0,
        .size = verticesSize,
    };
    SDL_UploadToGPUBuffer(copyPass, &bufferLocation, &bufferRegion, false);
    SDL_EndGPUCopyPass(copyPass);
    if (!SDL_SubmitGPUCommandBuffer(uploadCmdBuf)) {
        SDL_Log("Couldn't submit command buffer %s", SDL_GetError());
        return false;
    }
    SDL_ReleaseGPUTransferBuffer(state->device, transferBuffer);
    return true;
}


int main(int argc, char* argv[]) {
    if (!SDL_Init(SDL_INIT_VIDEO)) return -1;

    AppState state = {0};
    state.window = SDL_CreateWindow("Hello World", SCREEN_WIDTH, SCREEN_HEIGHT, 0);
    state.device = SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_SPIRV | SDL_GPU_SHADERFORMAT_DXIL | SDL_GPU_SHADERFORMAT_MSL, true, NULL);
    SDL_ClaimWindowForGPUDevice(state.device, state.window);

    if (!create_pipeline(&state)) return -1;
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
    create_vertex_buffer(&state, vertices, sizeof(vertices)/sizeof(vertices[0]));
    create_index_buffer(&state, indices, sizeof(indices)/sizeof(indices[0]));

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