#ifndef ABYSS_SPRITE_H
#define ABYSS_SPRITE_H

#include <SDL2/SDL.h>
#include <stdbool.h>
#include <stdint.h>

typedef struct Sprite Sprite;

Sprite *Sprite_Create(const char *path, const char *palette_name);
void    Sprite_Destroy(Sprite **sprite);
void    Sprite_SetBlendMode(const Sprite *sprite, SDL_BlendMode blend_mode);
void    Sprite_SetPlayLength(Sprite *sprite, double play_length);
void    Sprite_Draw(const Sprite *sprite, uint8_t frame_index, int x, int y);
void    Sprite_DrawMulti(const Sprite *sprite, uint8_t frame_index, int x, int y, int frames_x, int frames_y);
void    Sprite_DrawAnimated(Sprite *sprite, int x, int y);

#endif // ABYSS_SPRITE_H
