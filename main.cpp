#include <GL/glut.h>
#include <cstdio>      // Para sprintf (formatar strings)
#include <cstdlib>     // Para rand() e srand() (geração de números aleatórios)
#include <ctime>       // Para time() (inicializar o gerador aleatório)
#include <string>      // Para std::string
#include <vector>      // Para std::vector (armazenar objetos, prédios, partículas)
#include <cmath>       // Para funções matemáticas (sin, cos, sqrt)
#include <algorithm>   // Para std::sort e std::min/max
#include <fstream>     // Para operações com arquivos (ranking)
#include <sstream>     // Para std::stringstream (parse do ranking)

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Constantes Globais de Jogo
const int MAX_MISSES = 1; // Máximo de erros permitidos antes do game over
const std::string RANKING_FILENAME = "ranking.txt"; // Nome do arquivo para salvar o ranking
const int MAX_RANKING_DISPLAY_ENTRIES = 5; // Quantidade de scores a serem exibidos
const int MAX_PLAYER_NAME_LENGTH = 15; // Comprimento máximo do nome do jogador

// Variáveis Globais para dificuldade dinâmica
const int MAX_NUM_OBJECTS = 4; // Máximo de objetos caindo simultaneamente
const float INITIAL_MIN_OBJECT_SPEED = 0.008f; // Velocidade mínima inicial
const float INITIAL_MAX_OBJECT_SPEED_OFFSET = 0.003f; // Componente aleatória da velocidade inicial
const float SPEED_INCREASE_AMOUNT = 0.0002f; // Incremento de velocidade a cada nível
const int SCORE_INCREMENT_FOR_DIFFICULTY = 5; // Pontuação necessária para aumentar a dificuldade

// Estados do Jogo
// Enum para controlar a tela atual do jogo (Menu, Jogando, Pausado, Fim de Jogo)
enum GameState {
    STATE_HOME,
    STATE_PLAYING,
    STATE_PAUSED,
    STATE_GAMEOVER
};

// Tipos, Cores e Alinhamento
// Enum para os tipos de lixo, facilitando a leitura e a lógica do código
enum WASTE_TYPE {
    PAPER = 0,
    PLASTIC,
    METAL,
    GLASS,
    ORGANIC,
    WASTE_TYPE_COUNT
};
// Tabela de cores RGB, na mesma ordem do enum WASTE_TYPE, para as cestas e lixos
static const GLfloat COLOR_TABLE[WASTE_TYPE_COUNT][3] = {
    {0.0f, 0.5f, 0.8f},   // PAPER (Azul)
    {0.8f, 0.2f, 0.2f},   // PLASTIC (Vermelho)
    {0.9f, 0.8f, 0.1f},   // METAL (Amarelo)
    {0.2f, 0.7f, 0.2f},   // GLASS (Verde)
    {0.5f, 0.35f, 0.05f}  // ORGANIC (Marrom)
};
// Enum para alinhamento de texto na tela
enum Alignment {
    ALIGN_LEFT,
    ALIGN_CENTER
};

// Protótipos das Funções
// Declarações antecipadas de funções para que possam ser usadas em qualquer ordem
void renderStrokeText(float x, float y, float scale, float lineWidth, const char *string, Alignment align);
float getStrokeTextWidth(const char *string);

// Estruturas de Dados
// Estrutura para os botões da interface (UI)
struct Button {
    float x, y, width, height; // Posição e dimensões
    std::string text;          // Texto exibido no botão
    bool isHovered;            // Flag para indicar se o mouse está sobre o botão

    Button(float _x, float _y, float _w, float _h, const std::string& _text)
        : x(_x), y(_y), width(_w), height(_h), text(_text), isHovered(false) {}

    // Verifica se um ponto (x, y) está dentro da área do botão
    bool isInside(float px, float py) const {
        return px >= x - width / 2 && px <= x + width / 2 &&
               py >= y - height / 2 && py <= y + height / 2;
    }

    // Desenha o botão na tela
    void draw() const {
        // Muda a cor se o mouse estiver por cima (efeito hover)
        if (isHovered) {
            glColor4f(0.8f, 0.8f, 1.0f, 0.9f);
        } else {
            glColor4f(0.2f, 0.2f, 0.5f, 0.85f);
        }
        // Desenha o corpo do botão
        glBegin(GL_QUADS);
        glVertex2f(x - width / 2, y - height / 2);
        glVertex2f(x + width / 2, y - height / 2);
        glVertex2f(x + width / 2, y + height / 2);
        glVertex2f(x - width / 2, y + height / 2);
        glEnd();

        // Desenha la borda do botão
        glColor4f(1.0f, 1.0f, 1.0f, 0.9f);
        glLineWidth(2.0f);
        glBegin(GL_LINE_LOOP);
        glVertex2f(x - width / 2, y - height / 2);
        glVertex2f(x + width / 2, y - height / 2);
        glVertex2f(x + width / 2, y + height / 2);
        glVertex2f(x - width / 2, y + height / 2);
        glEnd();

        // Desenha o texto centralizado no botão
        glColor3f(1.0f, 1.0f, 1.0f);
        renderStrokeText(x, y - 0.015f, 0.0003f, 2.0f, text.c_str(), ALIGN_CENTER);
    }
};

// Estrutura para as partículas de lixo das animações
struct TrashParticle {
    float x, y, vx, vy, rotation, rotationSpeed, size, largura, altura;
    bool estatico;     // A partícula para de se mover?
    WASTE_TYPE type;   // Tipo de lixo para definir a cor/forma
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

// Variáveis de Estado do Jogo
GameState gameState = STATE_HOME;           // Controla a tela atual do jogo
std::string currentPlayerName = "Jogador";  // Nome do jogador, editável na tela inicial
std::vector<FallingObject> objects;         // Vetor de objetos que estão caindo
std::vector<Building> cityscape;            // Vetor de prédios do cenário
Basket basket;                              // A cesta do jogador
int score = 0;                              // Pontuação atual
int misses = 0;                             // Contagem de erros
int windowWidth = 600, windowHeight = 800;  // Dimensões da janela
bool key_a_pressed = false, key_d_pressed = false; // Flags para controle de movimento contínuo

// Telas e Menus
std::vector<Button> pauseButtons; // Botões da tela de pausa
Button homeButton(0.0f, -0.1f, 0.8f, 0.15f, "Iniciar"); // Botão "Iniciar" da tela principal
float mouseGameX, mouseGameY;     // Coordenadas do mouse no sistema do jogo

// Dificuldade Dinâmica
float currentMinObjectSpeed = INITIAL_MIN_OBJECT_SPEED;
float currentMaxObjectSpeedOffset = INITIAL_MAX_OBJECT_SPEED_OFFSET;
int scoreForNextDifficultyIncrease = 5;

// Animação de Game Over
std::vector<TrashParticle> trashRain;         // Partículas para a animação de "chuva de lixo"
float gameOverAnimationTimer = 0.0f;          // Timer para controlar a duração da animação
const float GAMEOVER_ANIMATION_DURATION = 1.5f; // Duração total da animação em segundos
int displayedScore = 0, finalScoreHolder = 0; // Scores para a animação de contagem
float gameOverBasketAngle = 0.0f;             // Ângulo de rotação da cesta na animação
float gameOverBasketY = 0.0f;                 // Posição Y da cesta na animação

// Animação da Tela Inicial
std::vector<TrashParticle> homeScreenDebris;      // Partículas para a tela inicial
float homeScreenAnimationTimer = 0.0f;          // Timer para a animação
const float HOMESCREEN_ANIMATION_DURATION = 2.0f; // Duração da animação de entrada

// Ranking
std::vector<PlayerScore> ranking; // Vetor para armazenar os scores carregados do arquivo

// Protótipos das Funções
void updateWindowTitle();
void renderBitmapText(float x, float y, void *font, const char *string);
void drawRainObject(const TrashParticle& p);
void loadRanking();
void saveRanking(const std::string& playerName, int finalScore);
void initUrbanScenery();
void drawUrbanScenery();
void resetGame();
void goToHomeScreen();
void triggerGameOver();
void initPauseMenu();
void initHomeScreen();
void drawHomeScreen();
void drawPauseMenu();
void drawGamePlayScreen();
void drawGameOverScreen();
void convertMouseToGameCoords(int mouseX, int mouseY, float& gameX, float& gameY);
void display();
void update(int value);
void reshape(GLsizei w, GLsizei h);
void keyboard(unsigned char key, int x, int y);
void specialKeyboard(int key, int x, int y);
void specialKeyboardUp(int key, int x, int y);
void mouseClick(int button, int state, int x, int y);
void mousePassiveMotion(int x, int y);

// Função Principal
int main(int argc, char **argv) {
    // Inicializa o gerador de números aleatórios com o tempo atual para garantir valores diferentes a cada execução
    srand(static_cast<unsigned int>(time(0)));

    // Carrega o ranking do arquivo ao iniciar o jogo
    loadRanking();

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
    initUrbanScenery();
    initPauseMenu();
    initHomeScreen();

    // Registra as funções de callback do GLUT, que respondem a eventos
    glutDisplayFunc(display);             // O que desenhar
    glutReshapeFunc(reshape);             // O que fazer quando a janela é redimensionada
    glutKeyboardFunc(keyboard);           // O que fazer quando uma tecla é pressionada
    glutSpecialFunc(specialKeyboard);     // O que fazer para teclas especiais (setas)
    glutSpecialUpFunc(specialKeyboardUp); // O que fazer quando uma tecla especial é solta
    glutMouseFunc(mouseClick);            // O que fazer com cliques do mouse
    glutPassiveMotionFunc(mousePassiveMotion); // O que fazer quando o mouse se move sem clicar
    glutTimerFunc(0, update, 0);          // Inicia o loop de atualização da lógica do jogo

    // Entra no loop principal de eventos do GLUT, que processa todos os eventos
    glutMainLoop();
    return 0;
}

// Construtores das Estruturas
// Construtor do objeto que cai: define um tamanho padrão e o inicializa
FallingObject::FallingObject() {
    size = 0.12f;
    respawn(); // Chama respawn para definir posição inicial, velocidade e tipo
}
// Construtor da cesta: define suas dimensões e posição inicial
Basket::Basket() {
    width = 0.3f;
    height = 0.2f;
    x = 0.0f;                      // Começa no centro da tela
    y = -0.8f + height / 2.0f;     // Posição Y fixa na parte inferior
    wasteType = PAPER;             // Tipo inicial da cesta é Papel
    speed = 0.05f;                 // Velocidade de movimento horizontal
}

// Funções de Desenho e Update de Objetos
// Reposiciona o objeto no topo da tela com valores aleatórios
void FallingObject::respawn() {
    x = (static_cast<float>(rand()) / RAND_MAX) * 2.0f - 1.0f; // Posição X aleatória (-1 a 1)
    y = 1.0f + (static_cast<float>(rand()) / RAND_MAX) * 0.5f; // Começa um pouco acima da tela
    // Usa as velocidades dinâmicas atuais, que aumentam com a pontuação
    speed = currentMinObjectSpeed + (static_cast<float>(rand()) / RAND_MAX) * currentMaxObjectSpeedOffset;
    rotation = static_cast<float>(rand() % 360); // Ângulo de rotação inicial aleatório
    // A velocidade de rotação depende da velocidade de queda
    rotationSpeed = ((static_cast<float>(rand()) / RAND_MAX) - 0.5f) * 2.0f * speed * 100.0f;
    wasteType = static_cast<WASTE_TYPE>(rand() % WASTE_TYPE_COUNT); // Define um tipo de lixo aleatório
}
// Atualiza a posição e rotação do objeto a cada frame
void FallingObject::update() {
    y -= speed; // Move o objeto para baixo
    rotation += rotationSpeed; // Aplica a rotação
}
// Desenha o objeto na tela com base no seu tipo (wasteType)
void FallingObject::draw() {
    glPushMatrix();
    glTranslatef(x, y, 0.0f);
    glRotatef(rotation, 0.0f, 0.0f, 1.0f);
    glScalef(size, size, 1.0f);
    // O switch escolhe o conjunto de primitivas OpenGL para desenhar o lixo apropriado
    switch (wasteType) {
        case PAPER:
            glColor3f(0.9f, 0.9f, 0.85f);
            glBegin(GL_QUADS);
            glVertex2f(-0.5f, -0.2f); glVertex2f(0.5f, -0.2f); glVertex2f(0.5f, 0.2f); glVertex2f(-0.5f, 0.2f);
            glEnd();
            glColor3f(0.4f, 0.4f, 0.4f);
            glBegin(GL_LINES);
            for (int i = 0; i < 4; ++i) { float lineY = -0.15f + i * 0.1f; glVertex2f(-0.4f, lineY); glVertex2f(0.4f, lineY); }
            glEnd();
            break;
        case PLASTIC:
            glBegin(GL_QUADS);
            glColor3f(0.8f, 0.2f, 0.2f); glVertex2f(-0.25f, -0.5f); glVertex2f(0.25f, -0.5f);
            glColor3f(1.0f, 0.5f, 0.5f); glVertex2f(0.25f, 0.2f); glVertex2f(-0.25f, 0.2f);
            glEnd();
            glBegin(GL_QUADS);
            glVertex2f(-0.15f, 0.2f); glVertex2f(0.15f, 0.2f); glVertex2f(0.15f, 0.4f); glVertex2f(-0.15f, 0.4f);
            glEnd();
            glColor3f(0.6f, 0.1f, 0.1f);
            glBegin(GL_QUADS);
            glVertex2f(-0.17f, 0.4f); glVertex2f(0.17f, 0.4f); glVertex2f(0.17f, 0.5f); glVertex2f(-0.17f, 0.5f);
            glEnd();
            glColor4f(1.0f, 1.0f, 1.0f, 0.5f);
            glBegin(GL_QUADS);
            glVertex2f(-0.2f, -0.3f); glVertex2f(-0.1f, -0.3f); glVertex2f(-0.1f, 0.1f); glVertex2f(-0.2f, 0.1f);
            glEnd();
            break;
        case METAL:
            glBegin(GL_QUAD_STRIP);
            glColor3f(0.6f, 0.6f, 0.65f); glVertex2f(-0.3f, -0.5f); glVertex2f(-0.3f, 0.5f);
            glColor3f(0.9f, 0.9f, 0.95f); glVertex2f(-0.1f, -0.5f); glVertex2f(-0.1f, 0.5f);
            glColor3f(0.9f, 0.9f, 0.95f); glVertex2f(0.1f, -0.5f); glVertex2f(0.1f, 0.5f);
            glColor3f(0.6f, 0.6f, 0.65f); glVertex2f(0.3f, -0.5f); glVertex2f(0.3f, 0.5f);
            glEnd();
            glColor3f(0.5f, 0.5f, 0.55f);
            glBegin(GL_QUADS);
            glVertex2f(-0.3f, 0.5f); glVertex2f(0.3f, 0.5f); glVertex2f(0.3f, 0.4f); glVertex2f(-0.3f, 0.4f);
            glVertex2f(-0.3f, -0.5f); glVertex2f(0.3f, -0.5f); glVertex2f(0.3f, -0.4f); glVertex2f(-0.3f, -0.4f);
            glEnd();
            break;
        case GLASS:
            glColor4f(0.2f, 0.7f, 0.2f, 0.7f);
            glBegin(GL_QUADS);
            glVertex2f(-0.25f, -0.5f); glVertex2f(0.25f, -0.5f); glVertex2f(0.25f, 0.1f); glVertex2f(-0.25f, 0.1f);
            glEnd();
            glBegin(GL_TRIANGLES);
            glVertex2f(-0.25f, 0.1f); glVertex2f(0.25f, 0.1f); glVertex2f(0.15f, 0.3f);
            glVertex2f(-0.25f, 0.1f); glVertex2f(-0.15f, 0.3f); glVertex2f(0.15f, 0.3f);
            glEnd();
            glBegin(GL_QUADS);
            glVertex2f(-0.1f, 0.3f); glVertex2f(0.1f, 0.3f); glVertex2f(0.1f, 0.5f); glVertex2f(-0.1f, 0.5f);
            glEnd();
            glColor4f(1.0f, 1.0f, 1.0f, 0.6f);
            glBegin(GL_QUADS);
            glVertex2f(0.1f, -0.4f); glVertex2f(0.18f, -0.4f); glVertex2f(0.18f, 0.2f); glVertex2f(0.1f, 0.2f);
            glEnd();
            break;
        case ORGANIC:
            glBegin(GL_POLYGON);
            glColor3f(0.9f, 0.1f, 0.1f);
            for (int i = 0; i < 20; i++) { float ang = 2.0f * M_PI * i / 20.0f; glVertex2f(cos(ang) * 0.4f, sin(ang) * 0.5f); }
            glEnd();
            glColor3f(0.4f, 0.2f, 0.0f);
            glBegin(GL_QUADS);
            glVertex2f(-0.05f, 0.4f); glVertex2f(0.05f, 0.4f); glVertex2f(0.05f, 0.6f); glVertex2f(-0.05f, 0.6f);
            glEnd();
            glColor3f(0.1f, 0.8f, 0.1f);
            glBegin(GL_TRIANGLES);
            glVertex2f(0.05f, 0.5f); glVertex2f(0.3f, 0.7f); glVertex2f(0.1f, 0.4f);
            glEnd();
            break;
    }
    glPopMatrix();
}
// Desenha a cesta na sua posição atual
void Basket::draw() {
    const float* color = COLOR_TABLE[wasteType]; // Pega a cor correspondente ao tipo de lixo atual
    // Corpo da cesta com gradiente
    glBegin(GL_QUADS);
    glColor3f(color[0] * 0.7f, color[1] * 0.7f, color[2] * 0.7f); glVertex2f(-width / 2, -height / 2); glVertex2f(width / 2, -height / 2);
    glColor3f(color[0], color[1], color[2]); glVertex2f(width / 2, height / 2); glVertex2f(-width / 2, height / 2);
    glEnd();
    // Borda superior da cesta
    glColor3f(color[0] * 0.5f, color[1] * 0.5f, color[2] * 0.5f);
    glBegin(GL_QUADS);
    glVertex2f(-width / 2 - 0.02f, height / 2); glVertex2f(width / 2 + 0.02f, height / 2); glVertex2f(width / 2 + 0.02f, height / 2 + 0.03f); glVertex2f(-width / 2 - 0.02f, height / 2 + 0.03f);
    glEnd();
    // Símbolo de reciclagem (desenhado no centro da cesta, independente da posição dela)
    glPushMatrix();
    glColor3f(1.0f, 1.0f, 1.0f); float s = 0.05f;
    for (int i = 0; i < 3; ++i) {
        glRotatef(120.0, 0, 0, 1);
        glBegin(GL_QUADS);
        glVertex2f(-s, s); glVertex2f(s, s); glVertex2f(s * 1.5, s * 1.8); glVertex2f(-s * 0.5, s * 1.8);
        glEnd();
    }
    glPopMatrix();
}
// Move a cesta horizontalmente, com limites para não sair da tela
void Basket::move(float direction) {
    x += direction * speed;
    // Limita o movimento da cesta às bordas da área de jogo (-1.0 a 1.0)
    if (x - width / 2 < -1.0f) {
        x = -1.0f + width / 2;
    }
    if (x + width / 2 > 1.0f) {
        x = 1.0f - width / 2;
    }
}
// Desenha uma partícula de lixo para as animações
void drawRainObject(const TrashParticle& p) {
    glPushMatrix();
    glTranslatef(p.x, p.y, 0.0f);
    glRotatef(p.rotation, 0.0f, 0.0f, 1.0f);
    glScalef(p.size, p.size, 1.0f);
    // A lógica de desenho é a mesma de um FallingObject
    switch (p.type) {
        case PAPER:
            glColor3f(0.9f, 0.9f, 0.85f);
            glBegin(GL_QUADS);
            glVertex2f(-0.5f, -0.2f); glVertex2f(0.5f, -0.2f); glVertex2f(0.5f, 0.2f); glVertex2f(-0.5f, 0.2f);
            glEnd();
            glColor3f(0.4f, 0.4f, 0.4f);
            glBegin(GL_LINES);
            for (int i = 0; i < 4; ++i) { float lineY = -0.15f + i * 0.1f; glVertex2f(-0.4f, lineY); glVertex2f(0.4f, lineY); }
            glEnd();
            break;
        case PLASTIC:
            glBegin(GL_QUADS);
            glColor3f(0.8f, 0.2f, 0.2f); glVertex2f(-0.25f, -0.5f); glVertex2f(0.25f, -0.5f);
            glColor3f(1.0f, 0.5f, 0.5f); glVertex2f(0.25f, 0.2f); glVertex2f(-0.25f, 0.2f);
            glEnd();
            glBegin(GL_QUADS);
            glVertex2f(-0.15f, 0.2f); glVertex2f(0.15f, 0.2f); glVertex2f(0.15f, 0.4f); glVertex2f(-0.15f, 0.4f);
            glEnd();
            glColor3f(0.6f, 0.1f, 0.1f);
            glBegin(GL_QUADS);
            glVertex2f(-0.17f, 0.4f); glVertex2f(0.17f, 0.4f); glVertex2f(0.17f, 0.5f); glVertex2f(-0.17f, 0.5f);
            glEnd();
            glColor4f(1.0f, 1.0f, 1.0f, 0.5f);
            glBegin(GL_QUADS);
            glVertex2f(-0.2f, -0.3f); glVertex2f(-0.1f, -0.3f); glVertex2f(-0.1f, 0.1f); glVertex2f(-0.2f, 0.1f);
            glEnd();
            break;
        case METAL:
            glBegin(GL_QUAD_STRIP);
            glColor3f(0.6f, 0.6f, 0.65f); glVertex2f(-0.3f, -0.5f); glVertex2f(-0.3f, 0.5f);
            glColor3f(0.9f, 0.9f, 0.95f); glVertex2f(-0.1f, -0.5f); glVertex2f(-0.1f, 0.5f);
            glColor3f(0.9f, 0.9f, 0.95f); glVertex2f(0.1f, -0.5f); glVertex2f(0.1f, 0.5f);
            glColor3f(0.6f, 0.6f, 0.65f); glVertex2f(0.3f, -0.5f); glVertex2f(0.3f, 0.5f);
            glEnd();
            glColor3f(0.5f, 0.5f, 0.55f);
            glBegin(GL_QUADS);
            glVertex2f(-0.3f, 0.5f); glVertex2f(0.3f, 0.5f); glVertex2f(0.3f, 0.4f); glVertex2f(-0.3f, 0.4f);
            glVertex2f(-0.3f, -0.5f); glVertex2f(0.3f, -0.5f); glVertex2f(0.3f, -0.4f); glVertex2f(-0.3f, -0.4f);
            glEnd();
            break;
        case GLASS:
            glColor4f(0.2f, 0.7f, 0.2f, 0.7f);
            glBegin(GL_QUADS);
            glVertex2f(-0.25f, -0.5f); glVertex2f(0.25f, -0.5f); glVertex2f(0.25f, 0.1f); glVertex2f(-0.25f, 0.1f);
            glEnd();
            glBegin(GL_TRIANGLES);
            glVertex2f(-0.25f, 0.1f); glVertex2f(0.25f, 0.1f); glVertex2f(0.15f, 0.3f);
            glVertex2f(-0.25f, 0.1f); glVertex2f(-0.15f, 0.3f); glVertex2f(0.15f, 0.3f);
            glEnd();
            glBegin(GL_QUADS);
            glVertex2f(-0.1f, 0.3f); glVertex2f(0.1f, 0.3f); glVertex2f(0.1f, 0.5f); glVertex2f(-0.1f, 0.5f);
            glEnd();
            glColor4f(1.0f, 1.0f, 1.0f, 0.6f);
            glBegin(GL_QUADS);
            glVertex2f(0.1f, -0.4f); glVertex2f(0.18f, -0.4f); glVertex2f(0.18f, 0.2f); glVertex2f(0.1f, 0.2f);
            glEnd();
            break;
        case ORGANIC:
            glBegin(GL_POLYGON);
            glColor3f(0.9f, 0.1f, 0.1f);
            for (int i = 0; i < 20; i++) { float ang = 2.0f * M_PI * i / 20.0f; glVertex2f(cos(ang) * 0.4f, sin(ang) * 0.5f); }
            glEnd();
            glColor3f(0.4f, 0.2f, 0.0f);
            glBegin(GL_QUADS);
            glVertex2f(-0.05f, 0.4f); glVertex2f(0.05f, 0.4f); glVertex2f(0.05f, 0.6f); glVertex2f(-0.05f, 0.6f);
            glEnd();
            glColor3f(0.1f, 0.8f, 0.1f);
            glBegin(GL_TRIANGLES);
            glVertex2f(0.05f, 0.5f); glVertex2f(0.3f, 0.7f); glVertex2f(0.1f, 0.4f);
            glEnd();
            break;
    }
    glPopMatrix();
}

// Funções Auxiliares de Texto e Jogo
// Atualiza o título da janela para mostrar a pontuação atual
void updateWindowTitle() {
    char title[100];
    if (gameState == STATE_PLAYING) {
        sprintf(title, "Coleta Seletiva - Pontos: %d", score);
    } else {
        sprintf(title, "Coleta Seletiva");
    }
    glutSetWindowTitle(title);
}
// Renderiza texto na tela usando fontes de bitmap (rápidas, mas não escaláveis)
void renderBitmapText(float x, float y, void *font, const char *string) {
    glRasterPos2f(x, y);
    while (*string) {
        glutBitmapCharacter(font, *string++);
    }
}
// Calcula a largura de um texto que será renderizado com fonte de traço
float getStrokeTextWidth(const char* string) {
    float width = 0;
    while (*string) {
        width += glutStrokeWidth(GLUT_STROKE_ROMAN, *string++);
    }
    return width;
}
// Renderiza texto na tela usando fontes de traço (stroke fonts), que são vetoriais e escaláveis
void renderStrokeText(float x, float y, float scale, float lineWidth, const char *string, Alignment align) {
    glPushMatrix();
    float textWidth = 0;
    // Se o alinhamento for central, calcula a largura para ajustar a posição
    if (align == ALIGN_CENTER) {
        textWidth = getStrokeTextWidth(string);
    }
    // Translada para a posição correta, ajustando pelo alinhamento
    glTranslatef(x - (textWidth / 2.0f * scale), y, 0.0f);
    glScalef(scale, scale, 1.0f); // Escala o texto
    glLineWidth(lineWidth);       // Define a espessura da linha
    while (*string) {
        glutStrokeCharacter(GLUT_STROKE_ROMAN, *string++); // Desenha cada caractere
    }
    glPopMatrix();
}
// Carrega o ranking do arquivo de texto
void loadRanking() {
    std::ifstream inFile(RANKING_FILENAME);
    if (!inFile.is_open()) {
        return; // Se o arquivo não existe, simplesmente retorna
    }
    ranking.clear();
    std::string line;
    while (std::getline(inFile, line)) {
        std::stringstream ss(line);
        std::string name;
        int score_val;
        // Assume o formato "Nome Score" e faz o parse da linha
        if (ss >> name >> score_val) {
            ranking.push_back({name, score_val});
        }
    }
    inFile.close();
    // Ordena o ranking em ordem decrescente de pontuação
    std::sort(ranking.begin(), ranking.end(), [](const PlayerScore& a, const PlayerScore& b) {
        return a.score > b.score;
    });
}
// Adiciona a pontuação final ao ranking e salva no arquivo
void saveRanking(const std::string& playerName, int finalScore) {
    loadRanking(); // Recarrega para garantir que está com a lista mais atual
    ranking.push_back({playerName, finalScore}); // Adiciona o novo score
    // Reordena
    std::sort(ranking.begin(), ranking.end(), [](const PlayerScore& a, const PlayerScore& b) {
        return a.score > b.score;
    });
    std::ofstream outFile(RANKING_FILENAME);
    if (!outFile.is_open()) {
        fprintf(stderr, "Erro ao salvar ranking.\n");
        return;
    }
    // Escreve cada entrada do ranking de volta no arquivo, sobrescrevendo o antigo
    for (const auto& ps : ranking) {
        outFile << ps.name << " " << ps.score << std::endl;
    }
    outFile.close();
}
// Inicializa o cenário urbano com prédios gerados proceduralmente
void initUrbanScenery() {
    cityscape.clear();
    srand(1337); // Usa uma seed fixa para que o cenário seja sempre o mesmo
    float worldWidth = 4.0f;
    // Gera a primeira camada de prédios (mais escuros, ao fundo)
    float current_x = -worldWidth;
    while (current_x < worldWidth) {
        Building b;
        b.x_pos = current_x;
        b.width = 0.2f + (static_cast<float>(rand()) / RAND_MAX) * 0.3f;
        b.height = 0.2f + (static_cast<float>(rand()) / RAND_MAX) * 0.6f;
        float gray = 0.15f + (static_cast<float>(rand()) / RAND_MAX) * 0.1f;
        b.r = gray; b.g = gray; b.b = gray + 0.05f;
        b.layer = 0;
        cityscape.push_back(b);
        current_x += b.width + 0.05f;
    }
    // Gera a segunda camada de prédios (mais claros, à frente)
    current_x = -worldWidth;
    while (current_x < worldWidth) {
        Building b;
        b.x_pos = current_x;
        b.width = 0.3f + (static_cast<float>(rand()) / RAND_MAX) * 0.2f;
        b.height = -0.2f + (static_cast<float>(rand()) / RAND_MAX) * 0.3f;
        float gray = 0.25f + (static_cast<float>(rand()) / RAND_MAX) * 0.1f;
        b.r = gray; b.g = gray; b.b = gray;
        b.layer = 1;
        cityscape.push_back(b);
        current_x += b.width + 0.1f;
    }
    // Ordena os prédios por camada para desenhar corretamente o efeito de profundidade
    std::sort(cityscape.begin(), cityscape.end(), [](const Building& a, const Building& b) {
        return a.layer < b.layer;
    });
    srand(static_cast<unsigned int>(time(0))); // Reseta a seed para o resto do jogo ser aleatório
}
// Desenha o cenário urbano (céu, prédios, rua)
void drawUrbanScenery() {
    float aspect = (float)windowWidth / (float)windowHeight;
    float worldLeft = -1.0, worldRight = 1.0, worldTop = 1.0, worldBottom = -1.0;
    if (aspect > 1.0) {
        worldLeft = -aspect;
        worldRight = aspect;
    } else {
        worldTop = 1.0 / aspect;
        worldBottom = -1.0 / aspect;
    }
    // Desenha o céu com um gradiente do escuro (topo) para o claro (horizonte)
    glBegin(GL_QUADS);
    glColor3f(0.1f, 0.1f, 0.3f); glVertex2f(worldLeft, worldTop); glVertex2f(worldRight, worldTop);
    glColor3f(0.9f, 0.7f, 0.4f); glVertex2f(worldRight, -0.7f); glVertex2f(worldLeft, -0.7f);
    glEnd();
    int time_ms = glutGet(GLUT_ELAPSED_TIME);
    // Desenha cada prédio
    for (const auto& b : cityscape) {
        glColor3f(b.r, b.g, b.b);
        glBegin(GL_QUADS);
        glVertex2f(b.x_pos, -0.8f); glVertex2f(b.x_pos + b.width, -0.8f); glVertex2f(b.x_pos + b.width, b.height); glVertex2f(b.x_pos, b.height);
        glEnd();
        // Desenha as janelas, algumas acesas e outras apagadas de forma "aleatória" baseada no tempo
        float window_margin = 0.1f * b.width;
        float window_size = 0.08f * b.width;
        int num_floors = static_cast<int>((b.height + 0.8f) / 0.1f);
        int num_windows_per_floor = static_cast<int>((b.width - 2 * window_margin) / (window_size * 1.5f));
        for (int i = 0; i < num_floors; ++i) {
            for (int j = 0; j < num_windows_per_floor; ++j) {
                int window_seed = static_cast<int>(b.x_pos * 100) + i * 13 + j * 7;
                if (sin((time_ms / 1000.0f) * 0.2f + window_seed) > 0.8) {
                    glColor3f(0.9f, 0.9f, 0.6f);
                } else {
                    glColor3f(b.r * 0.5f, b.g * 0.5f, b.b * 0.5f);
                }
                float wx = b.x_pos + window_margin + j * (window_size * 1.5f);
                float wy = -0.75f + i * 0.1f;
                glBegin(GL_QUADS);
                glVertex2f(wx, wy); glVertex2f(wx + window_size, wy); glVertex2f(wx + window_size, wy + 0.05f); glVertex2f(wx, wy + 0.05f);
                glEnd();
            }
        }
    }
    // Desenha a rua/chão
    glColor3f(0.2f, 0.2f, 0.2f);
    glBegin(GL_QUADS);
    glVertex2f(worldLeft, -1.0f); glVertex2f(worldRight, -1.0f); glVertex2f(worldRight, -0.8f); glVertex2f(worldLeft, -0.8f);
    glEnd();
}

// Funções Principais de Jogo
// Ativa o estado de "Game Over" e salva a pontuação
void triggerGameOver() {
    gameState = STATE_GAMEOVER;
    finalScoreHolder = score;   // Guarda a pontuação final para a animação
    gameOverBasketY = basket.y; // Guarda a posição inicial da cesta para a animação
    saveRanking(currentPlayerName, score);
    loadRanking(); // Recarrega para exibir o ranking atualizado na tela de game over
}

// Inicializa as partículas de animação para a tela inicial
void initHomeScreen() {
    homeScreenDebris.clear();
    for (int i = 0; i < 50; ++i) { // Cria 50 partículas flutuantes
        TrashParticle p;
        p.x = (static_cast<float>(rand()) / RAND_MAX) * 4.0f - 2.0f;
        p.y = (static_cast<float>(rand()) / RAND_MAX) * 2.0f - 1.0f;
        p.vx = ((static_cast<float>(rand()) / RAND_MAX) - 0.5f) * 0.01f; // Movimento lento
        p.vy = ((static_cast<float>(rand()) / RAND_MAX) - 0.5f) * 0.01f;
        p.size = 0.04f + (static_cast<float>(rand()) / RAND_MAX) * 0.04f;
        p.rotation = static_cast<float>(rand() % 360);
        p.rotationSpeed = ((static_cast<float>(rand()) / RAND_MAX) - 0.5f) * 0.5f;
        p.type = static_cast<WASTE_TYPE>(rand() % WASTE_TYPE_COUNT);
        p.estatico = false;
        homeScreenDebris.push_back(p);
    }
}
// Muda o estado do jogo para a tela inicial
void goToHomeScreen() {
    gameState = STATE_HOME;
    updateWindowTitle();
    loadRanking();
    homeScreenAnimationTimer = 0.0f; // Reseta o timer da animação da tela inicial
    initHomeScreen(); // Recria as partículas para um efeito diferente a cada vez
}
// Reseta o estado do jogo para começar uma nova partida
void resetGame() {
    if (currentPlayerName.empty()) {
        currentPlayerName = "Jogador"; // Nome padrão se o jogador não digitar nada
    }
    // Zera todas as variáveis de jogo
    score = 0;
    misses = 0;
    gameOverAnimationTimer = 0.0f;
    displayedScore = 0;
    finalScoreHolder = 0;
    gameOverBasketAngle = 0.0f;
    trashRain.clear();
    // Restaura a dificuldade para o nível inicial
    currentMinObjectSpeed = INITIAL_MIN_OBJECT_SPEED;
    currentMaxObjectSpeedOffset = INITIAL_MAX_OBJECT_SPEED_OFFSET;
    scoreForNextDifficultyIncrease = 5;
    // Limpa e recria os objetos na tela
    objects.clear();
    objects.push_back(FallingObject());
    basket.x = 0.0f;
    basket.wasteType = PAPER;
    // Muda o estado e atualiza o título da janela
    gameState = STATE_PLAYING;
    updateWindowTitle();
}
// Cria os botões para o menu de pausa
void initPauseMenu() {
    pauseButtons.clear();
    pauseButtons.push_back(Button(0.0f, 0.3f, 0.8f, 0.15f, "Continuar"));
    pauseButtons.push_back(Button(0.0f, 0.1f, 0.8f, 0.15f, "Reiniciar"));
    pauseButtons.push_back(Button(0.0f, -0.1f, 0.8f, 0.15f, "Tela Inicial"));
    pauseButtons.push_back(Button(0.0f, -0.3f, 0.8f, 0.15f, "Sair do Jogo"));
}

// Funções de Desenho das Telas
// Função de desenho principal, chamada a cada frame pelo GLUT
void display() {
    glClear(GL_COLOR_BUFFER_BIT); // Limpa a tela
    drawUrbanScenery();           // Desenha o fundo primeiro

    // Escolhe qual tela desenhar com base no estado atual do jogo
    switch (gameState) {
        case STATE_HOME:
            drawHomeScreen();
            break;
        case STATE_PLAYING:
            drawGamePlayScreen();
            break;
        case STATE_PAUSED:
            drawGamePlayScreen(); // Desenha o jogo por baixo
            drawPauseMenu();      // Desenha o menu de pausa por cima
            break;
        case STATE_GAMEOVER:
            drawGameOverScreen();
            break;
    }
    glutSwapBuffers(); // Troca os buffers para exibir o que foi desenhado
}
// Desenha todos os elementos da tela inicial
void drawHomeScreen() {
    // 1. Desenha os detritos flutuantes no fundo para dar um efeito visual
    for (const auto& p : homeScreenDebris) {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glColor4f(1.0f, 1.0f, 1.0f, 0.2f); // Cor branca com alfa baixo para sutileza
        drawRainObject(p);
    }
    // 2. Desenha um overlay escuro para destacar os elementos da UI
    glColor4f(0.0f, 0.0f, 0.0f, 0.7f);
    glBegin(GL_QUADS);
    glVertex2f(-2.f, -2.f); glVertex2f(2.f, -2.f); glVertex2f(2.f, 2.f); glVertex2f(-2.f, 2.f);
    glEnd();
    // 3. Anima os elementos da UI (título, input, botões) para entrarem na tela
    float animProgress = std::min(1.0f, homeScreenAnimationTimer / HOMESCREEN_ANIMATION_DURATION);
    float easeOutProgress = 1.0f - pow(1.0f - animProgress, 3); // Curva suave de animação
    // Título Animado
    float titleY = 0.7f - (1.0f - easeOutProgress) * 0.2f; // Título desliza de cima para baixo
    float titleAlpha = animProgress; // Título aparece gradualmente (fade-in)
    glColor4f(1.0f, 1.0f, 1.0f, titleAlpha);
    renderStrokeText(0.0f, titleY, 0.0009f, 3.0f, "Coleta Seletiva", ALIGN_CENTER);
    // Animação para o resto do conteúdo, com um pequeno atraso
    float contentAnimDelay = 0.5f;
    float contentAnimProgress = std::min(1.0f, std::max(0.0f, (animProgress - contentAnimDelay) / (1.0f - contentAnimDelay)));
    float contentAlpha = contentAnimProgress;
    // Campo de Inserir Nome
    glColor4f(0.8f, 0.8f, 0.8f, contentAlpha);
    renderStrokeText(0.0f, 0.2f, 0.0003f, 1.0f, "Digite seu nome:", ALIGN_CENTER);
    // Caixa de input para o nome
    float boxWidth = 0.8f, boxHeight = 0.12f, boxY = 0.08f;
    glColor4f(0.1f, 0.1f, 0.1f, contentAlpha * 0.8f);
    glBegin(GL_QUADS);
    glVertex2f(-boxWidth / 2, boxY - boxHeight / 2); glVertex2f(boxWidth / 2, boxY - boxHeight / 2); glVertex2f(boxWidth / 2, boxY + boxHeight / 2); glVertex2f(-boxWidth / 2, boxY + boxHeight / 2);
    glEnd();
    glColor4f(1.0f, 1.0f, 1.0f, contentAlpha);
    glBegin(GL_LINE_LOOP);
    glVertex2f(-boxWidth / 2, boxY - boxHeight / 2); glVertex2f(boxWidth / 2, boxY - boxHeight / 2); glVertex2f(boxWidth / 2, boxY + boxHeight / 2); glVertex2f(-boxWidth / 2, boxY + boxHeight / 2);
    glEnd();
    // Texto do nome com cursor piscante
    std::string nameWithCursor = currentPlayerName;
    if (glutGet(GLUT_ELAPSED_TIME) % 1000 < 500) {
        nameWithCursor += "_";
    }
    glColor4f(1.0f, 1.0f, 1.0f, contentAlpha);
    renderStrokeText(0.0f, 0.06f, 0.0004f, 2.0f, nameWithCursor.c_str(), ALIGN_CENTER);
    // Botão Iniciar com animação de "pop"
    glPushMatrix();
    float buttonScale = 0.95f + contentAnimProgress * 0.05f;
    glTranslatef(homeButton.x, homeButton.y, 0.0f);
    glScalef(buttonScale, buttonScale, 1.0f);
    glTranslatef(-homeButton.x, -homeButton.y, 0.0f);
    homeButton.draw();
    glPopMatrix();
    // Ranking
    float rankingAnimDelay = 0.8f;
    float rankingAnimProgress = std::min(1.0f, std::max(0.0f, (animProgress - rankingAnimDelay) / (1.0f - rankingAnimDelay)));
    float rankingAlpha = rankingAnimProgress;
    glColor4f(1.0f, 1.0f, 0.0f, rankingAlpha);
    renderStrokeText(0.0f, -0.3f, 0.0004f, 2.0f, "Ranking (Top 5)", ALIGN_CENTER);
    float currentY = -0.4f;
    glColor4f(1.0f, 1.0f, 1.0f, rankingAlpha);
    for (size_t i = 0; i < ranking.size() && i < MAX_RANKING_DISPLAY_ENTRIES; ++i) {
        char buffer[100];
        sprintf(buffer, "%d. %s - %d", (int)i + 1, ranking[i].name.c_str(), ranking[i].score);
        renderStrokeText(0.0f, currentY, 0.0003f, 1.0f, buffer, ALIGN_CENTER);
        currentY -= 0.08f;
    }
    if (ranking.empty()) {
        renderStrokeText(0.0f, currentY, 0.00025f, 1.0f, "Nenhuma pontuacao registrada.", ALIGN_CENTER);
    }
}
// Desenha a tela de Pausa
void drawPauseMenu() {
    // Desenha um overlay escuro sobre o jogo
    glColor4f(0.0f, 0.0f, 0.0f, 0.8f);
    glBegin(GL_QUADS);
    glVertex2f(-2.f, -2.f); glVertex2f(2.f, -2.f); glVertex2f(2.f, 2.f); glVertex2f(-2.f, 2.f);
    glEnd();
    // Escreve "PAUSADO" e desenha os botões
    glColor3f(1.0f, 1.0f, 1.0f);
    renderStrokeText(0.0f, 0.5f, 0.0008f, 3.0f, "PAUSADO", ALIGN_CENTER);
    for (size_t i = 0; i < pauseButtons.size(); ++i) {
        pauseButtons[i].draw();
    }
}
// Desenha os elementos da tela de jogo (HUD, cesta, objetos)
void drawGamePlayScreen() {
    // Desenha um leve blur no fundo para destacar a área de jogo
    glColor4f(0.1f, 0.1f, 0.1f, 0.45f);
    glBegin(GL_QUADS);
    glVertex2f(-2.f, -2.f); glVertex2f(2.f, -2.f); glVertex2f(2.f, 2.f); glVertex2f(-2.f, 2.f);
    glEnd();
    // Desenha cada objeto caindo e a cesta
    for (size_t i = 0; i < objects.size(); ++i) {
        objects[i].draw();
    }
    glPushMatrix();
    glTranslatef(basket.x, basket.y, 0.0f);
    basket.draw();
    glPopMatrix();
    // Desenha a Interface do Usuário (HUD) com informações
    char scoreText[50];
    sprintf(scoreText, "Pontos: %d", score);
    glColor3f(1.0f, 1.0f, 1.0f);
    renderBitmapText(-0.95f, 0.9f, GLUT_BITMAP_HELVETICA_18, scoreText);
    char missesText[50];
    sprintf(missesText, "Erros: %d/%d", misses, MAX_MISSES);
    glColor3f(1.0f, 0.5f, 0.5f);
    renderBitmapText(-0.95f, 0.85f, GLUT_BITMAP_HELVETICA_18, missesText);
    const char* wasteTypeText;
    switch (basket.wasteType) {
        case PAPER:   wasteTypeText = "Papel";    break;
        case PLASTIC: wasteTypeText = "Plastico"; break;
        case METAL:   wasteTypeText = "Metal";    break;
        case GLASS:   wasteTypeText = "Vidro";    break;
        case ORGANIC: wasteTypeText = "Organico"; break;
        default:      wasteTypeText = "Desconhecido"; break;
    }
    glColor3f(0.8f, 0.8f, 0.8f);
    renderBitmapText(-0.95f, 0.80f, GLUT_BITMAP_HELVETICA_18, "Lixeira: ");
    const GLfloat* textColor = COLOR_TABLE[basket.wasteType];
    glColor3f(textColor[0], textColor[1], textColor[2]);
    renderBitmapText(-0.75f, 0.80f, GLUT_BITMAP_HELVETICA_18, wasteTypeText);
    renderBitmapText(0.6f, 0.9f, GLUT_BITMAP_HELVETICA_18, "'P' para Pausar");
}
// Desenha la tela de Fim de Jogo com suas animações
void drawGameOverScreen() {
    // Calcula o progresso da animação (de 0.0 a 1.0)
    float animProgress = std::min(1.0f, gameOverAnimationTimer / GAMEOVER_ANIMATION_DURATION);
    // Desenha a chuva de lixo da animação
    for (const auto& p : trashRain) {
        drawRainObject(p);
    }
    // Desenha a cesta caindo e girando
    glPushMatrix();
    glTranslatef(basket.x, gameOverBasketY, 0.0f);
    glRotatef(gameOverBasketAngle, 0.0f, 0.0f, 1.0f);
    basket.draw();
    glPopMatrix();
    // Desenha um overlay escuro semi-transparente que aparece gradualmente
    glColor4f(0.0f, 0.0f, 0.0f, 0.8f * animProgress);
    glBegin(GL_QUADS);
    glVertex2f(-2.f, -2.f); glVertex2f(2.f, -2.f); glVertex2f(2.f, 2.f); glVertex2f(-2.f, 2.f);
    glEnd();
    // O texto só aparece após uma parte da animação ter ocorrido
    if (animProgress > 0.7f) {
        const float startY = 0.6f, titleScale = 0.001f, scoreScale = 0.0005f, instructionScale = 0.0003f, rankingTitleScale = 0.0004f, rankingEntryScale = 0.00035f;
        const float lineSpacingTitle = 0.35f, lineSpacingScore = 0.2f, lineSpacingInstructions = 0.1f, lineSpacingRankingTitle = 0.2f, lineSpacingRankingEntry = 0.08f;
        float currentY = startY;
        char buffer[100];
        float textAlpha = (animProgress - 0.7f) / 0.3f; // Alpha do texto (fade-in)
        float titleY = startY + (1.0f - animProgress) * 0.5f; // Título "cai" na tela
        // Título com sombra
        glColor4f(0.0f, 0.0f, 0.0f, textAlpha * 0.5f);
        renderStrokeText(0.0f, titleY - 0.005f, titleScale, 3.0f, "FIM DE JOGO", ALIGN_CENTER);
        glColor4f(1.0f, 0.1f, 0.1f, textAlpha);
        renderStrokeText(0.0f, titleY, titleScale, 3.0f, "FIM DE JOGO", ALIGN_CENTER);
        currentY -= lineSpacingTitle;
        // Pontuação final (com animação de contagem)
        sprintf(buffer, "Pontuacao Final: %d", displayedScore);
        glColor4f(1.0f, 1.0f, 1.0f, textAlpha);
        renderStrokeText(0.0f, currentY, scoreScale, 1.5f, buffer, ALIGN_CENTER);
        currentY -= lineSpacingScore;
        // Instruções
        glColor4f(0.8f, 0.8f, 0.8f, textAlpha);
        renderStrokeText(0.0f, currentY, instructionScale, 1.0f, "Pressione 'R' para Reiniciar", ALIGN_CENTER);
        currentY -= lineSpacingInstructions;
        renderStrokeText(0.0f, currentY, instructionScale, 1.0f, "'M' para Menu | 'ESC' para Sair", ALIGN_CENTER);
        currentY -= lineSpacingRankingTitle;
        // Ranking
        glColor4f(1.0f, 1.0f, 0.0f, textAlpha);
        renderStrokeText(0.0f, currentY, rankingTitleScale, 2.0f, "Ranking", ALIGN_CENTER);
        currentY -= lineSpacingRankingTitle * 0.7;
        bool highlighted = false; // Para destacar a pontuação do jogador atual
        for (size_t i = 0; i < ranking.size() && i < MAX_RANKING_DISPLAY_ENTRIES; ++i) {
            sprintf(buffer, "%d. %s: %d", (int)i + 1, ranking[i].name.c_str(), ranking[i].score);
            // Destaca a pontuação recém-adquirida em amarelo
            if (!highlighted && ranking[i].name == currentPlayerName && ranking[i].score == finalScoreHolder) {
                glColor4f(1.0f, 1.0f, 0.0f, textAlpha);
                highlighted = true;
            } else {
                glColor4f(1.0f, 1.0f, 1.0f, textAlpha);
            }
            renderStrokeText(0.0f, currentY, rankingEntryScale, 1.0f, buffer, ALIGN_CENTER);
            currentY -= lineSpacingRankingEntry;
        }
    }
}

// Funções de Lógica e Callback
// Função de atualização da lógica do jogo, chamada a cada ~16ms
void update(int value) {
    float dt = 16.0f / 1000.0f; // Delta time: tempo desde a última atualização

    if (gameState == STATE_HOME) {
        // Lógica da Tela Inicial
        // Avança o timer da animação
        if (homeScreenAnimationTimer < HOMESCREEN_ANIMATION_DURATION) {
            homeScreenAnimationTimer += dt;
        }
        // Move as partículas de fundo
        for (auto& p : homeScreenDebris) {
            p.x += p.vx * dt;
            p.y += p.vy * dt;
            p.rotation += p.rotationSpeed * dt;
            // Efeito de "wrap around" para as partículas que saem da tela
            if (p.x > 2.2f) p.x = -2.2f;
            if (p.x < -2.2f) p.x = 2.2f;
            if (p.y > 1.2f) p.y = -1.2f;
            if (p.y < -1.2f) p.y = 1.2f;
        }
    } else if (gameState == STATE_PLAYING) {
        // Lógica do Jogo em Andamento
        // Movimento contínuo da cesta se as teclas A/D estiverem pressionadas
        if (key_a_pressed && !key_d_pressed) {
            basket.move(-1.0f);
        } else if (key_d_pressed && !key_a_pressed) {
            basket.move(1.0f);
        }

        for (size_t i = 0; i < objects.size(); ++i) {
            objects[i].update();
            // Define as bordas do objeto e da cesta para detecção de colisão
            float obj_right = objects[i].x + objects[i].size / 2, obj_left = objects[i].x - objects[i].size / 2, obj_bottom = objects[i].y - objects[i].size * 0.5f;
            float basket_right = basket.x + basket.width / 2, basket_left = basket.x - basket.width / 2, basket_top = basket.y + basket.height / 2;
            // Verifica se o objeto caiu no chão (erro)
            if (obj_bottom < -0.8f) {
                if (++misses >= MAX_MISSES) {
                    triggerGameOver();
                }
                objects[i].respawn();
            }
            // Verifica colisão com a cesta
            else if (obj_right > basket_left && obj_left < basket_right && obj_bottom <= basket_top && objects[i].y >= basket.y) {
                if (objects[i].wasteType == basket.wasteType) {
                    score++; // Acerto: incrementa a pontuação
                } else {
                    if (++misses >= MAX_MISSES) {
                        triggerGameOver(); // Erro: coleta errada
                    }
                }
                objects[i].respawn();
                updateWindowTitle();
                // Aumenta a dificuldade se a pontuação atingir o próximo limiar
                if (score >= scoreForNextDifficultyIncrease) {
                    if (currentMinObjectSpeed < 0.015f) currentMinObjectSpeed += SPEED_INCREASE_AMOUNT;
                    if (currentMaxObjectSpeedOffset < 0.01f) currentMaxObjectSpeedOffset += SPEED_INCREASE_AMOUNT;
                    if (objects.size() < MAX_NUM_OBJECTS) objects.push_back(FallingObject()); // Adiciona mais um objeto
                    scoreForNextDifficultyIncrease += SCORE_INCREMENT_FOR_DIFFICULTY; // Define o próximo limiar
                }
            }
        }
    } else if (gameState == STATE_GAMEOVER) {
        // Lógica da Animação de Game Over
        if (gameOverAnimationTimer < GAMEOVER_ANIMATION_DURATION) {
            gameOverAnimationTimer += dt;
        }
        // Animação de contagem da pontuação
        if (displayedScore < finalScoreHolder) {
            int increment = std::max(1, (finalScoreHolder - displayedScore) / 15);
            displayedScore = std::min(finalScoreHolder, displayedScore + increment);
        }
        // Animação da cesta tombando e caindo
        if (gameOverBasketAngle > -90.0f) {
            gameOverBasketAngle -= 2.0f;
            if (gameOverBasketAngle < -90.0f) {
                gameOverBasketAngle = -90.0f;
            }
        }
        float finalY = -0.8f + basket.width / 2.0f;
        gameOverBasketY = basket.y + (finalY - basket.y) * fabs(gameOverBasketAngle / -90.0f);
        // Gera novas partículas para a chuva de lixo
        if (glutGet(GLUT_ELAPSED_TIME) % 2 == 0 && trashRain.size() < 1200) {
            TrashParticle p;
            p.x = (static_cast<float>(rand()) / RAND_MAX) * 4.0f - 2.0f;
            p.y = 1.2f + (static_cast<float>(rand()) / RAND_MAX) * 0.5f;
            p.vx = ((static_cast<float>(rand()) / RAND_MAX) - 0.5f) * 0.5f;
            p.vy = 0.0f;
            p.size = 0.06f + (static_cast<float>(rand()) / RAND_MAX) * 0.05f;
            p.largura = p.size; p.altura = p.size;
            p.rotation = static_cast<float>(rand() % 360);
            p.rotationSpeed = ((static_cast<float>(rand()) / RAND_MAX) - 0.5f) * 4.0f;
            p.type = static_cast<WASTE_TYPE>(rand() % WASTE_TYPE_COUNT);
            p.estatico = false;
            trashRain.push_back(p);
        }
        // Simulação física simples para as partículas (gravidade, colisão)
        for (size_t i = 0; i < trashRain.size(); ++i) {
            TrashParticle& p = trashRain[i];
            if (p.estatico) {
                continue; // Pula partículas que já pararam
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
            for (size_t j = 0; j < trashRain.size(); ++j) {
                if (i == j) {
                    continue;
                }
                const TrashParticle& other = trashRain[j];
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
    glutPostRedisplay();            // Marca a janela para ser redesenhada
    glutTimerFunc(16, update, 0);   // Agenda a próxima chamada desta função em ~16ms (aprox. 60 FPS)
}
// Chamada quando a janela é redimensionada
void reshape(GLsizei width, GLsizei height) {
    if (height == 0) {
        height = 1; // Previne divisão por zero
    }
    windowWidth = width;
    windowHeight = height;
    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    float aspect = (float)width / (float)height;
    // Ajusta a projeção ortográfica para manter a proporção dos objetos, evitando distorção
    if (width <= height) {
        gluOrtho2D(-1.0f, 1.0f, -1.0f / aspect, 1.0f / aspect);
    } else {
        gluOrtho2D(-1.0f * aspect, 1.0f * aspect, -1.0f, 1.0f);
    }
    basket.move(0); // Garante que a cesta permaneça nos limites após redimensionar
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}
// Chamada quando uma tecla normal (ASCII) é pressionada
void keyboard(unsigned char key, int x, int y) {
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
                exit(0); // ESC para sair
            } else if (key >= '1' && key <= '5') {
                basket.wasteType = static_cast<WASTE_TYPE>(key - '1'); // Teclas numéricas mudam a cesta
            }
            break;
        case STATE_PAUSED:
            if (key == 'p' || key == 'P') {
                gameState = STATE_PLAYING; // 'P' para despausar
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
// Chamada quando uma tecla especial (setas, F1, etc.) é pressionada
void specialKeyboard(int key, int x, int y) {
    if (gameState != STATE_PLAYING) {
        return;
    }
    // Ativa as flags de movimento para um controle mais fluido
    if (key == GLUT_KEY_LEFT) {
        key_a_pressed = true;
    }
    if (key == GLUT_KEY_RIGHT) {
        key_d_pressed = true;
    }
}
// Chamada quando uma tecla especial é solta
void specialKeyboardUp(int key, int x, int y) {
    // Desativa as flags de movimento
    if (key == GLUT_KEY_LEFT) {
        key_a_pressed = false;
    }
    if (key == GLUT_KEY_RIGHT) {
        key_d_pressed = false;
    }
}
// Converte as coordenadas do mouse (pixels) para o sistema de coordenadas do jogo (-1 a 1)
void convertMouseToGameCoords(int mouseX, int mouseY, float& gameX, float& gameY) {
    float aspect = (float)windowWidth / (float)windowHeight;
    float worldLeft, worldRight, worldBottom, worldTop;
    if (aspect >= 1.0f) {
        worldLeft = -aspect; worldRight = aspect; worldBottom = -1.0f; worldTop = 1.0f;
    } else {
        worldLeft = -1.0f; worldRight = 1.0f; worldBottom = -1.0f / aspect; worldTop = 1.0f / aspect;
    }
    gameX = worldLeft + (mouseX / (float)windowWidth) * (worldRight - worldLeft);
    gameY = worldTop - (mouseY / (float)windowHeight) * (worldTop - worldBottom);
}
// Chamada quando um botão do mouse é clicado
void mouseClick(int button, int state, int x, int y) {
    if (button != GLUT_LEFT_BUTTON || state != GLUT_DOWN) {
        return;
    }
    convertMouseToGameCoords(x, y, mouseGameX, mouseGameY);

    if (gameState == STATE_PAUSED) {
        if (pauseButtons[0].isInside(mouseGameX, mouseGameY)) gameState = STATE_PLAYING;
        if (pauseButtons[1].isInside(mouseGameX, mouseGameY)) resetGame();
        if (pauseButtons[2].isInside(mouseGameX, mouseGameY)) goToHomeScreen();
        if (pauseButtons[3].isInside(mouseGameX, mouseGameY)) exit(0);
    } else if (gameState == STATE_HOME) {
        if (homeButton.isInside(mouseGameX, mouseGameY)) {
            resetGame();
        }
    }
}
// Chamada quando o mouse se move sem estar clicado (movimento passivo)
void mousePassiveMotion(int x, int y) {
    convertMouseToGameCoords(x, y, mouseGameX, mouseGameY);
    bool needsRedraw = false;
    // Verifica se o mouse está sobre algum botão para o efeito hover
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
        glutPostRedisplay(); // Se um botão mudou de estado, redesenha a tela
    }
}