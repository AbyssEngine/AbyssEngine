#ifndef ABYSS_LABEL_H
#define ABYSS_LABEL_H

#include "../types/DC6.h"
#include "../types/Font.h"
#include "../types/Palette.h"
#include <SDL2/SDL.h>

typedef uint8_t label_align_t;
#define LABEL_ALIGN_BEGIN  (label_align_t)0
#define LABEL_ALIGN_CENTER (label_align_t)1
#define LABEL_ALIGN_END    (label_align_t)2

typedef struct Label Label;

void Label_InitializeCaches(void);
void Label_FinalizeCaches(void);

Label *Label_Create(const char *font_path, const char *palette_name);
void   Label_Destroy(Label **Label);
void   Label_SetText(Label *Label, const char *text);
void   Label_Draw(const Label *Label, int x, int y);
void   Label_SetColor(Label *Label, uint8_t r, uint8_t g, uint8_t b, uint8_t a);
void   Label_SetAlignment(Label *Label, label_align_t horizontal, label_align_t vertical);

#endif // ABYSS_LABEL_H
