#include <GL/glut.h>
#include <cstdio>
#include <cstdlib>
#include <ctime>

// Incluir os módulos organizados
#include "AudioManager.h"
#include "GameConstants.h"
#include "GameLoop.h"
#include "GameState.h"
#include "InputHandler.h"
#include "RankingSystem.h"
#include "Scenery.h"

// Função Principal
int main(int argc, char **argv) {
    // Inicializa o gerador de números aleatórios
    srand(static_cast<unsigned int>(time(0)));

    // Carrega o ranking do arquivo ao iniciar o jogo
    loadRanking(ranking);

    // Inicializar sistema de áudio
    if (audioManager.initialize()) {
      printf("Sistema de áudio inicializado com sucesso!\n");

      // Carregar arquivos de música (opcional - se não existirem, o jogo
      // continua sem áudio)
      audioManager.loadMusic("menu", MUSIC_MENU);
      audioManager.loadMusic("gameplay", MUSIC_GAMEPLAY);
      audioManager.loadMusic("gameover", MUSIC_GAMEOVER);

      // Carregar efeitos sonoros
      audioManager.loadSound("collect_correct", SOUND_COLLECT_CORRECT);
      audioManager.loadSound("collect_wrong", SOUND_COLLECT_WRONG);
      audioManager.loadSound("button_click", SOUND_BUTTON_CLICK);

      // Configurar volumes
      audioManager.setMusicVolume(16); // Volume mais baixo para música de fundo
      audioManager.setSoundVolume(96); // Volume médio para efeitos

      // Iniciar música do menu
      audioManager.playMusic("menu");
    } else {
      printf("Aviso: Sistema de áudio não pôde ser inicializado. O jogo "
             "continuará sem som.\n");
    }

    // Configurações iniciais do GLUT
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_ALPHA | GLUT_MULTISAMPLE);
    glutInitWindowSize(windowWidth, windowHeight);
    glutInitWindowPosition(50, 50);
    glutCreateWindow("Coleta Seletiva");

    // Habilita configurações do OpenGL para transparência e suavização de linhas
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_LINE_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);

    // Inicializações dos componentes do jogo
    updateWindowTitle();
    initUrbanScenery(cityscape);
    initPauseMenu();
    initHomeScreen();

    // Registra as funções de callback do GLUT
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(specialKeyboard);
    glutSpecialUpFunc(specialKeyboardUp);
    glutMouseFunc(mouseClick);
    glutPassiveMotionFunc(mousePassiveMotion);
    glutTimerFunc(0, update, 0);

    // Entra no loop principal de eventos do GLUT
    glutMainLoop();
    return 0;
}
