//
// Created by berke on 3/14/2026.
//

#include "../../Headers/Systems/Debugger.h"

#include <math.h>

#include "../../config.h"
#include "../../Headers/Systems/Raycast.h"


void render_debug_walls(const Renderer* renderer, const WallsList* walls) {
    for (int i = 0; i < walls->count; i++) {
        SDL_SetRenderDrawColor(
            renderer->renderer,
            (Uint8)walls->items[i].color.x,
            (Uint8)walls->items[i].color.y,
            (Uint8)walls->items[i].color.z,
            255
        );

        const float x = walls->items[i].position.x;
        const float y = walls->items[i].position.y;
        const float w = walls->items[i].scale.x;
        const float h = walls->items[i].scale.y;

        const float screenX = (x + SCREEN_WIDTH / 2.0f) - w / 2.0f;
        const float screenY = (SCREEN_HEIGHT / 2.0f - y) - h / 2.0f;

        SDL_FRect rect = {screenX, screenY, w, h};
        SDL_RenderFillRect(renderer->renderer, &rect);
    }
}
void render_debug_lights(const Renderer* renderer, const LightsList* lights, const WallsList* walls) {
    for (int i = 0; i < lights->count; i++) {
        const float x = lights->items[i].position.x;
        const float y = lights->items[i].position.y;
        const float screenX = (x + SCREEN_WIDTH / 2.0f) - 10.0f;
        const float screenY = (SCREEN_HEIGHT / 2.0f - y) - 10.0f;

        // draw light rays
        SDL_SetRenderDrawColor(renderer->renderer, 160, 0, 0, 80);
        int rayCount = 0;
        switch (lights->items[i].detailLevel) {
            case MINIMAL: rayCount = 4; break;
            case LOW: rayCount = 90; break;
            case MEDIUM: rayCount = 180; break;
            case HIGH: rayCount = 270; break;
            default: return;
        }
        float angleStep = 2.0f * (float)M_PI / rayCount;
        const float startX = x + SCREEN_WIDTH / 2.0f;
        const float startY = SCREEN_HEIGHT / 2.0f - y;

        for (int r = 0; r < rayCount; r++) {
            float angle = r * angleStep;
            Vector2 dir = { cosf(angle), sinf(angle) };

            float nearestT = lights->items[i].intensity;
            for (int w = 0; w < walls->count; w++) {
                float t = 0.0f;
                int side = -1;
                if (ray_intersect_wall(lights->items[i].position, dir, &walls->items[w], &t, &side)) {
                    if (t > 0.0f && t < nearestT)
                        nearestT = t;
                }
            }

            float endX = startX + dir.x * nearestT;
            float endY = startY - dir.y * nearestT;
            SDL_RenderLine(renderer->renderer, startX, startY, endX, endY);
        }

        // draw light dot on top
        SDL_SetRenderDrawColor(renderer->renderer, 255, 0, 0, 255);
        SDL_FRect rect = {screenX, screenY, 20.0f, 20.0f};
        SDL_RenderFillRect(renderer->renderer, &rect);
    }
}

void render_debug_player(const Renderer* renderer, const Player* player) {
    SDL_SetRenderDrawColor(renderer->renderer, 160, 160, 60, 255);

    const float x = player->position.x;
    const float y = player->position.y;
    const float a = player->scale;

    const float screenX = (x + SCREEN_WIDTH / 2.0f) - a / 2.0f;
    const float screenY = (SCREEN_HEIGHT / 2.0f - y) - a / 2.0f;

    SDL_FRect rect = {screenX, screenY, a, a};
    SDL_RenderFillRect(renderer->renderer, &rect);
}

void render_debug_lineOfSight(const Renderer* renderer, const Player* player, const WallsList* walls) {
    const float fovRadians = FOV * ((float)M_PI / 180.0f);
    const float projectionPlane = SCREEN_WIDTH / 2.0f / tanf(fovRadians / 2.0f);
    const float sliceWidth = (float)SCREEN_WIDTH / (float)RAY_COUNT;

    // Start position (Player center on screen)
    const float startX = player->position.x + SCREEN_WIDTH / 2.0f;
    const float startY = SCREEN_HEIGHT / 2.0f - player->position.y;

    // Set a slightly transparent green for the "fan" effect
    SDL_SetRenderDrawColor(renderer->renderer, 0, 255, 0, 50);

    for (int rayIndex = 0; rayIndex < RAY_COUNT; rayIndex++) {
        // Calculate the angle for this specific ray (same logic as renderer_draw)
        float screenX = (RAY_COUNT / 2.0f - rayIndex) * sliceWidth;
        float ang = atan2f(screenX, projectionPlane);
        const float rayAngle = player->angle + ang;

        Vector2 dir = { cosf(rayAngle), sinf(rayAngle) };

        Ray nearestRay;
        RayReturn hits[MAX_WALL_OVERLAP];

        // Perform the raycast
        raycast_collect_hits(
            &nearestRay,
            player,
            dir,
            walls,
            hits,
            MAX_WALL_OVERLAP
        );

        // Convert the hit point to screen coordinates
        const float endX = nearestRay.position.x + SCREEN_WIDTH / 2.0f;
        const float endY = SCREEN_HEIGHT / 2.0f - nearestRay.position.y;

        // Draw the individual ray
        SDL_RenderLine(renderer->renderer, startX, startY, endX, endY);
    }
}
