#ifndef GAME_STATE_H
#define GAME_STATE_H

#include "GameConstants.h"
#include "GameObjects.h"
#include <string>
#include <vector>

// Variáveis globais de estado do jogo
extern GameState gameState;
extern std::string currentPlayerName;
extern std::vector<FallingObject> objects;
extern std::vector<Building> cityscape;
extern Basket basket;
extern int score;
extern int misses;
extern int windowWidth, windowHeight;
extern bool key_a_pressed, key_d_pressed;

// Telas e Menus
extern std::vector<Button> pauseButtons;
extern Button homeButton;
extern float mouseGameX, mouseGameY;

// Dificuldade Dinâmica
extern float currentMinObjectSpeed;
extern float currentMaxObjectSpeedOffset;
extern int scoreForNextDifficultyIncrease;
extern int currentDifficultyLevel;
extern int spawnDelay;
extern int spawnTimer;
extern float difficultyMultiplier;

// Animação de Game Over
extern std::vector<TrashParticle> trashRain;
extern float gameOverAnimationTimer;
extern int displayedScore, finalScoreHolder;
extern float gameOverBasketAngle;
extern float gameOverBasketY;

// Animação da Tela Inicial
extern std::vector<TrashParticle> homeScreenDebris;
extern float homeScreenAnimationTimer;

// Ranking
extern std::vector<PlayerScore> ranking;

// Funções principais
void updateWindowTitle();
void triggerGameOver();
void resetGame();
void goToHomeScreen();
void initPauseMenu();
void initHomeScreen();

// Funções de dificuldade
void updateDifficulty();
int getDifficultyLevel();
float getDifficultyMultiplier();

#endif // GAME_STATE_H
