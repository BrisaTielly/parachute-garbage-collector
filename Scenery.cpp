#include "Scenery.h"
#include <GL/glut.h>
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <ctime>

void initUrbanScenery(std::vector<Building> &cityscape) {
  cityscape.clear();
  srand(1337); // Usa uma seed fixa para que o cenário seja sempre o mesmo
  float worldWidth = 4.0f;

  // Gera a primeira camada de prédios (mais escuros, ao fundo)
  float current_x = -worldWidth;
  while (current_x < worldWidth) {
    Building b;
    b.x_pos = current_x;
    b.width = 0.2f + (static_cast<float>(rand()) / RAND_MAX) * 0.3f;
    b.height = 0.2f + (static_cast<float>(rand()) / RAND_MAX) * 0.6f;
    float gray = 0.15f + (static_cast<float>(rand()) / RAND_MAX) * 0.1f;
    b.r = gray;
    b.g = gray;
    b.b = gray + 0.05f;
    b.layer = 0;
    cityscape.push_back(b);
    current_x += b.width + 0.05f;
  }

  // Gera a segunda camada de prédios (mais claros, à frente)
  current_x = -worldWidth;
  while (current_x < worldWidth) {
    Building b;
    b.x_pos = current_x;
    b.width = 0.3f + (static_cast<float>(rand()) / RAND_MAX) * 0.2f;
    b.height = -0.2f + (static_cast<float>(rand()) / RAND_MAX) * 0.3f;
    float gray = 0.25f + (static_cast<float>(rand()) / RAND_MAX) * 0.1f;
    b.r = gray;
    b.g = gray;
    b.b = gray;
    b.layer = 1;
    cityscape.push_back(b);
    current_x += b.width + 0.1f;
  }

  // Ordena os prédios por camada para desenhar corretamente o efeito de
  // profundidade
  std::sort(
      cityscape.begin(), cityscape.end(),
      [](const Building &a, const Building &b) { return a.layer < b.layer; });

  srand(static_cast<unsigned int>(
      time(0))); // Reseta a seed para o resto do jogo ser aleatório
}

void drawUrbanScenery(const std::vector<Building> &cityscape, int windowWidth,
                      int windowHeight) {
  float aspect = (float)windowWidth / (float)windowHeight;
  float worldLeft = -1.0, worldRight = 1.0, worldTop = 1.0, worldBottom = -1.0;
  (void)worldBottom; // Suprime warning - variável pode ser usada futuramente

  if (aspect > 1.0) {
    worldLeft = -aspect;
    worldRight = aspect;
  } else {
    worldTop = 1.0 / aspect;
    worldBottom = -1.0 / aspect;
  }

  // Desenha o céu com um gradiente do escuro (topo) para o claro (horizonte)
  glBegin(GL_QUADS);
  glColor3f(0.1f, 0.1f, 0.3f);
  glVertex2f(worldLeft, worldTop);
  glVertex2f(worldRight, worldTop);
  glColor3f(0.9f, 0.7f, 0.4f);
  glVertex2f(worldRight, -0.7f);
  glVertex2f(worldLeft, -0.7f);
  glEnd();

  int time_ms = glutGet(GLUT_ELAPSED_TIME);

  // Desenha cada prédio
  for (const auto &b : cityscape) {
    glColor3f(b.r, b.g, b.b);
    glBegin(GL_QUADS);
    glVertex2f(b.x_pos, -0.8f);
    glVertex2f(b.x_pos + b.width, -0.8f);
    glVertex2f(b.x_pos + b.width, b.height);
    glVertex2f(b.x_pos, b.height);
    glEnd();

    // Desenha as janelas
    float window_margin = 0.1f * b.width;
    float window_size = 0.08f * b.width;
    int num_floors = static_cast<int>((b.height + 0.8f) / 0.1f);
    int num_windows_per_floor =
        static_cast<int>((b.width - 2 * window_margin) / (window_size * 1.5f));

    for (int i = 0; i < num_floors; ++i) {
      for (int j = 0; j < num_windows_per_floor; ++j) {
        int window_seed = static_cast<int>(b.x_pos * 100) + i * 13 + j * 7;
        if (sin((time_ms / 1000.0f) * 0.2f + window_seed) > 0.8) {
          glColor3f(0.9f, 0.9f, 0.6f);
        } else {
          glColor3f(b.r * 0.5f, b.g * 0.5f, b.b * 0.5f);
        }
        float wx = b.x_pos + window_margin + j * (window_size * 1.5f);
        float wy = -0.75f + i * 0.1f;
        glBegin(GL_QUADS);
        glVertex2f(wx, wy);
        glVertex2f(wx + window_size, wy);
        glVertex2f(wx + window_size, wy + 0.05f);
        glVertex2f(wx, wy + 0.05f);
        glEnd();
      }
    }
  }

  // Desenha a rua/chão
  glColor3f(0.2f, 0.2f, 0.2f);
  glBegin(GL_QUADS);
  glVertex2f(worldLeft, -1.0f);
  glVertex2f(worldRight, -1.0f);
  glVertex2f(worldRight, -0.8f);
  glVertex2f(worldLeft, -0.8f);
  glEnd();
}
