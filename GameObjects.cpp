#include "GameObjects.h"
#include "TextRenderer.h"
#include <GL/glut.h>
#include <cmath>
#include <cstdlib>
#include <ctime>

// Implementação do Button
Button::Button(float _x, float _y, float _w, float _h, const std::string &_text)
    : x(_x), y(_y), width(_w), height(_h), text(_text), isHovered(false) {}

bool Button::isInside(float px, float py) const {
  return px >= x - width / 2 && px <= x + width / 2 && py >= y - height / 2 &&
         py <= y + height / 2;
}

void Button::draw() const {
  // Muda a cor se o mouse estiver por cima (efeito hover)
  if (isHovered) {
    glColor4f(0.8f, 0.8f, 1.0f, 0.9f);
  } else {
    glColor4f(0.2f, 0.2f, 0.5f, 0.85f);
  }

  // Desenha o corpo do botão
  glBegin(GL_QUADS);
  glVertex2f(x - width / 2, y - height / 2);
  glVertex2f(x + width / 2, y - height / 2);
  glVertex2f(x + width / 2, y + height / 2);
  glVertex2f(x - width / 2, y + height / 2);
  glEnd();

  // Desenha a borda do botão
  glColor4f(1.0f, 1.0f, 1.0f, 0.9f);
  glLineWidth(2.0f);
  glBegin(GL_LINE_LOOP);
  glVertex2f(x - width / 2, y - height / 2);
  glVertex2f(x + width / 2, y - height / 2);
  glVertex2f(x + width / 2, y + height / 2);
  glVertex2f(x - width / 2, y + height / 2);
  glEnd();

  // Desenha o texto centralizado no botão
  glColor3f(1.0f, 1.0f, 1.0f);
  renderStrokeText(x, y - 0.015f, 0.0003f, 2.0f, text.c_str(), ALIGN_CENTER);
}

// Implementação do FallingObject
FallingObject::FallingObject() {
  size = 0.12f;
  respawn();
}

void FallingObject::respawn() {
  extern float currentMinObjectSpeed;
  extern float currentMaxObjectSpeedOffset;

  x = (static_cast<float>(rand()) / RAND_MAX) * 2.0f - 1.0f;
  y = 1.0f + (static_cast<float>(rand()) / RAND_MAX) * 0.5f;
  speed = currentMinObjectSpeed +
          (static_cast<float>(rand()) / RAND_MAX) * currentMaxObjectSpeedOffset;
  rotation = static_cast<float>(rand() % 360);
  rotationSpeed =
      ((static_cast<float>(rand()) / RAND_MAX) - 0.5f) * 2.0f * speed * 100.0f;
  wasteType = static_cast<WASTE_TYPE>(rand() % WASTE_TYPE_COUNT);
}

void FallingObject::update() {
  y -= speed;
  rotation += rotationSpeed;
}

void FallingObject::draw() {
  glPushMatrix();
  glTranslatef(x, y, 0.0f);
  glRotatef(rotation, 0.0f, 0.0f, 1.0f);
  glScalef(size, size, 1.0f);

  switch (wasteType) {
  case PAPER:
    glColor3f(0.9f, 0.9f, 0.85f);
    glBegin(GL_QUADS);
    glVertex2f(-0.5f, -0.2f);
    glVertex2f(0.5f, -0.2f);
    glVertex2f(0.5f, 0.2f);
    glVertex2f(-0.5f, 0.2f);
    glEnd();
    glColor3f(0.4f, 0.4f, 0.4f);
    glBegin(GL_LINES);
    for (int i = 0; i < 4; ++i) {
      float lineY = -0.15f + i * 0.1f;
      glVertex2f(-0.4f, lineY);
      glVertex2f(0.4f, lineY);
    }
    glEnd();
    break;
  case PLASTIC:
    glBegin(GL_QUADS);
    glColor3f(0.8f, 0.2f, 0.2f);
    glVertex2f(-0.25f, -0.5f);
    glVertex2f(0.25f, -0.5f);
    glColor3f(1.0f, 0.5f, 0.5f);
    glVertex2f(0.25f, 0.2f);
    glVertex2f(-0.25f, 0.2f);
    glEnd();
    glBegin(GL_QUADS);
    glVertex2f(-0.15f, 0.2f);
    glVertex2f(0.15f, 0.2f);
    glVertex2f(0.15f, 0.4f);
    glVertex2f(-0.15f, 0.4f);
    glEnd();
    glColor3f(0.6f, 0.1f, 0.1f);
    glBegin(GL_QUADS);
    glVertex2f(-0.17f, 0.4f);
    glVertex2f(0.17f, 0.4f);
    glVertex2f(0.17f, 0.5f);
    glVertex2f(-0.17f, 0.5f);
    glEnd();
    glColor4f(1.0f, 1.0f, 1.0f, 0.5f);
    glBegin(GL_QUADS);
    glVertex2f(-0.2f, -0.3f);
    glVertex2f(-0.1f, -0.3f);
    glVertex2f(-0.1f, 0.1f);
    glVertex2f(-0.2f, 0.1f);
    glEnd();
    break;
  case METAL:
    // --- INÍCIO DA ALTERAÇÃO ---
    // Lata de metal com faixa amarela para diferenciação
    glBegin(GL_QUAD_STRIP);
    glColor3f(0.6f, 0.6f, 0.65f);
    glVertex2f(-0.3f, -0.5f);
    glVertex2f(-0.3f, 0.5f);
    glColor3f(0.9f, 0.9f, 0.95f);
    glVertex2f(-0.1f, -0.5f);
    glVertex2f(-0.1f, 0.5f);
    glColor3f(0.9f, 0.9f, 0.95f);
    glVertex2f(0.1f, -0.5f);
    glVertex2f(0.1f, 0.5f);
    glColor3f(0.6f, 0.6f, 0.65f);
    glVertex2f(0.3f, -0.5f);
    glVertex2f(0.3f, 0.5f);
    glEnd();
    // Bordas superior e inferior
    glColor3f(0.5f, 0.5f, 0.55f);
    glBegin(GL_QUADS);
    glVertex2f(-0.3f, 0.5f);
    glVertex2f(0.3f, 0.5f);
    glVertex2f(0.3f, 0.4f);
    glVertex2f(-0.3f, 0.4f);
    glVertex2f(-0.3f, -0.5f);
    glVertex2f(0.3f, -0.5f);
    glVertex2f(0.3f, -0.4f);
    glVertex2f(-0.3f, -0.4f);
    glEnd();
    // Faixa amarela
    glColor3f(0.9f, 0.8f, 0.1f);
    glBegin(GL_QUADS);
    glVertex2f(-0.32f, 0.25f);
    glVertex2f(0.32f, 0.25f);
    glVertex2f(0.32f, -0.05f);
    glVertex2f(-0.32f, -0.05f);
    glEnd();
    // --- FIM DA ALTERAÇÃO ---
    break;
  case GLASS:
    glColor4f(0.2f, 0.7f, 0.2f, 0.7f);
    glBegin(GL_QUADS);
    glVertex2f(-0.25f, -0.5f);
    glVertex2f(0.25f, -0.5f);
    glVertex2f(0.25f, 0.1f);
    glVertex2f(-0.25f, 0.1f);
    glEnd();
    glBegin(GL_TRIANGLES);
    glVertex2f(-0.25f, 0.1f);
    glVertex2f(0.25f, 0.1f);
    glVertex2f(0.15f, 0.3f);
    glVertex2f(-0.25f, 0.1f);
    glVertex2f(-0.15f, 0.3f);
    glVertex2f(0.15f, 0.3f);
    glEnd();
    glBegin(GL_QUADS);
    glVertex2f(-0.1f, 0.3f);
    glVertex2f(0.1f, 0.3f);
    glVertex2f(0.1f, 0.5f);
    glVertex2f(-0.1f, 0.5f);
    glEnd();
    glColor4f(1.0f, 1.0f, 1.0f, 0.6f);
    glBegin(GL_QUADS);
    glVertex2f(0.1f, -0.4f);
    glVertex2f(0.18f, -0.4f);
    glVertex2f(0.18f, 0.2f);
    glVertex2f(0.1f, 0.2f);
    glEnd();
    break;
  case ORGANIC:
    glBegin(GL_POLYGON);
    glColor3f(0.9f, 0.1f, 0.1f);
    for (int i = 0; i < 20; i++) {
      float ang = 2.0f * M_PI * i / 20.0f;
      glVertex2f(cos(ang) * 0.4f, sin(ang) * 0.5f);
    }
    glEnd();
    glColor3f(0.4f, 0.2f, 0.0f);
    glBegin(GL_QUADS);
    glVertex2f(-0.05f, 0.4f);
    glVertex2f(0.05f, 0.4f);
    glVertex2f(0.05f, 0.6f);
    glVertex2f(-0.05f, 0.6f);
    glEnd();
    glColor3f(0.1f, 0.8f, 0.1f);
    glBegin(GL_TRIANGLES);
    glVertex2f(0.05f, 0.5f);
    glVertex2f(0.3f, 0.7f);
    glVertex2f(0.1f, 0.4f);
    glEnd();
    break;
  default:
    break;
  }
  glPopMatrix();
}

// Implementação do Basket
Basket::Basket() {
  width = 0.3f;
  height = 0.2f;
  x = 0.0f;
  y = -0.8f + height / 2.0f;
  wasteType = PAPER;
  speed = 0.05f;
}

void Basket::draw() {
  // A definição de COLOR_TABLE não está neste arquivo.
  // Supondo que ela seja definida em outro lugar e acessível.
  extern const GLfloat COLOR_TABLE[5][3];

  const float *color = COLOR_TABLE[wasteType];

  // Corpo da cesta com gradiente
  glBegin(GL_QUADS);
  glColor3f(color[0] * 0.7f, color[1] * 0.7f, color[2] * 0.7f);
  glVertex2f(-width / 2, -height / 2);
  glVertex2f(width / 2, -height / 2);
  glColor3f(color[0], color[1], color[2]);
  glVertex2f(width / 2, height / 2);
  glVertex2f(-width / 2, height / 2);
  glEnd();

  // Borda superior da cesta
  glColor3f(color[0] * 0.5f, color[1] * 0.5f, color[2] * 0.5f);
  glBegin(GL_QUADS);
  glVertex2f(-width / 2 - 0.02f, height / 2);
  glVertex2f(width / 2 + 0.02f, height / 2);
  glVertex2f(width / 2 + 0.02f, height / 2 + 0.03f);
  glVertex2f(-width / 2 - 0.02f, height / 2 + 0.03f);
  glEnd();

// Símbolo de reciclagem
    glPushMatrix();
  glColor3f(1.0f, 1.0f, 1.0f);
  
  // Reduz a escala do símbolo para 80% para que ele caiba na cesta
  glScalef(0.8f, 0.8f, 1.0f); 

  float s = 0.05f;
  for (int i = 0; i < 3; ++i) {
    glRotatef(120.0, 0, 0, 1);
    glBegin(GL_QUADS);
    glVertex2f(-s, s);
    glVertex2f(s, s);
    glVertex2f(s * 1.5, s * 1.8);
    glVertex2f(-s * 0.5, s * 1.8);
    glEnd();
  }
  glPopMatrix();
}

void Basket::move(float direction) {
  x += direction * speed;
  if (x - width / 2 < -1.0f) {
    x = -1.0f + width / 2;
  }
  if (x + width / 2 > 1.0f) {
    x = 1.0f - width / 2;
  }
}

// Implementação da função drawRainObject
void drawRainObject(const TrashParticle &p) {
  glPushMatrix();
  glTranslatef(p.x, p.y, 0.0f);
  glRotatef(p.rotation, 0.0f, 0.0f, 1.0f);
  glScalef(p.size, p.size, 1.0f);

  switch (p.type) {
  case PAPER:
    glColor3f(0.9f, 0.9f, 0.85f);
    glBegin(GL_QUADS);
    glVertex2f(-0.5f, -0.2f);
    glVertex2f(0.5f, -0.2f);
    glVertex2f(0.5f, 0.2f);
    glVertex2f(-0.5f, 0.2f);
    glEnd();
    glColor3f(0.4f, 0.4f, 0.4f);
    glBegin(GL_LINES);
    for (int i = 0; i < 4; ++i) {
      float lineY = -0.15f + i * 0.1f;
      glVertex2f(-0.4f, lineY);
      glVertex2f(0.4f, lineY);
    }
    glEnd();
    break;
  case PLASTIC:
    glBegin(GL_QUADS);
    glColor3f(0.8f, 0.2f, 0.2f);
    glVertex2f(-0.25f, -0.5f);
    glVertex2f(0.25f, -0.5f);
    glColor3f(1.0f, 0.5f, 0.5f);
    glVertex2f(0.25f, 0.2f);
    glVertex2f(-0.25f, 0.2f);
    glEnd();
    glBegin(GL_QUADS);
    glVertex2f(-0.15f, 0.2f);
    glVertex2f(0.15f, 0.2f);
    glVertex2f(0.15f, 0.4f);
    glVertex2f(-0.15f, 0.4f);
    glEnd();
    glColor3f(0.6f, 0.1f, 0.1f);
    glBegin(GL_QUADS);
    glVertex2f(-0.17f, 0.4f);
    glVertex2f(0.17f, 0.4f);
    glVertex2f(0.17f, 0.5f);
    glVertex2f(-0.17f, 0.5f);
    glEnd();
    glColor4f(1.0f, 1.0f, 1.0f, 0.5f);
    glBegin(GL_QUADS);
    glVertex2f(-0.2f, -0.3f);
    glVertex2f(-0.1f, -0.3f);
    glVertex2f(-0.1f, 0.1f);
    glVertex2f(-0.2f, 0.1f);
    glEnd();
    break;
  case METAL:
    // --- INÍCIO DA ALTERAÇÃO ---
    // Lata de metal com faixa amarela para diferenciação
    glBegin(GL_QUAD_STRIP);
    glColor3f(0.6f, 0.6f, 0.65f);
    glVertex2f(-0.3f, -0.5f);
    glVertex2f(-0.3f, 0.5f);
    glColor3f(0.9f, 0.9f, 0.95f);
    glVertex2f(-0.1f, -0.5f);
    glVertex2f(-0.1f, 0.5f);
    glColor3f(0.9f, 0.9f, 0.95f);
    glVertex2f(0.1f, -0.5f);
    glVertex2f(0.1f, 0.5f);
    glColor3f(0.6f, 0.6f, 0.65f);
    glVertex2f(0.3f, -0.5f);
    glVertex2f(0.3f, 0.5f);
    glEnd();
    // Bordas superior e inferior
    glColor3f(0.5f, 0.5f, 0.55f);
    glBegin(GL_QUADS);
    glVertex2f(-0.3f, 0.5f);
    glVertex2f(0.3f, 0.5f);
    glVertex2f(0.3f, 0.4f);
    glVertex2f(-0.3f, 0.4f);
    glVertex2f(-0.3f, -0.5f);
    glVertex2f(0.3f, -0.5f);
    glVertex2f(0.3f, -0.4f);
    glVertex2f(-0.3f, -0.4f);
    glEnd();
    // Faixa amarela
    glColor3f(0.9f, 0.8f, 0.1f);
    glBegin(GL_QUADS);
    glVertex2f(-0.32f, 0.25f);
    glVertex2f(0.32f, 0.25f);
    glVertex2f(0.32f, -0.05f);
    glVertex2f(-0.32f, -0.05f);
    glEnd();
    // --- FIM DA ALTERAÇÃO ---
    break;
  case GLASS:
    glColor4f(0.2f, 0.7f, 0.2f, 0.7f);
    glBegin(GL_QUADS);
    glVertex2f(-0.25f, -0.5f);
    glVertex2f(0.25f, -0.5f);
    glVertex2f(0.25f, 0.1f);
    glVertex2f(-0.25f, 0.1f);
    glEnd();
    glBegin(GL_TRIANGLES);
    glVertex2f(-0.25f, 0.1f);
    glVertex2f(0.25f, 0.1f);
    glVertex2f(0.15f, 0.3f);
    glVertex2f(-0.25f, 0.1f);
    glVertex2f(-0.15f, 0.3f);
    glVertex2f(0.15f, 0.3f);
    glEnd();
    glBegin(GL_QUADS);
    glVertex2f(-0.1f, 0.3f);
    glVertex2f(0.1f, 0.3f);
    glVertex2f(0.1f, 0.5f);
    glVertex2f(-0.1f, 0.5f);
    glEnd();
    glColor4f(1.0f, 1.0f, 1.0f, 0.6f);
    glBegin(GL_QUADS);
    glVertex2f(0.1f, -0.4f);
    glVertex2f(0.18f, -0.4f);
    glVertex2f(0.18f, 0.2f);
    glVertex2f(0.1f, 0.2f);
    glEnd();
    break;
  case ORGANIC:
    glBegin(GL_POLYGON);
    glColor3f(0.9f, 0.1f, 0.1f);
    for (int i = 0; i < 20; i++) {
      float ang = 2.0f * M_PI * i / 20.0f;
      glVertex2f(cos(ang) * 0.4f, sin(ang) * 0.5f);
    }
    glEnd();
    glColor3f(0.4f, 0.2f, 0.0f);
    glBegin(GL_QUADS);
    glVertex2f(-0.05f, 0.4f);
    glVertex2f(0.05f, 0.4f);
    glVertex2f(0.05f, 0.6f);
    glVertex2f(-0.05f, 0.6f);
    glEnd();
    glColor3f(0.1f, 0.8f, 0.1f);
    glBegin(GL_TRIANGLES);
    glVertex2f(0.05f, 0.5f);
    glVertex2f(0.3f, 0.7f);
    glVertex2f(0.1f, 0.4f);
    glEnd();
    break;
  default:
    break;
  }
  glPopMatrix();
}