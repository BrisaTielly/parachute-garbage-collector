#ifndef GAME_OBJECTS_H
#define GAME_OBJECTS_H

#include "GameConstants.h"
#include <string>
#include <vector>

// Estrutura para os botões da interface (UI)
struct Button {
  float x, y, width, height; // Posição e dimensões
  std::string text;          // Texto exibido no botão
  bool isHovered;            // Flag para indicar se o mouse está sobre o botão

  Button(float _x, float _y, float _w, float _h, const std::string &_text);
  bool isInside(float px, float py) const;
  void draw() const;
};

// Estrutura para as partículas de lixo das animações
struct TrashParticle {
  float x, y, vx, vy, rotation, rotationSpeed, size, largura, altura;
  bool estatico;   // A partícula para de se mover?
  WASTE_TYPE type; // Tipo de lixo para definir a cor/forma
};

// Estrutura para os prédios do cenário
struct Building {
  float x_pos, width, height, r, g, b;
  int layer; // Camada para efeito de paralaxe
};

// Estrutura para representar um objeto caindo
struct FallingObject {
  float x, y, size, speed, rotation, rotationSpeed;
  WASTE_TYPE wasteType; // Tipo de lixo

  FallingObject();
  void respawn();
  void update();
  void draw();
};

// Estrutura para representar a cesta coletora
struct Basket {
  float x, y, width, height, speed; // Posição, dimensões e velocidade
  WASTE_TYPE wasteType;             // Tipo de lixo que a cesta aceita

  Basket();
  void draw();
  void move(float direction);
};

// Estrutura para armazenar o score de um jogador no ranking
struct PlayerScore {
  std::string name;
  int score;
};

// Funções para desenhar objetos
void drawRainObject(const TrashParticle &p);

#endif // GAME_OBJECTS_H
