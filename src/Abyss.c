#include "audio/AudioManager.h"
#include "common/AbyssConfiguration.h"
#include "common/FileManager.h"
#include "common/Globals.h"
#include "common/Logging.h"
#include "drawing/Cursor.h"
#include "drawing/Label.h"
#include "scenes/Scene.h"
#include "scenes/SceneMainMenu.h"
#include "types/Palette.h"
#include "util/Crypto.h"
#include <libavutil/log.h>

int main(int argc, char **argv) {
    log_set_level(LOG_LEVEL_EVERYTHING);
    av_log_set_level(AV_LOG_FATAL);
    LOG_INFO("Abyss Engine");

    LOG_DEBUG("Initializing SDL...");
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        FATAL(SDL_GetError());
    }

    LOG_DEBUG("Initializing crypto...");
    crypto_init();

    LOG_DEBUG("Loading configuration...");
    char config_path[4096];
    memset(config_path, 0, 4096);
#ifdef _WIN32
    snprintf(config_path, 4096, "%s\\abyss\\abyss.ini", getenv("APPDATA"));
#elif __APPLE__
    snprintf(config_path, 4096, "%s/Library/Application Support/abyss/abyss.ini", getenv("HOME"));
#else
    snprintf(config_path, 4096, "%s/.config/abyss/abyss.ini", getenv("HOME"));
#endif
    abyss_configuration_load(config_path);

    LOG_DEBUG("Initializing file manager...");
    file_manager_init();

    LOG_DEBUG("Creating window...");
    sdl_window = SDL_CreateWindow("Abyss Engine", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                  (int)((float)800 * abyss_configuration.graphics.initial_scale),
                                  (int)((float)600 * abyss_configuration.graphics.initial_scale),
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
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, abyss_configuration.graphics.scale_quality);

    if (abyss_configuration.graphics.fullscreen) {
        SDL_SetWindowFullscreen(sdl_window, SDL_WINDOW_FULLSCREEN_DESKTOP);
    }

    palette_initialize();
    cursor_initialize();
    scene_initialize();
    cursor_set_type(CURSOR_STANDARD);
    scene_set(&scene_main_menu);
    label_initialize_caches();
    audio_manager_init();

    SDL_Event sdl_event;
    running             = true;
    uint64_t last_ticks = SDL_GetTicks64();

    while (running) {
        while (SDL_PollEvent(&sdl_event)) {
            switch (sdl_event.type) {
            case SDL_QUIT:
                running = false;
                continue;
            case SDL_MOUSEMOTION:
                mouse_x = sdl_event.motion.x;
                mouse_y = sdl_event.motion.y;
                continue;
            case SDL_KEYDOWN:
                if (sdl_event.key.keysym.sym == SDLK_F4 && sdl_event.key.keysym.mod == KMOD_LALT) {
                    running = false;
                }
                if (sdl_event.key.keysym.sym == SDLK_RETURN &&
                    (sdl_event.key.keysym.mod == KMOD_LALT || sdl_event.key.keysym.mod == KMOD_RALT)) {
                    SDL_SetWindowFullscreen(
                        sdl_window, abyss_configuration.graphics.fullscreen ? 0 : SDL_WINDOW_FULLSCREEN_DESKTOP);
                    abyss_configuration.graphics.fullscreen = !abyss_configuration.graphics.fullscreen;
                }
                // if in text input, handle backspace
                if (strlen(text_input) > 0 && sdl_event.key.keysym.sym == SDLK_BACKSPACE) {
                    text_input[strlen(text_input) - 1] = '\0';
                }
                key_pressed[sdl_event.key.keysym.scancode] = true;

                continue;
            case SDL_KEYUP:
                key_pressed[sdl_event.key.keysym.scancode] = false;
                continue;
            case SDL_TEXTINPUT:
                if (strlen(text_input) + strlen(sdl_event.text.text) < MAX_TEXT_INPUT_LENGTH) {
                    strcat(text_input, sdl_event.text.text);
                }
                continue;
            }
        }

        const uint64_t current_ticks = SDL_GetTicks64();
        const uint64_t tick_delta    = current_ticks - last_ticks;

        if (tick_delta == 0) {
            SDL_Delay(1);
            continue;
        }

        last_ticks = current_ticks;

        scene_update(tick_delta);
        audio_manager_update();

        SDL_RenderClear(sdl_renderer);
        scene_render();
        cursor_draw();
        SDL_RenderPresent(sdl_renderer);
    }

    audio_manager_free();
    label_finalize_caches();
    scene_finalize();
    cursor_finalize();
    palette_finalize();
    abyss_configuration_free();
    file_manager_free();

    SDL_DestroyRenderer(sdl_renderer);
    SDL_DestroyWindow(sdl_window);
    SDL_Quit();
    return 0;
}
