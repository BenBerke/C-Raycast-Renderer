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
void render_debug_light(const Renderer* renderer, const WallsList* walls) {
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

        const float screenX = (x - SCREEN_WIDTH / 2.0f) - w / 2.0f;
        const float screenY = (SCREEN_HEIGHT / 2.0f - y) - h / 2.0f;

        SDL_FRect rect = {screenX, screenY, w, h};
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