#include "TextRenderer.h"
#include <GL/glut.h>

void renderBitmapText(float x, float y, void *font, const char *string) {
  glRasterPos2f(x, y);
  while (*string) {
    glutBitmapCharacter(font, *string++);
  }
}

float getStrokeTextWidth(const char *string) {
  float width = 0;
  while (*string) {
    width += glutStrokeWidth(GLUT_STROKE_ROMAN, *string++);
  }
  return width;
}

void renderStrokeText(float x, float y, float scale, float lineWidth,
                      const char *string, Alignment align) {
  glPushMatrix();
  float textWidth = 0;

  if (align == ALIGN_CENTER) {
    textWidth = getStrokeTextWidth(string);
  }

  glTranslatef(x - (textWidth / 2.0f * scale), y, 0.0f);
  glScalef(scale, scale, 1.0f);
  glLineWidth(lineWidth);

  while (*string) {
    glutStrokeCharacter(GLUT_STROKE_ROMAN, *string++);
  }
  glPopMatrix();
}
