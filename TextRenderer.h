#ifndef TEXT_RENDERER_H
#define TEXT_RENDERER_H

#include "GameConstants.h"

// Funções de renderização de texto
void renderBitmapText(float x, float y, void *font, const char *string);
void renderStrokeText(float x, float y, float scale, float lineWidth,
                      const char *string, Alignment align);
float getStrokeTextWidth(const char *string);

#endif // TEXT_RENDERER_H
