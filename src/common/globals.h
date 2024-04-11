#ifndef ABYSS_GLOBALS_H
#define ABYSS_GLOBALS_H

#include "resource_paths.h"
#include <SDL2/SDL.h>
#include <stdbool.h>

#define FATAL(MSG)                                                                                                     \
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Fatal Error", MSG, sdl_window);                                    \
    exit(-1)

#define MAX_TEXT_INPUT_LENGTH 100

extern SDL_Window   *sdl_window;
extern SDL_Renderer *sdl_renderer;
extern bool          running;
extern int           mouse_x;
extern int           mouse_y;
extern bool          key_pressed[SDL_NUM_SCANCODES];
extern char          text_input[MAX_TEXT_INPUT_LENGTH + 1];

void start_text_input(void);
void stop_text_input(void);
bool is_in_text_input(void);

#endif // ABYSS_GLOBALS_H
