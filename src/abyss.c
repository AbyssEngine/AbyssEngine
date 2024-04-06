#include "globals.h"
#include "log.h"
#include "config.h"
#include "mpq.h"
#include "crypto.h"
#include "mpq_stream.h"

int main(int argc, char** argv) {
    log_set_level(LOG_LEVEL_EVERYTHING);
    LOG_INFO("Abyss Engine");
    
    crypto_init();
    
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        FATAL(SDL_GetError());
    }
    
    config_t *config = config_load("abyss.ini");

    sdl_window = SDL_CreateWindow("Abyss Engine", 
                                  SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED,
                                  800, 600, 
                                  SDL_WINDOW_SHOWN|SDL_WINDOW_RESIZABLE);

    if (sdl_window == NULL) {
        FATAL(SDL_GetError());
    }
        
    sdl_renderer = SDL_CreateRenderer(sdl_window, -1, 
                        SDL_RENDERER_ACCELERATED|SDL_RENDERER_PRESENTVSYNC);
    

    if (sdl_renderer == NULL) {
        FATAL(SDL_GetError());
    }

    SDL_RenderSetLogicalSize(sdl_renderer, 800, 600);
    
    mpq_t* mpq = mpq_load(config->mpqs[5]);
    mpq_stream_t* stream = mpq_stream_create(mpq, "(listfile)");
    mpq_stream_free(stream);
    mpq_free(mpq);

    SDL_Event sdl_event;
    running = true;
    while(running) {
        while(SDL_PollEvent(&sdl_event)) {
            switch(sdl_event.type) {
                case SDL_QUIT:
                    running = false;
                    break;
            }
        }

        SDL_RenderClear(sdl_renderer);
        SDL_RenderPresent(sdl_renderer);
    }
    
    config_free(config);

    SDL_DestroyRenderer(sdl_renderer);
    SDL_DestroyWindow(sdl_window);
    SDL_Quit();
    return 0;
}
