#include "globals.h"
#include "log.h"
#include "config.h"
#include "mpq.h"
#include "crypto.h"
#include "fileman.h"
#include "palette.h"
#include "sprite.h"
#include "cursor.h"

int main(int argc, char** argv) {
    log_set_level(LOG_LEVEL_EVERYTHING);
    LOG_INFO("Abyss Engine");
    
    LOG_DEBUG("Initializing SDL...");
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        FATAL(SDL_GetError());
    }
    
    LOG_DEBUG("Initializing crypto...");
    crypto_init();
    
    LOG_DEBUG("Loading configuration...");
    config_load("abyss.ini");
    
    LOG_DEBUG("Initializing file manager...");
    fileman_init();

    LOG_DEBUG("Creating window...");
    sdl_window = SDL_CreateWindow("Abyss Engine", 
                                  SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED,
                                  800, 600, 
                                  SDL_WINDOW_SHOWN|SDL_WINDOW_RESIZABLE);

    if (sdl_window == NULL) {
        FATAL(SDL_GetError());
    }
    
    LOG_DEBUG("Creating renderer...");
    sdl_renderer = SDL_CreateRenderer(sdl_window, -1, 
                        SDL_RENDERER_ACCELERATED|SDL_RENDERER_PRESENTVSYNC);
    

    if (sdl_renderer == NULL) {
        FATAL(SDL_GetError());
    }

    SDL_RenderSetLogicalSize(sdl_renderer, 800, 600);
    SDL_ShowCursor(false);
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");
    
    palette_initialize();
    
    cursor_initialize();
    cursor_set_type(CURSOR_STANDARD);
    

    sprite_t* test = sprite_load(TRADEMARK_SCREEN, PALETTE_SKY);

    SDL_Event sdl_event;
    running = true;
    while(running) {
        while(SDL_PollEvent(&sdl_event)) {
            switch(sdl_event.type) {
                case SDL_QUIT:
                    running = false;
                    break;
                case SDL_MOUSEMOTION:
                    mouse_x = sdl_event.motion.x;
                    mouse_y = sdl_event.motion.y;
                    break;
            }
        }

        SDL_RenderClear(sdl_renderer);
        sprite_draw_multi(test, 0, 0, 0, 4, 3);
        
        cursor_draw();
        SDL_RenderPresent(sdl_renderer);
    }
       
    sprite_free(test);
    cursor_finalize();
    palette_finalize();
    config_free();
    fileman_free();

    SDL_DestroyRenderer(sdl_renderer);
    SDL_DestroyWindow(sdl_window);
    SDL_Quit();
    return 0;
}
