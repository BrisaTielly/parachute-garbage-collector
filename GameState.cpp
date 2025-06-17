#include "GameState.h"
#include "AudioManager.h"
#include "RankingSystem.h"
#include <GL/glut.h>
#include <cstdio>
#include <cstdlib>

// Definição das variáveis globais
GameState gameState = STATE_HOME;
std::string currentPlayerName = "Jogador";
std::vector<FallingObject> objects;
std::vector<Building> cityscape;
Basket basket;
int score = 0;
int misses = 0;
int windowWidth = 600, windowHeight = 800;
bool key_a_pressed = false, key_d_pressed = false;

std::vector<Button> pauseButtons;
Button homeButton(0.0f, -0.1f, 0.8f, 0.15f, "Iniciar");
float mouseGameX, mouseGameY;

float currentMinObjectSpeed = INITIAL_MIN_OBJECT_SPEED;
float currentMaxObjectSpeedOffset = INITIAL_MAX_OBJECT_SPEED_OFFSET;
int scoreForNextDifficultyIncrease = 5;

std::vector<TrashParticle> trashRain;
float gameOverAnimationTimer = 0.0f;
int displayedScore = 0, finalScoreHolder = 0;
float gameOverBasketAngle = 0.0f;
float gameOverBasketY = 0.0f;

std::vector<TrashParticle> homeScreenDebris;
float homeScreenAnimationTimer = 0.0f;

std::vector<PlayerScore> ranking;

void updateWindowTitle() {
  char title[100];
  if (gameState == STATE_PLAYING) {
    sprintf(title, "Coleta Seletiva - Pontos: %d", score);
  } else {
    sprintf(title, "Coleta Seletiva");
  }
  glutSetWindowTitle(title);
}

void triggerGameOver() {
  gameState = STATE_GAMEOVER;
  finalScoreHolder = score;
  gameOverBasketY = basket.y;
  saveRanking(currentPlayerName, score, ranking);
  loadRanking(ranking);

  // Trocar música para game over (tocar apenas uma vez com volume alto)
  if (audioManager.isEnabled()) {
    audioManager.playMusicWithVolume(
        "gameover", 64, 0); // Volume 64 (4x maior), tocar apenas uma vez
  }
}

void initHomeScreen() {
  homeScreenDebris.clear();
  for (int i = 0; i < 50; ++i) {
    TrashParticle p;
    p.x = (static_cast<float>(rand()) / RAND_MAX) * 4.0f - 2.0f;
    p.y = (static_cast<float>(rand()) / RAND_MAX) * 2.0f - 1.0f;
    p.vx = ((static_cast<float>(rand()) / RAND_MAX) - 0.5f) * 0.01f;
    p.vy = ((static_cast<float>(rand()) / RAND_MAX) - 0.5f) * 0.01f;
    p.size = 0.04f + (static_cast<float>(rand()) / RAND_MAX) * 0.04f;
    p.rotation = static_cast<float>(rand() % 360);
    p.rotationSpeed = ((static_cast<float>(rand()) / RAND_MAX) - 0.5f) * 0.5f;
    p.type = static_cast<WASTE_TYPE>(rand() % WASTE_TYPE_COUNT);
    p.estatico = false;
    homeScreenDebris.push_back(p);
  }
}

void goToHomeScreen() {
  gameState = STATE_HOME;
  updateWindowTitle();
  loadRanking(ranking);
  homeScreenAnimationTimer = 0.0f;
  initHomeScreen();

  // Trocar música para menu
  if (audioManager.isEnabled()) {
    audioManager.setMusicVolume(16); // Restaurar volume normal
    audioManager.playMusic("menu");
  }
}

void resetGame() {
  if (currentPlayerName.empty()) {
    currentPlayerName = "Jogador";
  }

  score = 0;
  misses = 0;
  gameOverAnimationTimer = 0.0f;
  displayedScore = 0;
  finalScoreHolder = 0;
  gameOverBasketAngle = 0.0f;
  trashRain.clear();

  currentMinObjectSpeed = INITIAL_MIN_OBJECT_SPEED;
  currentMaxObjectSpeedOffset = INITIAL_MAX_OBJECT_SPEED_OFFSET;
  scoreForNextDifficultyIncrease = 5;

  objects.clear();
  objects.push_back(FallingObject());
  basket.x = 0.0f;
  basket.wasteType = PAPER;

  gameState = STATE_PLAYING;
  updateWindowTitle();

  // Trocar música para gameplay
  if (audioManager.isEnabled()) {
    audioManager.setMusicVolume(16); // Restaurar volume normal
    audioManager.playMusic("gameplay");
  }
}

void initPauseMenu() {
  pauseButtons.clear();
  pauseButtons.push_back(Button(0.0f, 0.3f, 0.8f, 0.15f, "Continuar"));
  pauseButtons.push_back(Button(0.0f, 0.1f, 0.8f, 0.15f, "Reiniciar"));
  pauseButtons.push_back(Button(0.0f, -0.1f, 0.8f, 0.15f, "Tela Inicial"));
  pauseButtons.push_back(Button(0.0f, -0.3f, 0.8f, 0.15f, "Sair do Jogo"));
}
