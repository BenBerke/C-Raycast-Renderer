#include "../../Headers/Systems/Renderer.h"

#include <stdio.h>
#include <SDL3/SDL_filesystem.h>
#include <SDL3/SDL_log.h>

SDL_GPUShader* renderer_load_shader(SDL_GPUDevice* device, const char* fileName) {
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

bool renderer_create_pipeline(AppState* state) {
    SDL_GPUShader* vertexShader = renderer_load_shader(state->device, "OnlyPosition.vert");
    if (vertexShader == NULL) {
        SDL_Log("Unable to load vertex shader: %s", SDL_GetError());
        return false;
    }
    SDL_GPUShader* fragmentShader = renderer_load_shader(state->device, "SolidColor.frag");
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

bool renderer_create_index_buffer(AppState* state, Uint16* indices, int indexCount) {
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

bool renderer_create_vertex_buffer(AppState* state, Vertex* vertices, int vertexCount) {
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