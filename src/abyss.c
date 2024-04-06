#include "globals.h"
#include "log.h"
#include "config.h"
#include "mpq.h"
#include "crypto.h"
#include "fileman.h"

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
    
    mpq_stream_t* stream = fileman_load("/data/global/excel/LvlPrest.txt");
    char* data = malloc(mpq_stream_get_size(stream));
    memset(data, 0, mpq_stream_get_size(stream));
    mpq_stream_read(stream, data, 0, mpq_stream_get_size(stream));
    printf("File Data:\n%s", data);
    free(data);
    mpq_stream_free(stream);

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
    
    config_free();
    fileman_free();

    SDL_DestroyRenderer(sdl_renderer);
    SDL_DestroyWindow(sdl_window);
    SDL_Quit();
    return 0;
}
