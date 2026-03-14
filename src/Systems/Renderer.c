
#include <math.h>
#include <stdlib.h>

#include "../../config.h"
#include "../../Headers/Systems/Renderer.h"
#include "../../Headers/Systems/Raycast.h"

Renderer create_renderer(SDL_Window* window, SDL_Renderer* renderer) {
    Renderer r;
    r.window = window;
    r.renderer = renderer;
    return r;
}

void destroy_renderer(const Renderer* renderer) {
    SDL_DestroyRenderer(renderer->renderer);
}

void begin_frame(const Renderer* renderer, SDL_Texture* skyBox, const SDL_FRect* skyDst) {
    SDL_SetRenderDrawColor(renderer->renderer, 255, 255, 255, 255);
    SDL_RenderClear(renderer->renderer);

    if (skyBox != NULL) {
        SDL_RenderTexture(renderer->renderer, skyBox, NULL, skyDst);
    }
}

void end_frame(const Renderer* renderer) {
    SDL_RenderPresent(renderer->renderer);
}

void renderer_draw(
    const TexturesList* texturesList,
    const Player* player,
    const WallsList* walls,
    const Renderer* renderer,
    ObjectsList* objects
) {
    const float fovRadians = FOV * ((float)M_PI / 180.0f);
    const float projectionPlane = SCREEN_WIDTH / 2.0f / tanf(fovRadians / 2.0f);
    const float sliceWidth = (float)SCREEN_WIDTH / (float)RAY_COUNT;
    const float wallWorldHeight = WALL_HEIGHT;

    float columnDepthBuffer[RAY_COUNT];
    static float columnWallDists[RAY_COUNT][MAX_WALL_OVERLAP];
    static float columnWallTops[RAY_COUNT][MAX_WALL_OVERLAP];
    static int columnWallCounts[RAY_COUNT];
    static int columnWallAlphas[RAY_COUNT][MAX_WALL_OVERLAP];

    for (int rayIndex = 0; rayIndex < RAY_COUNT; rayIndex++) {
        columnDepthBuffer[rayIndex]  = 1e9f;
        columnWallCounts[rayIndex]   = 0;
        float screenX = (RAY_COUNT / 2.0f - rayIndex) * sliceWidth;
        float adj = projectionPlane;
        float opp = screenX;
        float ang = atan2f(opp, adj);
        Ray nearestRay = {{player->position.x, player->position.y}};
        const float rayAngle = player->angle + ang;
        const Vector2 dir = { cosf(rayAngle), sinf(rayAngle) };

        ang = atanf( opp / adj );

        RayReturn hits[MAX_WALL_OVERLAP];
        const int hitCount = raycast_collect_hits(
            &nearestRay,
            player,
            dir,
            walls,
            hits,
            MAX_WALL_OVERLAP
        );

        if (hitCount <= 0) {
            continue;
        }

        const int xStart = (int)floorf((float)rayIndex * sliceWidth);
        int xEnd = (int)ceilf((float)(rayIndex + 1) * sliceWidth);
        if (xEnd <= xStart) {
            xEnd = xStart + 1;
        }

        for (int hitIndex = 0; hitIndex < hitCount; hitIndex++) {
            const RayReturn hit = hits[hitIndex];

            float correctedDistance = hit.distance * cosf(ang);
            if (correctedDistance < 0.5f) {
                correctedDistance = 0.001f;
            }

            const float wallHeight = (wallWorldHeight / correctedDistance) * projectionPlane;

            const float horizon = SCREEN_HEIGHT / 2.0f;
            const float y2 = horizon + wallHeight * 0.5f;
            const float y1 = y2 - wallHeight * hit.height;

            const float maxDist = 700.0f;
            const float ambient = 0.4f;

            float t = correctedDistance / maxDist;
            if (t < 0.0f) t = 0.0f;
            if (t > 1.0f) t = 1.0f;

            float brightness = (ambient + (1.0f - ambient) * (1.0f - t)) * AMBIENT;
            brightness *= hit.faceBrightness[hit.side];

            Uint8 r = (Uint8)(hit.r * brightness);
            Uint8 g = (Uint8)(hit.g * brightness);
            Uint8 b = (Uint8)(hit.b * brightness);
            Uint8 a = (Uint8)(hit.a);

            const float textureWidth = texturesList->items[hit.textures[hit.side]].width;
            const float textureHeight = texturesList->items[hit.textures[hit.side]].height;
            SDL_Texture* wallTexture = texturesList->items[hit.textures[hit.side]].texture;

            int texX = (int)(hit.u * textureWidth);
            if (texX < 0) texX = 0;
            if (texX >= (int)textureWidth) texX = (int)textureWidth - 1;

            if (hit.side == 0 || hit.side == 2) {
                texX = (int)textureWidth - 1 - texX;
            }

            SDL_FRect src = { (float)texX, 0.0f, 1.0f, textureHeight };
            SDL_FRect dst = {
                (float)xStart,
                y1,
                (float)(xEnd - xStart),
                y2 - y1
            };

            SDL_SetTextureColorMod(wallTexture, r, g, b);
            SDL_SetTextureAlphaMod(wallTexture, a);
            SDL_RenderTexture(renderer->renderer, wallTexture, &src, &dst);

            if (hit.distance < columnDepthBuffer[rayIndex]) {
                columnDepthBuffer[rayIndex] = hit.distance;
            }
            if (columnWallCounts[rayIndex] < MAX_WALL_OVERLAP) {
                int idx = columnWallCounts[rayIndex]++;
                columnWallDists[rayIndex][idx] = hit.distance;
                columnWallTops[rayIndex][idx]  = y1;
                columnWallAlphas[rayIndex][idx] = a;
            }
        }
    }
    for (int a = 0; a < objects->count - 1; a++) {
        for (int b = a + 1; b < objects->count; b++) {
            float distA = vector2_distance(player->position, objects->items[a].position);
            float distB = vector2_distance(player->position, objects->items[b].position);
            if (distB > distA) {
                Object tmp = objects->items[a];
                objects->items[a] = objects->items[b];
                objects->items[b] = tmp;
            }
        }
    }
    for (int i = 0; i < objects->count; i++) {
        const Object currentObject = objects->items[i];
        const float distance = vector2_distance(player->position, currentObject.position);
        float dx = currentObject.position.x - player->position.x;
        float dy = currentObject.position.y - player->position.y;
        float objectAngle = atan2f(dy, dx);
        float relativeAngle = objectAngle - player->angle;

        while (relativeAngle <= -M_PI) relativeAngle += 2 * M_PI;
        while (relativeAngle > M_PI)  relativeAngle -= 2 * M_PI;

        float screenX = (SCREEN_WIDTH / 2.0f) * (1.0f - tanf(relativeAngle) / tanf(fovRadians / 2.0f));
        float correctedDistance = distance * cosf(relativeAngle);

        float baseHeight    = (WALL_HEIGHT / correctedDistance) * projectionPlane;
        float spriteHeight  = baseHeight * currentObject.scale.y;

        float spriteBottom  = (SCREEN_HEIGHT / 2.0f) + baseHeight / 2.0f; // always fixed
        float spriteTop     = spriteBottom - spriteHeight;

        if (currentObject.texture < 0 || currentObject.texture >= texturesList->count) {
            continue;
        }

        SDL_Texture* spriteTexture = texturesList->items[currentObject.texture].texture;
        if (spriteTexture == NULL) continue;

        float texWidth  = texturesList->items[currentObject.texture].width;
        float texHeight = texturesList->items[currentObject.texture].height;

        float spriteWidth = spriteHeight * (texWidth / texHeight) * currentObject.scale.x;
        float spriteLeft  = screenX - spriteWidth / 2.0f;

        int colStart = (int)floorf(spriteLeft);
        int colEnd   = (int)ceilf(spriteLeft + spriteWidth);

        const float maxDist = 700.0f;
        const float ambient = .4f;
        float fade = correctedDistance / maxDist;
        if (fade < 0.0f) fade = 0.0f;
        if (fade > 1.0f) fade = 1.0f;

        float brightness = (ambient + (1.0f - ambient) * (1.0f - fade)) * AMBIENT;
        Uint8 r = (Uint8)(currentObject.color.x * brightness);
        Uint8 g = (Uint8)(currentObject.color.y * brightness);
        Uint8 b = (Uint8)(currentObject.color.z * brightness);
        Uint8 a = (Uint8)(currentObject.color.q);
        SDL_SetTextureColorMod(spriteTexture, r, g, b);
        SDL_SetTextureAlphaMod(spriteTexture, a);

        for (int col = colStart; col < colEnd; col++) {
            if (col < 0 || col >= SCREEN_WIDTH) continue;

            int bufferIndex = (int)((float)col / ((float)SCREEN_WIDTH / (float)RAY_COUNT));
            if (bufferIndex < 0) bufferIndex = 0;
            if (bufferIndex >= RAY_COUNT) bufferIndex = RAY_COUNT - 1;

            float clipTop = SCREEN_HEIGHT;
            for (int w = 0; w < columnWallCounts[bufferIndex]; w++) {
                if (columnWallDists[bufferIndex][w] < correctedDistance) {
                    if (columnWallAlphas[bufferIndex][w] == 255) {
                        if (columnWallTops[bufferIndex][w] < clipTop) {
                            clipTop = columnWallTops[bufferIndex][w];
                        }
                    }
                }
            }

            float dstTop    = spriteTop;
            float dstBottom = spriteTop + spriteHeight;
            if (dstBottom > clipTop) dstBottom = clipTop;
            if (dstTop >= dstBottom) continue;

            float srcYStart  = (dstTop - spriteTop) / spriteHeight * texHeight;
            float srcYHeight = (dstBottom - dstTop) / spriteHeight * texHeight;

            float t    = (float)(col - colStart) / spriteWidth;
            float srcX = t * texWidth;

            SDL_FRect src = { srcX, srcYStart, 1.0f, srcYHeight };
            SDL_FRect dst = { (float)col, dstTop, 1.0f, dstBottom - dstTop };

            SDL_RenderTexture(renderer->renderer, spriteTexture, &src, &dst);
        }
    }
}
