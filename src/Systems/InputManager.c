//
// Created by berke on 3/11/2026.
//

#include "../../Headers/Systems/InputManager.h"

#include <SDL3/SDL_events.h>
#include <SDL3/SDL_keyboard.h>

void input_manager_begin_frame(InputManager* inputManager) {
    if (!inputManager->keyboardState) inputManager->keyboardState = SDL_GetKeyboardState(0);
    memcpy(inputManager->prevKeyboardState, inputManager->keyboardState, SDL_SCANCODE_COUNT * sizeof(bool));
    SDL_PumpEvents();
}

bool input_manager_get_key(const InputManager* inputManager, SDL_Scancode key) {
    return inputManager->keyboardState[key];
}

bool input_manager_get_key_down(const InputManager* input_manager, SDL_Scancode key) {
    return input_manager->keyboardState[key] && !input_manager->prevKeyboardState[key];
}

bool input_manager_get_key_up(const InputManager* input_manager, SDL_Scancode key) {
    return !input_manager->keyboardState[key] && input_manager->prevKeyboardState[key];
}
