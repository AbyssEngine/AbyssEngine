#ifndef ABYSS_GLOBALS_H
#define ABYSS_GLOBALS_H

#include <SDL2/SDL.h>
#include <stdbool.h>

#define FATAL(MSG) SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, \
        "Fatal Error", MSG, sdl_window); exit(-1);


#define LOCAL_LANGUAGE                  "/data/local/use"
#define LANGUAGE_FONT_TOKEN             "{LANG_FONT}"
#define LAGUAGE_TOKEN_TABLE             "{LANG}"

#define CURSOR_DEFAULT                  "/data/global/ui/CURSOR/ohand.DC6"

// -- Main Menu
#define TRADEMARK_SCREEN                "/data/global/ui/FrontEnd/trademarkscreenEXP.dc6"
#define GAME_SELECT_SCREEN              "/data/global/ui/FrontEnd/gameselectscreenEXP.dc6"
#define D2LOGO_BLACK_LEFT               "/data/global/ui/FrontEnd/D2logoBlackLeft.DC6"
#define D2LOGO_BLACK_RIGHT              "/data/global/ui/FrontEnd/D2logoBlackRight.DC6"
#define D2LOGO_FIRE_LEFT                "/data/global/ui/FrontEnd/D2logoFireLeft.DC6"
#define D2LOGO_FIRE_RIGHT               "/data/global/ui/FrontEnd/D2logoFireRight.DC6"

// --

#define LOADING_SCREEN                  "/data/global/ui/loading/loadingscreen.dc6"

#define PALETTE_PATH                    "/data/global/palette/%s/pal.dat"

#define PALETTE_ACT_1                   "act1"
#define PALETTE_ACT_2                   "act2"
#define PALETTE_ACT_3                   "act3"
#define PALETTE_ACT_4                   "act4"
#define PALETTE_ACT_5                   "act5"
#define PALETTE_END_GAME                "endgame"
#define PALETTE_END_GAME_2              "endgame2"
#define PALETTE_FECHAR                  "fechar"
#define PALETTE_LOADING                 "loading"
#define PALETTE_MENU_0                  "menu0"
#define PALETTE_MENU_1                  "menu1"
#define PALETTE_MENU_2                  "menu2"
#define PALETTE_MENU_3                  "menu3"
#define PALETTE_MENU_4                  "menu4"
#define PALETTE_SKY                     "sky"
#define PALETTE_STATIC                  "static"
#define PALETTE_TRADEMARK               "trademark"
#define PALETTE_UNITS                   "units"


extern SDL_Window*      sdl_window;
extern SDL_Renderer*    sdl_renderer;
extern bool             running;
extern int              mouse_x;
extern int              mouse_y;

#endif // ABYSS_GLOBALS_H
