#include "GameLoop.h"
#include "AudioManager.h"
#include "GameState.h"
#include "Scenery.h"
#include "Screens.h"
#include <GL/glut.h>
#include <algorithm>
#include <cmath>
#include <cstdlib>

void display() {
  glClear(GL_COLOR_BUFFER_BIT);
  drawUrbanScenery(cityscape, windowWidth, windowHeight);

  switch (gameState) {
  case STATE_HOME:
    drawHomeScreen(ranking, currentPlayerName, homeButton, homeScreenDebris,
                   homeScreenAnimationTimer);
    break;
  case STATE_PLAYING:
    drawGamePlayScreen(objects, basket, score, misses);
    break;
  case STATE_PAUSED:
    drawGamePlayScreen(objects, basket, score, misses);
    drawPauseMenu(pauseButtons);
    break;
  case STATE_GAMEOVER:
    drawGameOverScreen(trashRain, basket, gameOverBasketY, gameOverBasketAngle,
                       gameOverAnimationTimer, displayedScore, ranking,
                       currentPlayerName, finalScoreHolder);
    break;
  }
  glutSwapBuffers();
}

void update(int value) {
  (void)value; 
  float dt = 16.0f / 1000.0f;

  if (gameState == STATE_HOME) {
    // Animação da tela inicial
    if (homeScreenAnimationTimer < HOMESCREEN_ANIMATION_DURATION) {
      homeScreenAnimationTimer += dt;
    }

    // Move as partículas de fundo
    for (auto &p : homeScreenDebris) {
      p.x += p.vx * dt;
      p.y += p.vy * dt;
      p.rotation += p.rotationSpeed * dt;

      if (p.x > 2.2f)
        p.x = -2.2f;
      if (p.x < -2.2f)
        p.x = 2.2f;
      if (p.y > 1.2f)
        p.y = -1.2f;
      if (p.y < -1.2f)
        p.y = 1.2f;
    }
  } else if (gameState == STATE_PLAYING) {
    // Movimento da cesta
    if (key_a_pressed && !key_d_pressed) {
      basket.move(-1.0f);
    } else if (key_d_pressed && !key_a_pressed) {
      basket.move(1.0f);
    }

    // Sistema de spawn controlado por timer
    spawnTimer++;
    if (spawnTimer >= spawnDelay &&
        objects.size() < static_cast<size_t>(MAX_NUM_OBJECTS)) {
      objects.push_back(FallingObject());
      spawnTimer = 0;
    }

    // Atualiza objetos
    for (size_t i = 0; i < objects.size(); ++i) {
      objects[i].update();

      // Colisão e verificações
      float obj_right = objects[i].x + objects[i].size / 2;
      float obj_left = objects[i].x - objects[i].size / 2;
      float obj_bottom = objects[i].y - objects[i].size * 0.5f;
      float basket_right = basket.x + basket.width / 2;
      float basket_left = basket.x - basket.width / 2;
      float basket_top = basket.y + basket.height / 2;

      // Verifica se caiu no chão
      if (obj_bottom < -0.8f) {
        if (++misses >= MAX_MISSES) {
          triggerGameOver();
        }
        objects[i].respawn();
      }
      // Verifica colisão com a cesta
      else if (obj_right > basket_left && obj_left < basket_right &&
               obj_bottom <= basket_top && objects[i].y >= basket.y) {
        if (objects[i].wasteType == basket.wasteType) {
          score++;
          // Som de coleta correta
          if (audioManager.isEnabled()) {
            audioManager.playSound("collect_correct");
          }
        } else {
          // Som de coleta incorreta
          if (audioManager.isEnabled()) {
            audioManager.playSound("collect_wrong");
          }

          if (++misses >= MAX_MISSES) {
            triggerGameOver();
          }
        }
        objects[i].respawn();
        updateWindowTitle();

        // Atualiza sistema de dificuldade
        updateDifficulty();
      }
    }
  } else if (gameState == STATE_GAMEOVER) {
    // Animação de game over
    if (gameOverAnimationTimer < GAMEOVER_ANIMATION_DURATION) {
      gameOverAnimationTimer += dt;
    }

    // Animação de contagem da pontuação
    if (displayedScore < finalScoreHolder) {
      int increment = std::max(1, (finalScoreHolder - displayedScore) / 15);
      displayedScore = std::min(finalScoreHolder, displayedScore + increment);
    }

    // Animação da cesta tombando
    if (gameOverBasketAngle > -90.0f) {
      gameOverBasketAngle -= 2.0f;
      if (gameOverBasketAngle < -90.0f) {
        gameOverBasketAngle = -90.0f;
      }
    }
    float finalY = -0.8f + basket.width / 2.0f;
    gameOverBasketY =
        basket.y + (finalY - basket.y) * fabs(gameOverBasketAngle / -90.0f);

    // Gera partículas de chuva de lixo
    if (glutGet(GLUT_ELAPSED_TIME) % 2 == 0 && trashRain.size() < 1200) {
      TrashParticle p;
      p.x = (static_cast<float>(rand()) / RAND_MAX) * 4.0f - 2.0f;
      p.y = 1.2f + (static_cast<float>(rand()) / RAND_MAX) * 0.5f;
      p.vx = ((static_cast<float>(rand()) / RAND_MAX) - 0.5f) * 0.5f;
      p.vy = 0.0f;
      p.size = 0.06f + (static_cast<float>(rand()) / RAND_MAX) * 0.05f;
      p.largura = p.size;
      p.altura = p.size;
      p.rotation = static_cast<float>(rand() % 360);
      p.rotationSpeed = ((static_cast<float>(rand()) / RAND_MAX) - 0.5f) * 4.0f;
      p.type = static_cast<WASTE_TYPE>(rand() % WASTE_TYPE_COUNT);
      p.estatico = false;
      trashRain.push_back(p);
    }

    // Simulação física das partículas
    for (size_t i = 0; i < trashRain.size(); ++i) {
      TrashParticle &p = trashRain[i];
      if (p.estatico) {
        continue;
      }

      p.vy -= 0.0008f;
      p.x += p.vx * dt;
      p.y += p.vy;
      p.rotation += p.rotationSpeed;

      bool emRepouso = false;
      if (p.y - p.altura / 2.0f <= -0.8f) {
        p.y = -0.8f + p.altura / 2.0f;
        emRepouso = true;
      }

      // Colisão entre partículas
      for (size_t j = 0; j < trashRain.size(); ++j) {
        if (i == j)
          continue;

        const TrashParticle &other = trashRain[j];
        float dist_x = p.x - other.x, dist_y = p.y - other.y;
        float dist_total = sqrt(dist_x * dist_x + dist_y * dist_y);
        float raios_soma = (p.largura + other.largura) / 2.0f;

        if (dist_total < raios_soma) {
          float overlap = raios_soma - dist_total;
          p.x += (overlap * (p.x - other.x)) / dist_total;
          p.y += (overlap * (p.y - other.y)) / dist_total;
          p.vx += (dist_x > 0 ? 1 : -1) * 0.05f * dt;
          if (p.y > other.y) {
            emRepouso = true;
          }
        }
      }

      if (emRepouso) {
        p.vx *= 0.85f;
        p.vy = 0;
        if (fabs(p.vx) < 0.001f) {
          p.estatico = true;
          p.vx = 0;
          p.rotationSpeed = 0;
        }
      }
    }
  }

  glutPostRedisplay();
  glutTimerFunc(16, update, 0);
}

void reshape(int w, int h) {
  if (h == 0) {
    h = 1;
  }
  windowWidth = w;
  windowHeight = h;
  glViewport(0, 0, w, h);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();

  float aspect = (float)w / (float)h;
  if (w <= h) {
    gluOrtho2D(-1.0f, 1.0f, -1.0f / aspect, 1.0f / aspect);
  } else {
    gluOrtho2D(-1.0f * aspect, 1.0f * aspect, -1.0f, 1.0f);
  }

  basket.move(0);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
}
