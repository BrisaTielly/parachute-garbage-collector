#ifndef GAME_CONSTANTS_H
#define GAME_CONSTANTS_H

#include <GL/glut.h>
#include <string>

// Constantes Globais de Jogo
const int MAX_MISSES = 8; // Máximo de erros permitidos antes do game over
const std::string RANKING_FILENAME =
    "ranking.txt"; // Nome do arquivo para salvar o ranking
const int MAX_RANKING_DISPLAY_ENTRIES =
    5;                                 // Quantidade de scores a serem exibidos
const int MAX_PLAYER_NAME_LENGTH = 15; // Comprimento máximo do nome do jogador

// Variáveis Globais para dificuldade dinâmica
const int MAX_NUM_OBJECTS =
    6; // Máximo de objetos caindo simultaneamente (8 níveis)
const float INITIAL_MIN_OBJECT_SPEED =
    0.004f; // Velocidade mínima inicial (mais lenta)
const float INITIAL_MAX_OBJECT_SPEED_OFFSET =
    0.002f; // Componente aleatória inicial (menor)
const float SPEED_INCREASE_RATE = 0.00015f; // Taxa de incremento de velocidade
const float MAX_OBJECT_SPEED =
    0.035f; // Velocidade máxima dos objetos (aumentada para 8 níveis)
const int INITIAL_SCORE_FOR_DIFFICULTY =
    3; // Primeiros pontos para aumentar dificuldade
const float DIFFICULTY_SCALING_FACTOR =
    1.4f;                            // Fator de escalonamento da dificuldade
const int SPAWN_DELAY_INITIAL = 120; // Delay inicial entre spawns (frames)
const int SPAWN_DELAY_MINIMUM = 30;  // Delay mínimo entre spawns

// Animação
const float GAMEOVER_ANIMATION_DURATION =
    1.5f; // Duração total da animação em segundos
const float HOMESCREEN_ANIMATION_DURATION =
    2.0f; // Duração da animação de entrada

// Estados do Jogo
enum GameState { STATE_HOME, STATE_PLAYING, STATE_PAUSED, STATE_GAMEOVER };

// Tipos de Lixo
enum WASTE_TYPE { PAPER = 0, PLASTIC, METAL, GLASS, ORGANIC, WASTE_TYPE_COUNT };

// Tabela de cores RGB
static const GLfloat COLOR_TABLE[WASTE_TYPE_COUNT][3] = {
    {0.0f, 0.5f, 0.8f},  // PAPER (Azul)
    {0.8f, 0.2f, 0.2f},  // PLASTIC (Vermelho)
    {0.9f, 0.8f, 0.1f},  // METAL (Amarelo)
    {0.2f, 0.7f, 0.2f},  // GLASS (Verde)
    {0.5f, 0.35f, 0.05f} // ORGANIC (Marrom)
};

// Alinhamento de texto
enum Alignment { ALIGN_LEFT, ALIGN_CENTER };

// Constantes de Áudio
const std::string AUDIO_DIR = "assets/audio/";

// Arquivos de música
const std::string MUSIC_MENU = AUDIO_DIR + "menu_theme.ogg";
const std::string MUSIC_GAMEPLAY = AUDIO_DIR + "gameplay_theme.ogg";
const std::string MUSIC_GAMEOVER = AUDIO_DIR + "gameover_theme.wav";

// Arquivos de efeitos sonoros
const std::string SOUND_COLLECT_CORRECT = AUDIO_DIR + "collect_correct.ogg";
const std::string SOUND_COLLECT_WRONG = AUDIO_DIR + "collect_wrong.ogg";
const std::string SOUND_BUTTON_CLICK = AUDIO_DIR + "button_click.wav";

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#endif // GAME_CONSTANTS_H
