#include "Screens.h"
#include "GameConstants.h"
#include "TextRenderer.h"
#include <GL/glut.h>
#include <algorithm>
#include <cmath>
#include <cstdio>

void drawHomeScreen(const std::vector<PlayerScore> &ranking,
                    const std::string &currentPlayerName,
                    const Button &homeButton,
                    const std::vector<TrashParticle> &homeScreenDebris,
                    float homeScreenAnimationTimer) {
  // Desenha os detritos flutuantes no fundo
  for (const auto &p : homeScreenDebris) {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(1.0f, 1.0f, 1.0f, 0.2f);
    drawRainObject(p);
  }

  // Overlay escuro
  glColor4f(0.0f, 0.0f, 0.0f, 0.7f);
  glBegin(GL_QUADS);
  glVertex2f(-2.f, -2.f);
  glVertex2f(2.f, -2.f);
  glVertex2f(2.f, 2.f);
  glVertex2f(-2.f, 2.f);
  glEnd();

  // Animação dos elementos da UI
  float animProgress =
      std::min(1.0f, homeScreenAnimationTimer / HOMESCREEN_ANIMATION_DURATION);
  float easeOutProgress = 1.0f - pow(1.0f - animProgress, 3);

  // Título Animado
  float titleY = 0.7f - (1.0f - easeOutProgress) * 0.2f;
  float titleAlpha = animProgress;
  glColor4f(1.0f, 1.0f, 1.0f, titleAlpha);
  renderStrokeText(0.0f, titleY, 0.0009f, 3.0f, "Coleta Seletiva",
                   ALIGN_CENTER);

  // Conteúdo com atraso
  float contentAnimDelay = 0.5f;
  float contentAnimProgress =
      std::min(1.0f, std::max(0.0f, (animProgress - contentAnimDelay) /
                                        (1.0f - contentAnimDelay)));
  float contentAlpha = contentAnimProgress;

  // Campo de inserir nome
  glColor4f(0.8f, 0.8f, 0.8f, contentAlpha);
  renderStrokeText(0.0f, 0.2f, 0.0003f, 1.0f, "Digite seu nome:", ALIGN_CENTER);

  // Caixa de input
  float boxWidth = 0.8f, boxHeight = 0.12f, boxY = 0.08f;
  glColor4f(0.1f, 0.1f, 0.1f, contentAlpha * 0.8f);
  glBegin(GL_QUADS);
  glVertex2f(-boxWidth / 2, boxY - boxHeight / 2);
  glVertex2f(boxWidth / 2, boxY - boxHeight / 2);
  glVertex2f(boxWidth / 2, boxY + boxHeight / 2);
  glVertex2f(-boxWidth / 2, boxY + boxHeight / 2);
  glEnd();

  glColor4f(1.0f, 1.0f, 1.0f, contentAlpha);
  glBegin(GL_LINE_LOOP);
  glVertex2f(-boxWidth / 2, boxY - boxHeight / 2);
  glVertex2f(boxWidth / 2, boxY - boxHeight / 2);
  glVertex2f(boxWidth / 2, boxY + boxHeight / 2);
  glVertex2f(-boxWidth / 2, boxY + boxHeight / 2);
  glEnd();

  // Texto do nome com cursor
  std::string nameWithCursor = currentPlayerName;
  if (glutGet(GLUT_ELAPSED_TIME) % 1000 < 500) {
    nameWithCursor += "_";
  }
  glColor4f(1.0f, 1.0f, 1.0f, contentAlpha);
  renderStrokeText(0.0f, 0.06f, 0.0004f, 2.0f, nameWithCursor.c_str(),
                   ALIGN_CENTER);

  // Botão Iniciar
  glPushMatrix();
  float buttonScale = 0.95f + contentAnimProgress * 0.05f;
  glTranslatef(homeButton.x, homeButton.y, 0.0f);
  glScalef(buttonScale, buttonScale, 1.0f);
  glTranslatef(-homeButton.x, -homeButton.y, 0.0f);
  homeButton.draw();
  glPopMatrix();

  // Ranking
  float rankingAnimDelay = 0.8f;
  float rankingAnimProgress =
      std::min(1.0f, std::max(0.0f, (animProgress - rankingAnimDelay) /
                                        (1.0f - rankingAnimDelay)));
  float rankingAlpha = rankingAnimProgress;

  glColor4f(1.0f, 1.0f, 0.0f, rankingAlpha);
  renderStrokeText(0.0f, -0.3f, 0.0004f, 2.0f, "Ranking (Top 5)", ALIGN_CENTER);

  float currentY = -0.4f;
  glColor4f(1.0f, 1.0f, 1.0f, rankingAlpha);
  for (size_t i = 0; i < ranking.size() && i < MAX_RANKING_DISPLAY_ENTRIES;
       ++i) {
    char buffer[100];
    sprintf(buffer, "%d. %s - %d", (int)i + 1, ranking[i].name.c_str(),
            ranking[i].score);
    renderStrokeText(0.0f, currentY, 0.0003f, 1.0f, buffer, ALIGN_CENTER);
    currentY -= 0.08f;
  }
  if (ranking.empty()) {
    renderStrokeText(0.0f, currentY, 0.00025f, 1.0f,
                     "Nenhuma pontuacao registrada.", ALIGN_CENTER);
  }
}

void drawPauseMenu(const std::vector<Button> &pauseButtons) {
  // Overlay escuro
  glColor4f(0.0f, 0.0f, 0.0f, 0.8f);
  glBegin(GL_QUADS);
  glVertex2f(-2.f, -2.f);
  glVertex2f(2.f, -2.f);
  glVertex2f(2.f, 2.f);
  glVertex2f(-2.f, 2.f);
  glEnd();

  // Título PAUSADO
  glColor3f(1.0f, 1.0f, 1.0f);
  renderStrokeText(0.0f, 0.5f, 0.0008f, 3.0f, "PAUSADO", ALIGN_CENTER);

  // Botões
  for (const auto &button : pauseButtons) {
    button.draw();
  }
}

void drawGamePlayScreen(const std::vector<FallingObject> &objects,
                        const Basket &basket, int score, int misses) {
  // Blur no fundo
  glColor4f(0.1f, 0.1f, 0.1f, 0.45f);
  glBegin(GL_QUADS);
  glVertex2f(-2.f, -2.f);
  glVertex2f(2.f, -2.f);
  glVertex2f(2.f, 2.f);
  glVertex2f(-2.f, 2.f);
  glEnd();

  // Desenha objetos
  for (const auto &obj : objects) {
    const_cast<FallingObject &>(obj).draw();
  }

  // Desenha cesta
  glPushMatrix();
  glTranslatef(basket.x, basket.y, 0.0f);
  const_cast<Basket &>(basket).draw();
  glPopMatrix();

  // HUD
  char scoreText[50];
  sprintf(scoreText, "Pontos: %d", score);
  glColor3f(1.0f, 1.0f, 1.0f);
  renderBitmapText(-0.95f, 0.9f, GLUT_BITMAP_HELVETICA_18, scoreText);

  char missesText[50];
  sprintf(missesText, "Erros: %d/%d", misses, MAX_MISSES);
  glColor3f(1.0f, 0.5f, 0.5f);
  renderBitmapText(-0.95f, 0.85f, GLUT_BITMAP_HELVETICA_18, missesText);

  const char *wasteTypeText;
  switch (basket.wasteType) {
  case PAPER:
    wasteTypeText = "Papel";
    break;
  case PLASTIC:
    wasteTypeText = "Plastico";
    break;
  case METAL:
    wasteTypeText = "Metal";
    break;
  case GLASS:
    wasteTypeText = "Vidro";
    break;
  case ORGANIC:
    wasteTypeText = "Organico";
    break;
  default:
    wasteTypeText = "Desconhecido";
    break;
  }

  glColor3f(0.8f, 0.8f, 0.8f);
  renderBitmapText(-0.95f, 0.80f, GLUT_BITMAP_HELVETICA_18, "Lixeira: ");

  const GLfloat *textColor = COLOR_TABLE[basket.wasteType];
  glColor3f(textColor[0], textColor[1], textColor[2]);
  renderBitmapText(-0.75f, 0.80f, GLUT_BITMAP_HELVETICA_18, wasteTypeText);
  renderBitmapText(0.6f, 0.9f, GLUT_BITMAP_HELVETICA_18, "'P' para Pausar");
}

void drawGameOverScreen(const std::vector<TrashParticle> &trashRain,
                        const Basket &basket, float gameOverBasketY,
                        float gameOverBasketAngle, float gameOverAnimationTimer,
                        int displayedScore,
                        const std::vector<PlayerScore> &ranking,
                        const std::string &currentPlayerName,
                        int finalScoreHolder) {
  float animProgress =
      std::min(1.0f, gameOverAnimationTimer / GAMEOVER_ANIMATION_DURATION);

  // Chuva de lixo
  for (const auto &p : trashRain) {
    drawRainObject(p);
  }

  // Cesta caindo
  glPushMatrix();
  glTranslatef(basket.x, gameOverBasketY, 0.0f);
  glRotatef(gameOverBasketAngle, 0.0f, 0.0f, 1.0f);
  const_cast<Basket &>(basket).draw();
  glPopMatrix();

  // Overlay escuro
  glColor4f(0.0f, 0.0f, 0.0f, 0.8f * animProgress);
  glBegin(GL_QUADS);
  glVertex2f(-2.f, -2.f);
  glVertex2f(2.f, -2.f);
  glVertex2f(2.f, 2.f);
  glVertex2f(-2.f, 2.f);
  glEnd();

  // Texto só aparece após 70% da animação
  if (animProgress > 0.7f) {
    const float startY = 0.6f, titleScale = 0.001f, scoreScale = 0.0005f,
                instructionScale = 0.0003f, rankingTitleScale = 0.0004f,
                rankingEntryScale = 0.00035f;
    const float lineSpacingTitle = 0.35f, lineSpacingScore = 0.2f,
                lineSpacingInstructions = 0.1f, lineSpacingRankingTitle = 0.2f,
                lineSpacingRankingEntry = 0.08f;

    float currentY = startY;
    char buffer[100];
    float textAlpha = (animProgress - 0.7f) / 0.3f;
    float titleY = startY + (1.0f - animProgress) * 0.5f;

    // Título com sombra
    glColor4f(0.0f, 0.0f, 0.0f, textAlpha * 0.5f);
    renderStrokeText(0.0f, titleY - 0.005f, titleScale, 3.0f, "FIM DE JOGO",
                     ALIGN_CENTER);
    glColor4f(1.0f, 0.1f, 0.1f, textAlpha);
    renderStrokeText(0.0f, titleY, titleScale, 3.0f, "FIM DE JOGO",
                     ALIGN_CENTER);
    currentY -= lineSpacingTitle;

    // Pontuação final
    sprintf(buffer, "Pontuacao Final: %d", displayedScore);
    glColor4f(1.0f, 1.0f, 1.0f, textAlpha);
    renderStrokeText(0.0f, currentY, scoreScale, 1.5f, buffer, ALIGN_CENTER);
    currentY -= lineSpacingScore;

    // Instruções
    glColor4f(0.8f, 0.8f, 0.8f, textAlpha);
    renderStrokeText(0.0f, currentY, instructionScale, 1.0f,
                     "Pressione 'R' para Reiniciar", ALIGN_CENTER);
    currentY -= lineSpacingInstructions;
    renderStrokeText(0.0f, currentY, instructionScale, 1.0f,
                     "'M' para Menu | 'ESC' para Sair", ALIGN_CENTER);
    currentY -= lineSpacingRankingTitle;

    // Ranking
    glColor4f(1.0f, 1.0f, 0.0f, textAlpha);
    renderStrokeText(0.0f, currentY, rankingTitleScale, 2.0f, "Ranking",
                     ALIGN_CENTER);
    currentY -= lineSpacingRankingTitle * 0.7;

    bool highlighted = false;
    for (size_t i = 0; i < ranking.size() && i < MAX_RANKING_DISPLAY_ENTRIES;
         ++i) {
      sprintf(buffer, "%d. %s: %d", (int)i + 1, ranking[i].name.c_str(),
              ranking[i].score);

      if (!highlighted && ranking[i].name == currentPlayerName &&
          ranking[i].score == finalScoreHolder) {
        glColor4f(1.0f, 1.0f, 0.0f, textAlpha);
        highlighted = true;
      } else {
        glColor4f(1.0f, 1.0f, 1.0f, textAlpha);
      }
      renderStrokeText(0.0f, currentY, rankingEntryScale, 1.0f, buffer,
                       ALIGN_CENTER);
      currentY -= lineSpacingRankingEntry;
    }
  }
}
