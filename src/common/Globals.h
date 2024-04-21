#ifndef ABYSS_GLOBALS_H
#define ABYSS_GLOBALS_H

#include "ResourcePaths.h"
#include <SDL2/SDL.h>
#include <stdbool.h>

#ifndef ABYSS_VERSION_TEXT
#define ABYSS_VERSION_TEXT "local build"
#endif // ABYSS_VERSION_TEXT

extern SDL_Window   *sdl_window;
extern SDL_Renderer *sdl_renderer;
extern bool          running;

#define ABYSS_COPYRIGHT "Abyss Engine is (C) 2024 Timothy Sarbin, released under the MIT license."
#define ABYSS_TEXT_MAIN_MENU_DISCLAIMER                                                                                \
    "Abyss Engine is neither developed by, nor endorsed by Blizzard/Activision or its parent company Microsoft."

#endif // ABYSS_GLOBALS_H
