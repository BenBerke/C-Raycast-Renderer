#include <SDL3/SDL.h>
#define SDL_MAIN_USE_CALLBACKS
#include <stdio.h>
#include <stdlib.h>
#include <SDL3/SDL_main.h>

#include <string.h>

#include "config.h"

typedef struct {
    SDL_Window* window;
    SDL_GPUDevice* device;
    SDL_GPUGraphicsPipeline* pipeline;
    Uint32 numVertices;
    SDL_GPUBuffer* vertexBuffer;
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
    SDL_GPUTransferBufferCreateInfo transferBufferCreateInfo = {
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


SDL_AppResult SDL_AppInit(void** appstate, int argc, char* argv[]) {
    AppState* state = (AppState*)malloc(sizeof(AppState));
    *appstate = state;
    state->window = SDL_CreateWindow("Hello World", SCREEN_WIDTH, SCREEN_HEIGHT, 0);
    if (state->window == NULL) {
        SDL_Log("Window creation failed: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }
    const SDL_GPUShaderFormat formatFlags = SDL_GPU_SHADERFORMAT_SPIRV | SDL_GPU_SHADERFORMAT_DXIL | SDL_GPU_SHADERFORMAT_MSL;
    state->device = SDL_CreateGPUDevice(formatFlags, true, 0);
    if (state->device == NULL) {
        SDL_Log("Unable to create GPU device: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }
    if (!SDL_ClaimWindowForGPUDevice(state->device, state->window)) {
        SDL_Log("Unable to claim window for GPU device: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    if (!create_pipeline(state)) return SDL_APP_FAILURE;

    Vertex vertices[] = {
        {-1.0f, -1.0f, .0f},
            {1.0f, -1.0f, .0f},
            {0.0f, 1.0f, 0.0f},
    };
    if (!create_vertex_buffer(state, vertices, sizeof(vertices)/sizeof(vertices[0]))) return SDL_APP_FAILURE;

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void* appstate, SDL_Event* event){
    switch (event->type)
    {
        case SDL_EVENT_QUIT:
            // Quit the application with a success state
            return SDL_APP_SUCCESS;
        default:
            // Continue running the application
            return SDL_APP_CONTINUE;
    }
}

SDL_AppResult SDL_AppIterate(void* appstate){
    const AppState* state = (AppState*)appstate;
    SDL_GPUCommandBuffer* commandBuffer = SDL_AcquireGPUCommandBuffer(state->device);
    if (commandBuffer == NULL) {
        SDL_Log("Unable to acquire GPU command buffer: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }
    SDL_GPUTexture* swapChainTexture = 0;
    if (!SDL_WaitAndAcquireGPUSwapchainTexture(commandBuffer, state->window, &swapChainTexture, 0, 0)){
        SDL_Log("Unable to wait for GPU swap chain texture: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }
    if (swapChainTexture != NULL) {
        const SDL_FColor clearColor = {1.0f, 0.4f, 1.0f, 1.0f};
        const SDL_GPUColorTargetInfo colorTargetInfo = {
            .texture = swapChainTexture,
            .clear_color = clearColor,
            .load_op = SDL_GPU_LOADOP_CLEAR,
            .store_op = SDL_GPU_STOREOP_STORE,
        };
        SDL_GPURenderPass* renderPass = SDL_BeginGPURenderPass(commandBuffer, &colorTargetInfo, 1, 0);

        SDL_BindGPUGraphicsPipeline(renderPass, state->pipeline);
        SDL_GPUBufferBinding vertexBuffers[] = {
            {
                .buffer = state->vertexBuffer,
                .offset = 0,
            },
        };
        SDL_BindGPUVertexBuffers(renderPass, 0, vertexBuffers, sizeof(vertexBuffers)/sizeof(vertexBuffers[0]));
        SDL_DrawGPUPrimitives(renderPass, state->numVertices, 1, 0, 0);

        SDL_EndGPURenderPass(renderPass);
    }
    SDL_SubmitGPUCommandBuffer(commandBuffer);
    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void* appstate, SDL_AppResult result){
    const AppState* appState = (AppState*)appstate;
    SDL_ReleaseWindowFromGPUDevice(appState->device, appState->window);
    SDL_DestroyWindow(appState->window);
}