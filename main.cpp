#include <GL/glut.h>
#include <cstdio>
#include <cstdlib>
#include <ctime>

// Incluir os módulos organizados
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
