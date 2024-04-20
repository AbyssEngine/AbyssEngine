#ifndef ABYSS_INPUT_MANAGER_H
#define ABYSS_INPUT_MANAGER_H

#include <SDL2/SDL.h>
#include <stdbool.h>
#include <stdint.h>

void InputManager_InitializeSingleton(void);
void InputManager_DestroySingleton(void);
bool InputManager_ProcessSdlEvent(SDL_Event *sdl_event);
void InputManager_StartTextInput(void);
void InputManager_StopTextInput(void);
bool InputManager_IsTextInputActive(void);
void InputManager_GetMousePosition(int *mouse_x, int *mouse_y);
void InputManager_GetMouseButtons(bool *left, bool *middle, bool *right);
void InputManager_ResetMouseButtons(void);

#endif // ABYSS_INPUT_MANAGER_H
