#include "InputHandler.h"
#include "GameConstants.h"
#include "GameState.h"
#include <GL/glut.h>
#include <cstdlib>

void convertMouseToGameCoords(int mouseX, int mouseY, float &gameX,
                              float &gameY) {
  float aspect = (float)windowWidth / (float)windowHeight;
  float worldLeft, worldRight, worldBottom, worldTop;

  if (aspect >= 1.0f) {
    worldLeft = -aspect;
    worldRight = aspect;
    worldBottom = -1.0f;
    worldTop = 1.0f;
  } else {
    worldLeft = -1.0f;
    worldRight = 1.0f;
    worldBottom = -1.0f / aspect;
    worldTop = 1.0f / aspect;
  }

  gameX = worldLeft + (mouseX / (float)windowWidth) * (worldRight - worldLeft);
  gameY = worldTop - (mouseY / (float)windowHeight) * (worldTop - worldBottom);
}

void keyboard(unsigned char key, int x, int y) {
  (void)x; 
  (void)y; 
  switch (gameState) {
  case STATE_HOME:
    if (key == 8) { // Backspace
      if (!currentPlayerName.empty()) {
        currentPlayerName.pop_back();
      }
    } else if (key >= 32 && key <= 126) { // Caracteres imprimíveis
      if (currentPlayerName.length() < MAX_PLAYER_NAME_LENGTH) {
        currentPlayerName += key;
      }
    } else if (key == 13) { // Enter
      resetGame();
    } else if (key == 27) { // ESC
      exit(0);
    }
    break;
  case STATE_PLAYING:
    if (key == 'p' || key == 'P') {
      gameState = STATE_PAUSED;
    } else if (key == 27) {
      exit(0);
    } else if (key >= '1' && key <= '5') {
      basket.wasteType = static_cast<WASTE_TYPE>(key - '1');
    }
    break;
  case STATE_PAUSED:
    if (key == 'p' || key == 'P') {
      gameState = STATE_PLAYING;
    } else if (key == 27) {
      exit(0);
    }
    break;
  case STATE_GAMEOVER:
    if (key == 'r' || key == 'R') {
      resetGame();
    } else if (key == 'm' || key == 'M') {
      goToHomeScreen();
    } else if (key == 27) {
      exit(0);
    }
    break;
  }
}

void specialKeyboard(int key, int x, int y) {
  (void)x;
  (void)y;
  if (gameState != STATE_PLAYING) {
    return;
  }

  if (key == GLUT_KEY_LEFT) {
    key_a_pressed = true;
  }
  if (key == GLUT_KEY_RIGHT) {
    key_d_pressed = true;
  }
}

void specialKeyboardUp(int key, int x, int y) {
  (void)x;
  (void)y;
  if (key == GLUT_KEY_LEFT) {
    key_a_pressed = false;
  }
  if (key == GLUT_KEY_RIGHT) {
    key_d_pressed = false;
  }
}

void mouseClick(int button, int state, int x, int y) {
  if (button != GLUT_LEFT_BUTTON || state != GLUT_DOWN) {
    return;
  }

  convertMouseToGameCoords(x, y, mouseGameX, mouseGameY);

  if (gameState == STATE_PAUSED) {
    if (pauseButtons[0].isInside(mouseGameX, mouseGameY))
      gameState = STATE_PLAYING;
    if (pauseButtons[1].isInside(mouseGameX, mouseGameY))
      resetGame();
    if (pauseButtons[2].isInside(mouseGameX, mouseGameY))
      goToHomeScreen();
    if (pauseButtons[3].isInside(mouseGameX, mouseGameY))
      exit(0);
  } else if (gameState == STATE_HOME) {
    if (homeButton.isInside(mouseGameX, mouseGameY)) {
      resetGame();
    }
  }
}

void mousePassiveMotion(int x, int y) {
  convertMouseToGameCoords(x, y, mouseGameX, mouseGameY);
  bool needsRedraw = false;

  if (gameState == STATE_PAUSED) {
    for (size_t i = 0; i < pauseButtons.size(); ++i) {
      bool isInside = pauseButtons[i].isInside(mouseGameX, mouseGameY);
      if (pauseButtons[i].isHovered != isInside) {
        pauseButtons[i].isHovered = isInside;
        needsRedraw = true;
      }
    }
  } else if (gameState == STATE_HOME) {
    bool isInside = homeButton.isInside(mouseGameX, mouseGameY);
    if (homeButton.isHovered != isInside) {
      homeButton.isHovered = isInside;
      needsRedraw = true;
    }
  }

  if (needsRedraw) {
    glutPostRedisplay();
  }
}
