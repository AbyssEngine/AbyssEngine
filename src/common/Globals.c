#include "Globals.h"

SDL_Window   *sdl_window                            = NULL;
SDL_Renderer *sdl_renderer                          = NULL;
bool          running                               = false;
int           mouse_x                               = 0;
int           mouse_y                               = 0;
bool          key_pressed[SDL_NUM_SCANCODES]        = {0};
char          text_input[MAX_TEXT_INPUT_LENGTH + 1] = {0};

void start_text_input(void) {
    memset(text_input, 0, sizeof(text_input));
    SDL_StartTextInput();
}

void stop_text_input(void) {
    memset(text_input, 0, sizeof(text_input));
    SDL_StopTextInput();
}

bool is_in_text_input(void) { return SDL_IsTextInputActive(); }
