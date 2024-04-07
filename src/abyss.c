#include "common/config.h"
#include "common/fileman.h"
#include "common/globals.h"
#include "common/log.h"
#include "drawing/cursor.h"
#include "scenes/scene.h"
#include "scenes/scene_mainmenu.h"
#include "types/palette.h"
#include "util/crypto.h"

int main(int argc, char **argv) {
    log_set_level(LOG_LEVEL_EVERYTHING);
    LOG_INFO("Abyss Engine");

    LOG_DEBUG("Initializing SDL...");
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        FATAL(SDL_GetError());
    }

    LOG_DEBUG("Initializing crypto...");
    crypto_init();

    LOG_DEBUG("Loading configuration...");
    char *config_path = malloc(4096);
    memset(config_path, 0, 4096);
#ifdef _WIN32
    snprintf(config_path, 4096, "%s\\abyss.ini", getenv("APPDATA"));
#elif __linux__
    snprintf(config_path, 4096, "%s/.config/abyss/abyss.ini", getenv("HOME"));
#elif __APPLE__
    snprintf(config_path, 4096, "%s/Library/Application Support/abyss/abyss.ini", getenv("HOME"));
#else
    sprintf(config_path, "abyss.ini");
#endif
    config_load(config_path);
    free(config_path);

    LOG_DEBUG("Initializing file manager...");
    fileman_init();

    LOG_DEBUG("Creating window...");
    sdl_window = SDL_CreateWindow("Abyss Engine", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                  800 * config->graphics.initial_scale, 600 * config->graphics.initial_scale,
                                  SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);

    if (sdl_window == NULL) {
        FATAL(SDL_GetError());
    }

    LOG_DEBUG("Creating renderer...");
    sdl_renderer = SDL_CreateRenderer(sdl_window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    if (sdl_renderer == NULL) {
        FATAL(SDL_GetError());
    }

    SDL_RenderSetLogicalSize(sdl_renderer, 800, 600);
    SDL_ShowCursor(false);
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, config->graphics.scale_quality);

    palette_initialize();

    cursor_initialize();
    scene_initialize();
    cursor_set_type(CURSOR_STANDARD);
    scene_set(&scene_mainmenu);

    SDL_Event sdl_event;
    running             = true;
    uint32_t last_ticks = SDL_GetTicks();

    while (running) {
        while (SDL_PollEvent(&sdl_event)) {
            switch (sdl_event.type) {
            case SDL_QUIT:
                running = false;
                break;
            case SDL_MOUSEMOTION:
                mouse_x = sdl_event.motion.x;
                mouse_y = sdl_event.motion.y;
                break;
            }
        }

        uint32_t current_ticks = SDL_GetTicks();
        uint32_t tick_delta    = current_ticks - last_ticks;

        if (tick_delta == 0) {
            SDL_Delay(1);
            continue;
        }

        last_ticks = current_ticks;

        scene_update(tick_delta);

        SDL_RenderClear(sdl_renderer);
        scene_render();
        cursor_draw();
        SDL_RenderPresent(sdl_renderer);
    }

    scene_finalize();
    cursor_finalize();
    palette_finalize();
    config_free();
    fileman_free();

    SDL_DestroyRenderer(sdl_renderer);
    SDL_DestroyWindow(sdl_window);
    SDL_Quit();
    return 0;
}
