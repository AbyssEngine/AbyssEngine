#include "common/AbyssConfiguration.h"
#include "common/FileManager.h"
#include "common/Globals.h"
#include "common/Logging.h"
#include "drawing/Cursor.h"
#include "drawing/Label.h"
#include "managers/AudioManager.h"
#include "managers/InputManager.h"
#include "managers/VideoManager.h"
#include "scenes/Scene.h"
#include "scenes/SceneIntroVideos.h"
#include "types/Palette.h"
#include "util/Crypto.h"
#include <libavutil/log.h>

int main(int argc, char **argv) {
    (void)(argc);
    (void)(argv);

    Log_SetLevel(LOG_LEVEL_EVERYTHING);
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
    AbyssConfiguration_LoadSingleton(config_path);

    LOG_DEBUG("Initializing file manager...");
    FileManager_CreateSingleton();

    LOG_DEBUG("Creating window...");
    sdl_window = SDL_CreateWindow("Abyss Engine", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                  (int)((float)800 * AbyssConfiguration_GetInitialScale()),
                                  (int)((float)600 * AbyssConfiguration_GetInitialScale()),
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
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, AbyssConfiguration_GetScaleQuality());

    if (AbyssConfiguration_GetFullScreen()) {
        SDL_SetWindowFullscreen(sdl_window, SDL_WINDOW_FULLSCREEN_DESKTOP);
    }

    Palette_Initialize();
    Cursor_CreateSingleton();
    Scene_InitializeManager();
    Cursor_SetType(CURSOR_STANDARD);
    Scene_Set(SCENE_REF(IntroVideos));
    Label_InitializeCaches();
    AudioManager_InitSingleton();
    InputManager_InitializeSingleton();
    VideoManager_InitializeSingleton();

    SDL_Event sdl_event;
    running             = true;
    uint64_t last_ticks = SDL_GetTicks64();

    while (running) {
        while (SDL_PollEvent(&sdl_event)) {
            if (InputManager_ProcessSdlEvent(&sdl_event)) {
                continue;
            }

            switch (sdl_event.type) {
            case SDL_QUIT:
                running = false;
                break;
            }
        }

        if (!running) {
            break;
        }

        const uint64_t current_ticks = SDL_GetTicks64();
        const uint64_t tick_delta    = current_ticks - last_ticks;

        if (tick_delta == 0) {
            SDL_Delay(1);
            continue;
        }

        last_ticks = current_ticks;

        VideoManager_IsPlayingVideo() ? VideoManager_Update(tick_delta) : Scene_UpdateCurrentScene(tick_delta);
        AudioManager_Update();

        SDL_RenderClear(sdl_renderer);
        if (VideoManager_IsPlayingVideo()) {
            VideoManager_Render();
        } else {
            Scene_RenderCurrentScene();
            Cursor_Draw();
        }
        SDL_RenderPresent(sdl_renderer);
    }

    AudioManager_DestroySingleton(); // Destroy this first as it reaches into things via threads
    VideoManager_DestroySingleton();
    InputManager_DestroySingleton();
    Label_FinalizeCaches();
    Scene_DestroyManager();
    Cursor_Destroy();
    Palette_Finalize();
    AbyssConfiguration_DestroySingleton();
    FileManager_DestroySingleton();

    SDL_DestroyRenderer(sdl_renderer);
    SDL_DestroyWindow(sdl_window);
    SDL_Quit();
    return 0;
}
