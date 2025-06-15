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

// --- Constantes Globais de Jogo ---
const int MAX_MISSES = 1; // Máximo de erros permitidos antes do game over
const std::string RANKING_FILENAME = "ranking.txt"; // Nome do arquivo para salvar o ranking
const std::string PLAYER_NAME = "Jogador1"; // Nome padrão do jogador (hardcoded)
const int MAX_RANKING_DISPLAY_ENTRIES = 5; // Quantidade de scores a serem exibidos na tela de game over

// --- Variáveis Globais para dificuldade dinâmica ---
const int MAX_NUM_OBJECTS = 4; // Máximo de objetos caindo simultaneamente
const float INITIAL_MIN_OBJECT_SPEED = 0.008f; // Velocidade mínima inicial
const float INITIAL_MAX_OBJECT_SPEED_OFFSET = 0.003f; // Componente aleatória da velocidade inicial
const float SPEED_INCREASE_AMOUNT = 0.0002f; // Incremento de velocidade a cada nível
const int SCORE_INCREMENT_FOR_DIFFICULTY = 5; // Pontuação necessária para aumentar a dificuldade

// --- Tipos, Cores e Alinhamento ---
// Enum para os tipos de lixo, facilitando a leitura do código
enum WASTE_TYPE {
  PAPER = 0, PLASTIC, METAL, GLASS, ORGANIC, WASTE_TYPE_COUNT
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
enum Alignment { ALIGN_LEFT, ALIGN_CENTER };

// --- Estruturas de Dados ---
// Estrutura para as partículas de lixo da animação de game over
struct TrashParticle {
  float x, y, vx, vy, rotation, rotationSpeed, size, largura, altura;
  bool estatico; // A partícula para de se mover?
  WASTE_TYPE type; // Tipo de lixo para definir a cor/forma
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
  WASTE_TYPE wasteType; // Tipo de lixo que a cesta aceita

  Basket();
  void draw();
  void move(float direction);
};

// Estrutura para armazenar o score de um jogador no ranking
struct PlayerScore {
  std::string name;
  int score;
};


// --- Variáveis de Estado do Jogo ---
std::vector<FallingObject> objects; // Vetor de objetos que estão caindo
std::vector<Building> cityscape; // Vetor de prédios do cenário
Basket basket; // A cesta do jogador
int score = 0; // Pontuação atual
int misses = 0; // Contagem de erros
bool gameOver = false; // Flag que indica o fim de jogo
int windowWidth = 600, windowHeight = 800; // Dimensões da janela
bool key_a_pressed = false, key_d_pressed = false; // Flags para controle de movimento contínuo

// --- Dificuldade Dinâmica ---
float currentMinObjectSpeed = INITIAL_MIN_OBJECT_SPEED;
float currentMaxObjectSpeedOffset = INITIAL_MAX_OBJECT_SPEED_OFFSET;
int scoreForNextDifficultyIncrease = 5;

// --- Animação de Game Over ---
std::vector<TrashParticle> trashRain; // Partículas para a animação de "chuva de lixo"
float gameOverAnimationTimer = 0.0f; // Timer para controlar a duração da animação
const float GAMEOVER_ANIMATION_DURATION = 1.5f; // Duração total da animação em segundos
int displayedScore = 0, finalScoreHolder = 0; // Scores para a animação de contagem
float gameOverBasketAngle = 0.0f; // Ângulo de rotação da cesta na animação
float gameOverBasketY = 0.0f; // Posição Y da cesta na animação

// --- Ranking ---
std::vector<PlayerScore> ranking; // Vetor para armazenar os scores carregados do arquivo


// --- Protótipos das Funções ---
void updateWindowTitle();
void renderBitmapText(float x, float y, void *font, const char *string);
float getStrokeTextWidth(const char *string);
void renderStrokeText(float x, float y, float scale, float lineWidth, const char *string, Alignment align);
void drawRainObject(const TrashParticle& p);
void loadRanking();
void saveRanking(int finalScore);
void initUrbanScenery();
void drawUrbanScenery();
void resetGame();
void triggerGameOver();
void display();
void update(int value);
void reshape(GLsizei w, GLsizei h);
void keyboard(unsigned char key, int x, int y);
void keyboardUp(unsigned char key, int x, int y);
void specialKeyboard(int key, int x, int y);
void specialKeyboardUp(int key, int x, int y);

// --- Função Principal ---
int main(int argc, char **argv) {
  // Inicializa o gerador de números aleatórios com o tempo atual
  srand(static_cast<unsigned int>(time(0)));

  loadRanking(); // Carrega o ranking do arquivo ao iniciar
  objects.push_back(FallingObject()); // Adiciona o primeiro objeto caindo

  // Inicializa o GLUT
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_ALPHA | GLUT_MULTISAMPLE);
  glutInitWindowSize(windowWidth, windowHeight);
  glutInitWindowPosition(50, 50);
  glutCreateWindow("Coleta Seletiva"); // Título da janela (será atualizado com a pontuação)

  // Habilita configurações do OpenGL para transparência e antialiasing
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_LINE_SMOOTH);
  glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);

  // Inicializações do jogo
  updateWindowTitle();
  initUrbanScenery();

  // Registra as funções de callback do GLUT
  glutDisplayFunc(display);
  glutReshapeFunc(reshape);
  glutKeyboardFunc(keyboard);
  glutKeyboardUpFunc(keyboardUp);
  glutSpecialFunc(specialKeyboard);
  glutSpecialUpFunc(specialKeyboardUp);
  glutTimerFunc(0, update, 0); // Inicia o loop de atualização do jogo

  glutMainLoop(); // Entra no loop principal de eventos do GLUT
  return 0;
}

// --- Construtores das Estruturas ---
// Construtor do objeto que cai: define um tamanho padrão e o inicializa
FallingObject::FallingObject() {
  size = 0.12f; // Tamanho do modelo visual do objeto
  respawn();    // Chama respawn para definir posição inicial, velocidade e tipo
}

// Construtor da cesta: define suas dimensões e posição inicial
Basket::Basket() {
  width = 0.3f;
  height = 0.2f;
  x = 0.0f;                  // Começa no centro da tela
  y = -0.8f + height / 2.0f; // Posição Y fixa na parte inferior
  wasteType = PAPER;         // Tipo inicial da cesta é Papel
  speed = 0.05f;             // Velocidade de movimento horizontal
}

// --- Funções de Desenho e Update de Objetos ---
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
    case PAPER: glColor3f(0.9f, 0.9f, 0.85f); glBegin(GL_QUADS); glVertex2f(-0.5f, -0.2f); glVertex2f(0.5f, -0.2f); glVertex2f(0.5f, 0.2f); glVertex2f(-0.5f, 0.2f); glEnd(); glColor3f(0.4f, 0.4f, 0.4f); glBegin(GL_LINES); for (int i = 0; i < 4; ++i) { float lineY = -0.15f + i * 0.1f; glVertex2f(-0.4f, lineY); glVertex2f(0.4f, lineY); } glEnd(); break;
    case PLASTIC: glBegin(GL_QUADS); glColor3f(0.8f, 0.2f, 0.2f); glVertex2f(-0.25f, -0.5f); glVertex2f(0.25f, -0.5f); glColor3f(1.0f, 0.5f, 0.5f); glVertex2f(0.25f, 0.2f); glVertex2f(-0.25f, 0.2f); glEnd(); glBegin(GL_QUADS); glVertex2f(-0.15f, 0.2f); glVertex2f(0.15f, 0.2f); glVertex2f(0.15f, 0.4f); glVertex2f(-0.15f, 0.4f); glEnd(); glColor3f(0.6f, 0.1f, 0.1f); glBegin(GL_QUADS); glVertex2f(-0.17f, 0.4f); glVertex2f(0.17f, 0.4f); glVertex2f(0.17f, 0.5f); glVertex2f(-0.17f, 0.5f); glEnd(); glColor4f(1.0f, 1.0f, 1.0f, 0.5f); glBegin(GL_QUADS); glVertex2f(-0.2f, -0.3f); glVertex2f(-0.1f, -0.3f); glVertex2f(-0.1f, 0.1f); glVertex2f(-0.2f, 0.1f); glEnd(); break;
    case METAL: glBegin(GL_QUAD_STRIP); glColor3f(0.6f, 0.6f, 0.65f); glVertex2f(-0.3f, -0.5f); glVertex2f(-0.3f, 0.5f); glColor3f(0.9f, 0.9f, 0.95f); glVertex2f(-0.1f, -0.5f); glVertex2f(-0.1f, 0.5f); glColor3f(0.9f, 0.9f, 0.95f); glVertex2f(0.1f, -0.5f); glVertex2f(0.1f, 0.5f); glColor3f(0.6f, 0.6f, 0.65f); glVertex2f(0.3f, -0.5f); glVertex2f(0.3f, 0.5f); glEnd(); glColor3f(0.5f, 0.5f, 0.55f); glBegin(GL_QUADS); glVertex2f(-0.3f, 0.5f); glVertex2f(0.3f, 0.5f); glVertex2f(0.3f, 0.4f); glVertex2f(-0.3f, 0.4f); glVertex2f(-0.3f, -0.5f); glVertex2f(0.3f, -0.5f); glVertex2f(0.3f, -0.4f); glVertex2f(-0.3f, -0.4f); glEnd(); break;
    case GLASS: glColor4f(0.2f, 0.7f, 0.2f, 0.7f); glBegin(GL_QUADS); glVertex2f(-0.25f, -0.5f); glVertex2f(0.25f, -0.5f); glVertex2f(0.25f, 0.1f); glVertex2f(-0.25f, 0.1f); glEnd(); glBegin(GL_TRIANGLES); glVertex2f(-0.25f, 0.1f); glVertex2f(0.25f, 0.1f); glVertex2f(0.15f, 0.3f); glVertex2f(-0.25f, 0.1f); glVertex2f(-0.15f, 0.3f); glVertex2f(0.15f, 0.3f); glEnd(); glBegin(GL_QUADS); glVertex2f(-0.1f, 0.3f); glVertex2f(0.1f, 0.3f); glVertex2f(0.1f, 0.5f); glVertex2f(-0.1f, 0.5f); glEnd(); glColor4f(1.0f, 1.0f, 1.0f, 0.6f); glBegin(GL_QUADS); glVertex2f(0.1f, -0.4f); glVertex2f(0.18f, -0.4f); glVertex2f(0.18f, 0.2f); glVertex2f(0.1f, 0.2f); glEnd(); break;
    case ORGANIC: glBegin(GL_POLYGON); glColor3f(0.9f, 0.1f, 0.1f); for (int i = 0; i < 20; i++) { float ang = 2.0f * M_PI * i / 20.0f; glVertex2f(cos(ang) * 0.4f, sin(ang) * 0.5f); } glEnd(); glColor3f(0.4f, 0.2f, 0.0f); glBegin(GL_QUADS); glVertex2f(-0.05f, 0.4f); glVertex2f(0.05f, 0.4f); glVertex2f(0.05f, 0.6f); glVertex2f(-0.05f, 0.6f); glEnd(); glColor3f(0.1f, 0.8f, 0.1f); glBegin(GL_TRIANGLES); glVertex2f(0.05f, 0.5f); glVertex2f(0.3f, 0.7f); glVertex2f(0.1f, 0.4f); glEnd(); break;
  }
  glPopMatrix();
}

// Desenha a cesta na sua posição atual
void Basket::draw() {
  const float* color = COLOR_TABLE[wasteType]; // Pega a cor correspondente ao tipo de lixo atual
  // Corpo da cesta com gradiente
  glBegin(GL_QUADS); glColor3f(color[0] * 0.7f, color[1] * 0.7f, color[2] * 0.7f); glVertex2f(-width/2, -height/2); glVertex2f(width/2, -height/2); glColor3f(color[0], color[1], color[2]); glVertex2f(width/2, height/2); glVertex2f(-width/2, height/2); glEnd();
  // Borda superior da cesta
  glColor3f(color[0] * 0.5f, color[1] * 0.5f, color[2] * 0.5f); glBegin(GL_QUADS); glVertex2f(-width/2-0.02f, height/2); glVertex2f(width/2+0.02f, height/2); glVertex2f(width/2+0.02f, height/2+0.03f); glVertex2f(-width/2-0.02f, height/2+0.03f); glEnd();
  // Símbolo de reciclagem (desenhado no centro da cesta, independente da posição dela)
  glPushMatrix();
  glColor3f(1.0f, 1.0f, 1.0f); float s = 0.05f; for(int i=0; i<3; ++i) { glRotatef(120.0, 0, 0, 1); glBegin(GL_QUADS); glVertex2f(-s, s); glVertex2f(s, s); glVertex2f(s*1.5, s*1.8); glVertex2f(-s*0.5, s*1.8); glEnd(); }
  glPopMatrix();
}

// Move a cesta horizontalmente
void Basket::move(float direction) { // direction é -1 (esquerda) ou 1 (direita)
  x += direction * speed;
  // Limita o movimento da cesta às bordas da área de jogo (-1.0 a 1.0) para que não saia da tela
  if (x - width / 2 < -1.0f) {
    x = -1.0f + width / 2;
  }
  if (x + width / 2 > 1.0f) {
    x = 1.0f - width / 2;
  }
}

// --- Funções Auxiliares de Texto e Jogo ---
// Atualiza o título da janela para mostrar a pontuação atual
void updateWindowTitle() {
  char title[100];
  sprintf(title, "Coleta Seletiva - Pontos: %d", score);
  glutSetWindowTitle(title);
}

// Renderiza texto na tela usando fontes de bitmap (mais rápidas)
void renderBitmapText(float x, float y, void *font, const char *string) {
  glRasterPos2f(x, y);
  while (*string) glutBitmapCharacter(font, *string++);
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
  glLineWidth(lineWidth); // Define a espessura da linha
  while (*string) {
    glutStrokeCharacter(GLUT_STROKE_ROMAN, *string++); // Desenha cada caractere
  }
  glPopMatrix();
}

// Desenha uma partícula de lixo para a animação de game over (cópia da lógica de FallingObject::draw)
void drawRainObject(const TrashParticle& p) {
    glPushMatrix();
    glTranslatef(p.x, p.y, 0.0f);
    glRotatef(p.rotation, 0.0f, 0.0f, 1.0f);
    glScalef(p.size, p.size, 1.0f);
    switch (p.type) {
      case PAPER: glColor3f(0.9f, 0.9f, 0.85f); glBegin(GL_QUADS); glVertex2f(-0.5f, -0.2f); glVertex2f(0.5f, -0.2f); glVertex2f(0.5f, 0.2f); glVertex2f(-0.5f, 0.2f); glEnd(); glColor3f(0.4f, 0.4f, 0.4f); glBegin(GL_LINES); for (int i = 0; i < 4; ++i) { float lineY = -0.15f + i * 0.1f; glVertex2f(-0.4f, lineY); glVertex2f(0.4f, lineY); } glEnd(); break;
      case PLASTIC: glBegin(GL_QUADS); glColor3f(0.8f, 0.2f, 0.2f); glVertex2f(-0.25f, -0.5f); glVertex2f(0.25f, -0.5f); glColor3f(1.0f, 0.5f, 0.5f); glVertex2f(0.25f, 0.2f); glVertex2f(-0.25f, 0.2f); glEnd(); glBegin(GL_QUADS); glVertex2f(-0.15f, 0.2f); glVertex2f(0.15f, 0.2f); glVertex2f(0.15f, 0.4f); glVertex2f(-0.15f, 0.4f); glEnd(); glColor3f(0.6f, 0.1f, 0.1f); glBegin(GL_QUADS); glVertex2f(-0.17f, 0.4f); glVertex2f(0.17f, 0.4f); glVertex2f(0.17f, 0.5f); glVertex2f(-0.17f, 0.5f); glEnd(); glColor4f(1.0f, 1.0f, 1.0f, 0.5f); glBegin(GL_QUADS); glVertex2f(-0.2f, -0.3f); glVertex2f(-0.1f, -0.3f); glVertex2f(-0.1f, 0.1f); glVertex2f(-0.2f, 0.1f); glEnd(); break;
      case METAL: glBegin(GL_QUAD_STRIP); glColor3f(0.6f, 0.6f, 0.65f); glVertex2f(-0.3f, -0.5f); glVertex2f(-0.3f, 0.5f); glColor3f(0.9f, 0.9f, 0.95f); glVertex2f(-0.1f, -0.5f); glVertex2f(-0.1f, 0.5f); glColor3f(0.9f, 0.9f, 0.95f); glVertex2f(0.1f, -0.5f); glVertex2f(0.1f, 0.5f); glColor3f(0.6f, 0.6f, 0.65f); glVertex2f(0.3f, -0.5f); glVertex2f(0.3f, 0.5f); glEnd(); glColor3f(0.5f, 0.5f, 0.55f); glBegin(GL_QUADS); glVertex2f(-0.3f, 0.5f); glVertex2f(0.3f, 0.5f); glVertex2f(0.3f, 0.4f); glVertex2f(-0.3f, 0.4f); glVertex2f(-0.3f, -0.5f); glVertex2f(0.3f, -0.5f); glVertex2f(0.3f, -0.4f); glVertex2f(-0.3f, -0.4f); glEnd(); break;
      case GLASS: glColor4f(0.2f, 0.7f, 0.2f, 0.7f); glBegin(GL_QUADS); glVertex2f(-0.25f, -0.5f); glVertex2f(0.25f, -0.5f); glVertex2f(0.25f, 0.1f); glVertex2f(-0.25f, 0.1f); glEnd(); glBegin(GL_TRIANGLES); glVertex2f(-0.25f, 0.1f); glVertex2f(0.25f, 0.1f); glVertex2f(0.15f, 0.3f); glVertex2f(-0.25f, 0.1f); glVertex2f(-0.15f, 0.3f); glVertex2f(0.15f, 0.3f); glEnd(); glBegin(GL_QUADS); glVertex2f(-0.1f, 0.3f); glVertex2f(0.1f, 0.3f); glVertex2f(0.1f, 0.5f); glVertex2f(-0.1f, 0.5f); glEnd(); glColor4f(1.0f, 1.0f, 1.0f, 0.6f); glBegin(GL_QUADS); glVertex2f(0.1f, -0.4f); glVertex2f(0.18f, -0.4f); glVertex2f(0.18f, 0.2f); glVertex2f(0.1f, 0.2f); glEnd(); break;
      case ORGANIC: glBegin(GL_POLYGON); glColor3f(0.9f, 0.1f, 0.1f); for (int i = 0; i < 20; i++) { float ang = 2.0f * M_PI * i / 20.0f; glVertex2f(cos(ang) * 0.4f, sin(ang) * 0.5f); } glEnd(); glColor3f(0.4f, 0.2f, 0.0f); glBegin(GL_QUADS); glVertex2f(-0.05f, 0.4f); glVertex2f(0.05f, 0.4f); glVertex2f(0.05f, 0.6f); glVertex2f(-0.05f, 0.6f); glEnd(); glColor3f(0.1f, 0.8f, 0.1f); glBegin(GL_TRIANGLES); glVertex2f(0.05f, 0.5f); glVertex2f(0.3f, 0.7f); glVertex2f(0.1f, 0.4f); glEnd(); break;
    }
    glPopMatrix();
}

// Carrega o ranking do arquivo de texto
void loadRanking() {
  std::ifstream inFile(RANKING_FILENAME);
  if (!inFile.is_open()) return; // Se o arquivo não existe, simplesmente retorna
  ranking.clear();
  std::string line;
  while (std::getline(inFile, line)) {
    std::stringstream ss(line);
    std::string name;
    int score_val;
    // Assume o formato "Nome Score" e faz o parse
    if (ss >> name >> score_val) ranking.push_back({name, score_val});
  }
  inFile.close();
  // Ordena o ranking em ordem decrescente de pontuação
  std::sort(ranking.begin(), ranking.end(), [](const PlayerScore& a, const PlayerScore& b) {
    return a.score > b.score;
  });
}

// Adiciona a pontuação final ao ranking e salva no arquivo
void saveRanking(int finalScore) {
  loadRanking(); // Recarrega para garantir que está atualizado
  ranking.push_back({PLAYER_NAME, finalScore}); // Adiciona o novo score
  // Reordena
  std::sort(ranking.begin(), ranking.end(), [](const PlayerScore& a, const PlayerScore& b) {
    return a.score > b.score;
  });

  std::ofstream outFile(RANKING_FILENAME);
  if (!outFile.is_open()) {
    fprintf(stderr, "Erro ao salvar ranking.\n");
    return;
  }
  // Escreve cada entrada do ranking no arquivo
  for (const auto& ps : ranking) {
    outFile << ps.name << " " << ps.score << std::endl;
  }
  outFile.close();
}


// --- Funções de Cenário ---
// Inicializa o cenário urbano com prédios gerados proceduralmente
void initUrbanScenery() {
  cityscape.clear();
  srand(1337); // Usa uma seed fixa para que o cenário seja sempre o mesmo
  float worldWidth = 4.0f;
  // Gera a primeira camada de prédios (mais escuros, ao fundo)
  float current_x = -worldWidth;
  while(current_x < worldWidth) { Building b; b.x_pos = current_x; b.width = 0.2f + (static_cast<float>(rand()) / RAND_MAX) * 0.3f; b.height = 0.2f + (static_cast<float>(rand()) / RAND_MAX) * 0.6f; float gray = 0.15f + (static_cast<float>(rand()) / RAND_MAX) * 0.1f; b.r = gray; b.g = gray; b.b = gray + 0.05f; b.layer = 0; cityscape.push_back(b); current_x += b.width + 0.05f; }
  // Gera a segunda camada de prédios (mais claros, à frente)
  current_x = -worldWidth;
  while(current_x < worldWidth) { Building b; b.x_pos = current_x; b.width = 0.3f + (static_cast<float>(rand()) / RAND_MAX) * 0.2f; b.height = -0.2f + (static_cast<float>(rand()) / RAND_MAX) * 0.3f; float gray = 0.25f + (static_cast<float>(rand()) / RAND_MAX) * 0.1f; b.r = gray; b.g = gray; b.b = gray; b.layer = 1; cityscape.push_back(b); current_x += b.width + 0.1f; }
  // Ordena os prédios por camada para desenhar corretamente
  std::sort(cityscape.begin(), cityscape.end(), [](const Building& a, const Building& b){ return a.layer < b.layer; });
  srand(static_cast<unsigned int>(time(0))); // Reseta a seed para o resto do jogo
}

// Desenha o cenário urbano
void drawUrbanScenery() {
  float aspect = (float)windowWidth / (float)windowHeight;
  float worldLeft = -1.0, worldRight = 1.0, worldTop = 1.0, worldBottom = -1.0;
  if (aspect > 1.0) { worldLeft = -aspect; worldRight = aspect; } else { worldTop = 1.0 / aspect; worldBottom = -1.0 / aspect; }
  // Desenha o céu com um gradiente
  glBegin(GL_QUADS); glColor3f(0.1f, 0.1f, 0.3f); glVertex2f(worldLeft, worldTop); glVertex2f(worldRight, worldTop); glColor3f(0.9f, 0.7f, 0.4f); glVertex2f(worldRight, -0.7f); glVertex2f(worldLeft, -0.7f); glEnd();
  int time_ms = glutGet(GLUT_ELAPSED_TIME);
  // Desenha cada prédio
  for(const auto& b : cityscape) {
    glColor3f(b.r, b.g, b.b); glBegin(GL_QUADS); glVertex2f(b.x_pos, -0.8f); glVertex2f(b.x_pos + b.width, -0.8f); glVertex2f(b.x_pos + b.width, b.height); glVertex2f(b.x_pos, b.height); glEnd();
    // Desenha as janelas, algumas acesas e outras apagadas de forma "aleatória" baseada no tempo
    float window_margin = 0.1f * b.width; float window_size = 0.08f * b.width; int num_floors = static_cast<int>((b.height + 0.8f) / 0.1f); int num_windows_per_floor = static_cast<int>((b.width - 2*window_margin) / (window_size*1.5f));
    for(int i = 0; i < num_floors; ++i) { for(int j = 0; j < num_windows_per_floor; ++j) { int window_seed = static_cast<int>(b.x_pos * 100) + i * 13 + j * 7; if(sin( (time_ms / 1000.0f) * 0.2f + window_seed ) > 0.8) { glColor3f(0.9f, 0.9f, 0.6f); } else { glColor3f(b.r*0.5f, b.g*0.5f, b.b*0.5f); } float wx = b.x_pos + window_margin + j * (window_size * 1.5f); float wy = -0.75f + i * 0.1f; glBegin(GL_QUADS); glVertex2f(wx, wy); glVertex2f(wx + window_size, wy); glVertex2f(wx + window_size, wy + 0.05f); glVertex2f(wx, wy + 0.05f); glEnd(); } }
  }
  // Desenha a rua/chão
  glColor3f(0.2f, 0.2f, 0.2f); glBegin(GL_QUADS); glVertex2f(worldLeft, -1.0f); glVertex2f(worldRight, -1.0f); glVertex2f(worldRight, -0.8f); glVertex2f(worldLeft, -0.8f); glEnd();
}


// --- Funções Principais de Jogo ---
// Ativa o estado de "Game Over"
void triggerGameOver() {
  if (!gameOver) {
    gameOver = true;
    finalScoreHolder = score; // Guarda a pontuação final para a animação
    gameOverBasketY = basket.y; // Guarda a posição inicial da cesta
    saveRanking(score); // Salva a pontuação no ranking
    loadRanking(); // Recarrega para exibir o ranking atualizado
  }
}

// Reseta o estado do jogo para começar uma nova partida
void resetGame() {
  score = 0;
  misses = 0;
  gameOver = false;
  gameOverAnimationTimer = 0.0f;
  displayedScore = 0;
  finalScoreHolder = 0;
  gameOverBasketAngle = 0.0f;
  trashRain.clear();
  // Restaura a dificuldade inicial
  currentMinObjectSpeed = INITIAL_MIN_OBJECT_SPEED;
  currentMaxObjectSpeedOffset = INITIAL_MAX_OBJECT_SPEED_OFFSET;
  scoreForNextDifficultyIncrease = 5;
  objects.clear();
  objects.push_back(FallingObject()); // Adiciona o primeiro objeto
  basket.x = 0.0f; // Reposiciona a cesta
  basket.wasteType = PAPER;
  updateWindowTitle();
}


// --- Funções de Callback do GLUT ---
// Função de desenho principal, chamada a cada frame
void display() {
  glClear(GL_COLOR_BUFFER_BIT); // Limpa a tela
  drawUrbanScenery(); // Desenha o fundo

  // Se o jogo acabou, renderiza a tela de Game Over
  if (gameOver) {
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
    glBegin(GL_QUADS); glVertex2f(-2.f, -2.f); glVertex2f(2.f, -2.f); glVertex2f(2.f, 2.f); glVertex2f(-2.f, 2.f); glEnd();

    // Desenha o texto de Game Over, que também aparece gradualmente
    if (animProgress > 0.7f) {
      const float startY = 0.6f;
      const float titleScale = 0.001f, scoreScale = 0.0005f, instructionScale = 0.0003f;
      const float rankingTitleScale = 0.0004f, rankingEntryScale = 0.00035f;
      const float lineSpacingTitle = 0.35f, lineSpacingScore = 0.25f, lineSpacingInstructions = 0.12f;
      const float lineSpacingRankingTitle = 0.2f, lineSpacingRankingEntry = 0.08f;
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
      renderStrokeText(0.0f, currentY, instructionScale, 1.0f, "'ESC' para Sair", ALIGN_CENTER);
      currentY -= lineSpacingRankingTitle;

      // Ranking
      glColor4f(1.0f, 1.0f, 0.0f, textAlpha);
      renderStrokeText(0.0f, currentY, rankingTitleScale, 2.0f, "Ranking", ALIGN_CENTER);
      currentY -= lineSpacingRankingTitle * 0.7;

      // Calcula a largura máxima para alinhar o ranking à esquerda
      float maxRankWidth = 0.0f;
      for (int i = 0; i < ranking.size() && i < MAX_RANKING_DISPLAY_ENTRIES; ++i) {
        sprintf(buffer, "%d. %s: %d", i + 1, ranking[i].name.c_str(), ranking[i].score);
        float currentWidth = getStrokeTextWidth(buffer);
        if (currentWidth > maxRankWidth) maxRankWidth = currentWidth;
      }
      float rankStartX = 0.0f - (maxRankWidth / 2.0f * rankingEntryScale);

      bool highlighted = false; // Para destacar a pontuação do jogador atual
      for (int i = 0; i < ranking.size() && i < MAX_RANKING_DISPLAY_ENTRIES; ++i) {
        sprintf(buffer, "%d. %s: %d", i + 1, ranking[i].name.c_str(), ranking[i].score);
        if (!highlighted && ranking[i].name == PLAYER_NAME && ranking[i].score == finalScoreHolder) {
          glColor4f(1.0f, 1.0f, 0.0f, textAlpha); highlighted = true; // Destaque em amarelo
        } else { glColor4f(1.0f, 1.0f, 1.0f, textAlpha); }
        renderStrokeText(rankStartX, currentY, rankingEntryScale, 1.0f, buffer, ALIGN_LEFT);
        currentY -= lineSpacingRankingEntry;
      }
    }
  } else { // Se o jogo está em andamento
    // Desenha um "blur" no cenário para destacar os objetos de jogo
    float aspect = (float)windowWidth / (float)windowHeight;
    float worldLeft, worldRight, worldBottom, worldTop;
    if (aspect > 1.0) { worldLeft = -aspect; worldRight = aspect; worldBottom = -1.0; worldTop = 1.0; } 
    else { worldLeft = -1.0; worldRight = 1.0; worldBottom = -1.0/aspect; worldTop = 1.0/aspect; }
    glColor4f(0.1f, 0.1f, 0.1f, 0.45f);
    glBegin(GL_QUADS);
    glVertex2f(worldLeft, worldBottom);
    glVertex2f(worldRight, worldBottom);
    glVertex2f(worldRight, worldTop);
    glVertex2f(worldLeft, worldTop);
    glEnd();

    // Desenha os objetos caindo
    for (size_t i = 0; i < objects.size(); ++i) objects[i].draw();

    // Desenha a cesta
    glPushMatrix();
    glTranslatef(basket.x, basket.y, 0.0f);
    basket.draw();
    glPopMatrix();

    // Desenha a interface (HUD) com informações
    char scoreText[50]; sprintf(scoreText, "Pontos: %d", score);
    glColor3f(1.0f, 1.0f, 1.0f);
    renderBitmapText(-0.95f, 0.9f, GLUT_BITMAP_HELVETICA_18, scoreText);
    char missesText[50]; sprintf(missesText, "Erros: %d/%d", misses, MAX_MISSES);
    glColor3f(1.0f, 0.5f, 0.5f);
    renderBitmapText(-0.95f, 0.85f, GLUT_BITMAP_HELVETICA_18, missesText);
    const char* wasteTypeText;
    switch (basket.wasteType) {
      case PAPER: wasteTypeText = "Papel"; break;
      case PLASTIC: wasteTypeText = "Plastico"; break;
      case METAL: wasteTypeText = "Metal"; break;
      case GLASS: wasteTypeText = "Vidro"; break;
      case ORGANIC: wasteTypeText = "Organico"; break;
      default: wasteTypeText = "Desconhecido"; break;
    }
    glColor3f(0.8f, 0.8f, 0.8f);
    renderBitmapText(-0.95f, 0.80f, GLUT_BITMAP_HELVETICA_18, "Lixeira: ");
    const GLfloat* textColor = COLOR_TABLE[basket.wasteType];
    glColor3f(textColor[0], textColor[1], textColor[2]);
    renderBitmapText(-0.75f, 0.80f, GLUT_BITMAP_HELVETICA_18, wasteTypeText);
  }
  glutSwapBuffers(); // Troca os buffers para exibir o que foi desenhado
}

// Função de atualização da lógica do jogo, chamada a cada ~16ms
void update(int value) {
  float dt = 16.0f / 1000.0f; // Delta time

  if (gameOver) {
    // ---- LÓGICA DA ANIMAÇÃO DE GAME OVER ----
    // Avança o timer da animação
    if (gameOverAnimationTimer < GAMEOVER_ANIMATION_DURATION) gameOverAnimationTimer += dt;
    // Animação de contagem da pontuação
    if (displayedScore < finalScoreHolder) {
      int increment = std::max(1, (finalScoreHolder - displayedScore) / 15);
      displayedScore = std::min(finalScoreHolder, displayedScore + increment);
    }
    // Animação da cesta tombando
    float targetAngle = -90.0f;
    if (gameOverBasketAngle > targetAngle) {
      gameOverBasketAngle -= 2.0f;
      if(gameOverBasketAngle < targetAngle) gameOverBasketAngle = targetAngle;
    }
    // Animação da cesta caindo
    float initialY = basket.y;
    float finalY = -0.8f + basket.width / 2.0f;
    float animProgress = fabs(gameOverBasketAngle / targetAngle);
    gameOverBasketY = initialY + (finalY - initialY) * animProgress;
    
    // Gera novas partículas para a chuva de lixo
    int spawn_rate = 2;
    if (glutGet(GLUT_ELAPSED_TIME) % spawn_rate == 0 && trashRain.size() < 1200) {
      TrashParticle p;
      p.x = (static_cast<float>(rand())/RAND_MAX) * 4.0f - 2.0f;
      p.y = 1.2f + (static_cast<float>(rand())/RAND_MAX) * 0.5f;
      p.vx = ((static_cast<float>(rand())/RAND_MAX) - 0.5f) * 0.5f;
      p.vy = 0.0f;
      p.size = 0.06f + (static_cast<float>(rand())/RAND_MAX) * 0.05f;
      p.largura = p.size; p.altura = p.size;
      p.rotation = static_cast<float>(rand() % 360);
      p.rotationSpeed = ((static_cast<float>(rand())/RAND_MAX) - 0.5f) * 4.0f;
      p.type = static_cast<WASTE_TYPE>(rand()%WASTE_TYPE_COUNT);
      p.estatico = false;
      trashRain.push_back(p);
    }
    // Simulação física simples para as partículas (gravidade, colisão)
    float gravidade = 0.0008f;
    float chaoY = -0.8f;
    float friccao = 0.85f;
    float deslize = 0.05f;
    for (int i = 0; i < trashRain.size(); ++i) {
      TrashParticle& p = trashRain[i];
      if (p.estatico) continue; // Pula partículas que já pararam
      p.vy -= gravidade; p.x += p.vx * dt; p.y += p.vy; p.rotation += p.rotationSpeed;
      bool emRepouso = false;
      if (p.y - p.altura / 2.0f <= chaoY) { p.y = chaoY + p.altura / 2.0f; emRepouso = true; }
      // Detecção de colisão entre partículas (simplificada)
      for (int j = 0; j < trashRain.size(); ++j) {
        if (i == j) continue;
        const TrashParticle& other = trashRain[j];
        float dist_x = p.x - other.x; float dist_y = p.y - other.y;
        float dist_total = sqrt(dist_x * dist_x + dist_y * dist_y);
        float raios_soma = (p.largura + other.largura) / 2.0f;
        if (dist_total < raios_soma) {
          float overlap = raios_soma - dist_total;
          p.x += (overlap * (p.x - other.x)) / dist_total;
          p.y += (overlap * (p.y - other.y)) / dist_total;
          p.vx += (dist_x > 0 ? 1 : -1) * deslize * dt;
          if(p.y > other.y) { emRepouso = true; }
        }
      }
      if(emRepouso) {
        p.vx *= friccao; p.vy = 0;
        if (fabs(p.vx) < 0.001f) { p.estatico = true; p.vx = 0; p.rotationSpeed = 0; }
      }
    }
  } else {
    // ---- LÓGICA DO JOGO EM ANDAMENTO ----
    // Movimento contínuo da cesta se as teclas estiverem pressionadas
    if (key_a_pressed && !key_d_pressed) basket.move(-1.0f);
    else if (key_d_pressed && !key_a_pressed) basket.move(1.0f);

    for (size_t i = 0; i < objects.size(); ++i) {
      objects[i].update();

      // Define as bordas do objeto e da cesta para detecção de colisão
      float obj_right = objects[i].x + objects[i].size/2;
      float obj_left = objects[i].x - objects[i].size/2;
      float obj_bottom = objects[i].y - objects[i].size * 0.5f;
      
      float basket_right = basket.x + basket.width/2;
      float basket_left = basket.x - basket.width/2;
      float basket_top = basket.y + basket.height/2;

      // Verifica se o objeto caiu no chão (erro)
      if (obj_bottom < -0.8f) {
        misses++;
        objects[i].respawn();
        if (misses >= MAX_MISSES) triggerGameOver();
      } 
      // Verifica colisão com a cesta
      else if (obj_right > basket_left && obj_left < basket_right && obj_bottom <= basket_top && objects[i].y >= basket.y) {
        if (objects[i].wasteType == basket.wasteType) {
          score++; // Acerto: incrementa a pontuação
        } else {
          if (++misses >= MAX_MISSES) triggerGameOver(); // Erro: coleta errada
        }
        
        objects[i].respawn();
        updateWindowTitle();

        // Aumenta a dificuldade se a pontuação atingir o próximo limiar
        if (score >= scoreForNextDifficultyIncrease) {
          const float MAX_MIN_SPEED = 0.015f;
          const float MAX_OFFSET_SPEED = 0.01f;
          if(currentMinObjectSpeed < MAX_MIN_SPEED) currentMinObjectSpeed += SPEED_INCREASE_AMOUNT;
          if(currentMaxObjectSpeedOffset < MAX_OFFSET_SPEED) currentMaxObjectSpeedOffset += SPEED_INCREASE_AMOUNT;
          if(objects.size() < MAX_NUM_OBJECTS) objects.push_back(FallingObject()); // Adiciona mais um objeto
          scoreForNextDifficultyIncrease += SCORE_INCREMENT_FOR_DIFFICULTY; // Define o próximo limiar
        }
      }
    }
  }
  glutPostRedisplay(); // Marca a janela para ser redesenhada
  glutTimerFunc(16, update, 0); // Agenda a próxima chamada desta função
}

// Chamada quando a janela é redimensionada
void reshape(GLsizei width, GLsizei height) {
  if (height == 0) height = 1; // Previne divisão por zero
  windowWidth = width; windowHeight = height;
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
  
  // Garante que a cesta permaneça dentro dos limites da tela após o redimensionamento
  basket.move(0);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
}

// Chamada quando uma tecla normal é pressionada
void keyboard(unsigned char key, int x, int y) {
  if (gameOver) {
    // Se o jogo acabou, 'R' reinicia e 'ESC' sai
    if (key == 'r' || key == 'R') resetGame();
    else if (key == 27) exit(0);
    return;
  }

  // Durante o jogo, as teclas numéricas mudam o tipo da cesta
  switch (key) {
    case 27: exit(0); break; // ESC para sair
    case '1': basket.wasteType = PAPER; break;
    case '2': basket.wasteType = PLASTIC; break;
    case '3': basket.wasteType = METAL; break;
    case '4': basket.wasteType = GLASS; break;
    case '5': basket.wasteType = ORGANIC; break;
  }
  glutPostRedisplay(); // Redesenha para mostrar a nova cor da cesta
}

// Chamada quando uma tecla normal é solta
void keyboardUp(unsigned char key, int x, int y) {
    // Nenhuma ação necessária aqui
}

// Chamada quando uma tecla especial (setas, F1, etc.) é pressionada
void specialKeyboard(int key, int x, int y) {
  if (gameOver) return;
  // Ativa as flags de movimento
  switch (key) {
    case GLUT_KEY_LEFT: key_a_pressed = true; break;
    case GLUT_KEY_RIGHT: key_d_pressed = true; break;
  }
}

// Chamada quando uma tecla especial é solta
void specialKeyboardUp(int key, int x, int y) {
  // Desativa as flags de movimento
  if (key == GLUT_KEY_LEFT) key_a_pressed = false;
  if (key == GLUT_KEY_RIGHT) key_d_pressed = false;
}