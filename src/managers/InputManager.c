#include "InputManager.h"
#include "../common/AbyssConfiguration.h"
#include "../common/Globals.h"
#include <assert.h>
#include <string.h>

#define MAX_TEXT_INPUT_LENGTH 100

typedef struct InputManager {
    int     mouse_x;
    int     mouse_y;
    bool    key_pressed[SDL_NUM_SCANCODES];
    char    text_input[MAX_TEXT_INPUT_LENGTH + 1];
    uint8_t mouse_buttons;
} InputManager;

InputManager *input_manager;

void InputManager_InitializeSingleton(void) {
    input_manager = malloc(sizeof(InputManager));
    memset(input_manager, 0, sizeof(InputManager));
}

void InputManager_DestroySingleton(void) {
    assert(input_manager != NULL);

    free(input_manager);
}

bool InputManager_ProcessSdlEvent(SDL_Event *sdl_event) {
    switch (sdl_event->type) {
    case SDL_MOUSEMOTION:
        input_manager->mouse_x = sdl_event->motion.x;
        input_manager->mouse_y = sdl_event->motion.y;
        return true;
    case SDL_MOUSEBUTTONDOWN:
        input_manager->mouse_x        = sdl_event->button.x;
        input_manager->mouse_y        = sdl_event->button.y;
        input_manager->mouse_buttons |= sdl_event->button.button;
        return true;
    case SDL_MOUSEBUTTONUP:
        input_manager->mouse_x        = sdl_event->button.x;
        input_manager->mouse_y        = sdl_event->button.y;
        input_manager->mouse_buttons &= ~sdl_event->button.button;
        return true;
    case SDL_KEYDOWN:
        if (sdl_event->key.keysym.sym == SDLK_RETURN &&
            (sdl_event->key.keysym.mod == KMOD_LALT || sdl_event->key.keysym.mod == KMOD_RALT)) {
            const bool is_fullscreen = AbyssConfiguration_GetFullScreen();
            SDL_SetWindowFullscreen(sdl_window, is_fullscreen ? 0 : SDL_WINDOW_FULLSCREEN_DESKTOP);
            AbyssConfiguration_SetFullScreen(!is_fullscreen);
        }
        // if in text input, handle backspace
        if (strlen(input_manager->text_input) > 0 && sdl_event->key.keysym.sym == SDLK_BACKSPACE) {
            input_manager->text_input[strlen(input_manager->text_input) - 1] = '\0';
        }
        input_manager->key_pressed[sdl_event->key.keysym.scancode] = true;

        return true;
    case SDL_KEYUP:
        input_manager->key_pressed[sdl_event->key.keysym.scancode] = false;
        return true;
    case SDL_TEXTINPUT:
        if (strlen(input_manager->text_input) + strlen(sdl_event->text.text) < MAX_TEXT_INPUT_LENGTH) {
            strcat(input_manager->text_input, sdl_event->text.text);
        }
        return true;
    default:
        return false;
    }
}

void InputManager_StartTextInput(void) {
    memset(input_manager->text_input, 0, sizeof(input_manager->text_input));
    SDL_StartTextInput();
}

void InputManager_StopTextInput(void) {
    memset(input_manager->text_input, 0, sizeof(input_manager->text_input));
    SDL_StopTextInput();
}

bool InputManager_IsTextInputActive(void) { return SDL_IsTextInputActive(); }

void InputManager_GetMousePosition(int *mouse_x, int *mouse_y) {
    if (mouse_x != NULL) {
        *mouse_x = input_manager->mouse_x;
    }

    if (mouse_y != NULL) {
        *mouse_y = input_manager->mouse_y;
    }
}

void InputManager_GetMouseButtons(bool *left, bool *middle, bool *right) {
    if (left != NULL) {
        *left = (input_manager->mouse_buttons & SDL_BUTTON_LEFT) > 0;
    }

    if (middle != NULL) {
        *middle = (input_manager->mouse_buttons & SDL_BUTTON_MIDDLE) > 0;
    }

    if (right != NULL) {
        *right = (input_manager->mouse_buttons & SDL_BUTTON_RIGHT) > 0;
    }
}

void InputManager_ResetMouseButtons(void) { input_manager->mouse_buttons = 0; }
