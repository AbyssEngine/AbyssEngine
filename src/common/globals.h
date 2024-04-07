#ifndef ABYSS_GLOBALS_H
#define ABYSS_GLOBALS_H

#include <SDL2/SDL.h>
#include "resource_paths.h"
#include <stdbool.h>

#define FATAL(MSG)                                                                                                     \
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Fatal Error", MSG, sdl_window);                                    \
    exit(-1);

extern SDL_Window   *sdl_window;
extern SDL_Renderer *sdl_renderer;
extern bool          running;
extern int           mouse_x;
extern int           mouse_y;

#endif // ABYSS_GLOBALS_H
