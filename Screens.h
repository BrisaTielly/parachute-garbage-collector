#ifndef SCREENS_H
#define SCREENS_H

#include "GameObjects.h"
#include <vector>

// Funções para desenhar as diferentes telas
void drawHomeScreen(const std::vector<PlayerScore> &ranking,
                    const std::string &currentPlayerName,
                    const Button &homeButton,
                    const std::vector<TrashParticle> &homeScreenDebris,
                    float homeScreenAnimationTimer);

void drawPauseMenu(const std::vector<Button> &pauseButtons);

void drawGamePlayScreen(const std::vector<FallingObject> &objects,
                        const Basket &basket, int score, int misses);

void drawGameOverScreen(const std::vector<TrashParticle> &trashRain,
                        const Basket &basket, float gameOverBasketY,
                        float gameOverBasketAngle, float gameOverAnimationTimer,
                        int displayedScore,
                        const std::vector<PlayerScore> &ranking,
                        const std::string &currentPlayerName,
                        int finalScoreHolder);

#endif // SCREENS_H
