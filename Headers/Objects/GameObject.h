//
// Created by berke on 3/14/2026.
//

#ifndef RAYCAST_RENDERER_SPRITE_H
#define RAYCAST_RENDERER_SPRITE_H

#include "../Tools/Vector2.h"
#include "../Tools/Vector3.h"
#include "../Systems/TextureManager.h"

typedef struct {
    Vector2 position;
    Vector2 size;
    Vector3 color;
    Texture* texture;
} Object;

#endif //RAYCAST_RENDERER_SPRITE_H