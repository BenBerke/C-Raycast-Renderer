//
// Created by berke on 3/11/2026.
//

#ifndef RAYCAST_RENDERER_INPUTMANAGER_H
#define RAYCAST_RENDERER_INPUTMANAGER_H

#include <SDL3/SDL_scancode.h>

typedef struct {
    bool prevKeyboardState[SDL_SCANCODE_COUNT];
    const bool* keyboardState;
} InputManager;

void input_manager_begin_frame(InputManager* inputManager);
bool input_manager_get_key(const InputManager* inputManager, SDL_Scancode key);
bool input_manager_get_key_down(const InputManager* inputManager, SDL_Scancode key);
bool input_manager_get_key_up(const InputManager* inputManager, SDL_Scancode key);

#endif //RAYCAST_RENDERER_INPUTMANAGER_H