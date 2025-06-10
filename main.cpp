#include <GL/glut.h>   
#include <cstdio>      // Para sprintf (para formatar o texto do título)
#include <cstdlib>     // Para rand() e srand() (geração de números aleatórios)
#include <ctime>       // Para time() (para inicializar o gerador aleatório)
#include <string>      // Para uso de std::string
#include <vector>      // Para std::vector (armazenar os objetos e prédios)
#include <cmath>       // Para funções matemáticas como sin() e cos() (animações do cenário)
#include <algorithm>   // Para std::sort (ordenar as camadas de prédios)
#include <fstream>     // Para operações com arquivos (ranking)
#include <sstream>     // Para std::stringstream (parse do ranking)

// --- Constantes ---
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// --- Variáveis Globais para limites da tela ---
float screenMinX = -1.0f;
float screenMaxX = 1.0f;


// --- Variáveis Globais para dificuldade dinâmica ---
const int MAX_NUM_OBJECTS = 4; // Máximo de objetos caindo simultaneamente
float currentMinObjectSpeed = 0.004f;
float currentMaxObjectSpeedOffset =
  0.003f; // Componente aleatória da velocidade

const float INITIAL_MIN_OBJECT_SPEED = 0.008f;
const float INITIAL_MAX_OBJECT_SPEED_OFFSET = 0.003f;
const float MAX_MIN_OBJECT_SPEED =
  0.005f; // Velocidade mínima máxima que um objeto pode ter
const float MAX_MAX_OBJECT_SPEED_OFFSET =
  0.007f; // Offset máximo para a velocidade
const float SPEED_INCREASE_AMOUNT = 0.0002f;
const float SPEED_OFFSET_INCREASE_AMOUNT = 0.0001f;

int scoreForNextDifficultyIncrease =
  5; // Pontuação para o próximo aumento de dificuldade
const int SCORE_INCREMENT_FOR_DIFFICULTY =
  5; // A cada X pontos, aumenta a dificuldade

// Enum para os tipos de lixo
enum WASTE_TYPE {
  PAPER = 0,      // 0
  PLASTIC,        // 1
  METAL,          // 2
  GLASS,          // 3
  ORGANIC,        // 4
  WASTE_TYPE_COUNT
};

// Tabela de cores RGB, na mesma ordem do enum WASTE_TYPE
static const GLfloat COLOR_TABLE[WASTE_TYPE_COUNT][3] = {
  {0.0f, 0.5f, 0.8f},   // PAPER (Azul)
  {0.8f, 0.2f, 0.2f},   // PLASTIC (Vermelho)
  {0.9f, 0.8f, 0.1f},   // METAL (Amarelo)
  {0.2f, 0.7f, 0.2f},   // GLASS (Verde)
  {0.5f, 0.35f, 0.05f}  // ORGANIC (Marrom)
};

// Estrutura para os prédios do cenário
struct Building {
  float x_pos, width, height;
  float r, g, b;
  int layer;
};

// Estrutura para representar um objeto caindo
struct FallingObject {
  float x, y;    // Posição
  float size;    // Tamanho do objeto
  float speed;   // Velocidade de queda
  WASTE_TYPE wasteType; // Tipo de lixo
  float rotation, rotationSpeed; // Propriedades de rotação

  FallingObject() {
    size = 0.12f; // Tamanho do modelo visual
    respawn();    // Chama respawn para definir posição inicial e tipo
  }

  void respawn() {
    x = (static_cast<float>(rand()) / static_cast<float>(RAND_MAX)) * 2.0f - 
        1.0f; // Posição X aleatória
    y = 1.0f + (static_cast<float>(rand()) / static_cast<float>(RAND_MAX)) *
        0.5f; // Começa um pouco acima da tela
    // Usa as velocidades dinâmicas atuais
    speed = currentMinObjectSpeed + 
        (static_cast<float>(rand()) / static_cast<float>(RAND_MAX)) * 
            currentMaxObjectSpeedOffset;
    rotation = static_cast<float>(rand() % 360);
    rotationSpeed = (static_cast<float>(rand()) / static_cast<float>(RAND_MAX) - 0.5f) * 2.0f * speed * 100.0f;
    wasteType = static_cast<WASTE_TYPE>(rand() % WASTE_TYPE_COUNT);
  }

  void update() {
    y -= speed;
    rotation += rotationSpeed;
    // o miss será detectado no update principal
    // if (y < -1.0f - size) {
    //   respawn();
    // }
  }

  void draw() {
    glPushMatrix();
    glTranslatef(x, y, 0.0f);
    glRotatef(rotation, 0.0f, 0.0f, 1.0f);
    glScalef(size, size, 1.0f);
    switch (wasteType) {
      case PAPER:
        glColor3f(0.9f, 0.9f, 0.85f); glBegin(GL_QUADS); glVertex2f(-0.5f, -0.2f); glVertex2f(0.5f, -0.2f); glVertex2f(0.5f, 0.2f); glVertex2f(-0.5f, 0.2f); glEnd();
        glColor3f(0.4f, 0.4f, 0.4f); glBegin(GL_LINES); for (int i = 0; i < 4; ++i) { float lineY = -0.15f + i * 0.1f; glVertex2f(-0.4f, lineY); glVertex2f(0.4f, lineY); } glEnd();
        break;
      case PLASTIC:
        glBegin(GL_QUADS); glColor3f(0.8f, 0.2f, 0.2f); glVertex2f(-0.25f, -0.5f); glVertex2f(0.25f, -0.5f); glColor3f(1.0f, 0.5f, 0.5f); glVertex2f(0.25f, 0.2f); glVertex2f(-0.25f, 0.2f); glEnd();
        glBegin(GL_QUADS); glVertex2f(-0.15f, 0.2f); glVertex2f(0.15f, 0.2f); glVertex2f(0.15f, 0.4f); glVertex2f(-0.15f, 0.4f); glEnd();
        glColor3f(0.6f, 0.1f, 0.1f); glBegin(GL_QUADS); glVertex2f(-0.17f, 0.4f); glVertex2f(0.17f, 0.4f); glVertex2f(0.17f, 0.5f); glVertex2f(-0.17f, 0.5f); glEnd();
        glColor4f(1.0f, 1.0f, 1.0f, 0.5f); glBegin(GL_QUADS); glVertex2f(-0.2f, -0.3f); glVertex2f(-0.1f, -0.3f); glVertex2f(-0.1f, 0.1f); glVertex2f(-0.2f, 0.1f); glEnd();
        break;
      case METAL:
        glBegin(GL_QUAD_STRIP); glColor3f(0.6f, 0.6f, 0.65f); glVertex2f(-0.3f, -0.5f); glVertex2f(-0.3f, 0.5f); glColor3f(0.9f, 0.9f, 0.95f); glVertex2f(-0.1f, -0.5f); glVertex2f(-0.1f, 0.5f); glColor3f(0.9f, 0.9f, 0.95f); glVertex2f(0.1f, -0.5f); glVertex2f(0.1f, 0.5f); glColor3f(0.6f, 0.6f, 0.65f); glVertex2f(0.3f, -0.5f); glVertex2f(0.3f, 0.5f); glEnd();
        glColor3f(0.5f, 0.5f, 0.55f); glBegin(GL_QUADS); glVertex2f(-0.3f, 0.5f); glVertex2f(0.3f, 0.5f); glVertex2f(0.3f, 0.4f); glVertex2f(-0.3f, 0.4f); glVertex2f(-0.3f, -0.5f); glVertex2f(0.3f, -0.5f); glVertex2f(0.3f, -0.4f); glVertex2f(-0.3f, -0.4f); glEnd();
        break;
      case GLASS:
        glColor4f(0.2f, 0.7f, 0.2f, 0.7f); glBegin(GL_QUADS); glVertex2f(-0.25f, -0.5f); glVertex2f(0.25f, -0.5f); glVertex2f(0.25f, 0.1f); glVertex2f(-0.25f, 0.1f); glEnd();
        glBegin(GL_TRIANGLES); glVertex2f(-0.25f, 0.1f); glVertex2f(0.25f, 0.1f); glVertex2f(0.15f, 0.3f); glVertex2f(-0.25f, 0.1f); glVertex2f(-0.15f, 0.3f); glVertex2f(0.15f, 0.3f); glEnd();
        glBegin(GL_QUADS); glVertex2f(-0.1f, 0.3f); glVertex2f(0.1f, 0.3f); glVertex2f(0.1f, 0.5f); glVertex2f(-0.1f, 0.5f); glEnd();
        glColor4f(1.0f, 1.0f, 1.0f, 0.6f); glBegin(GL_QUADS); glVertex2f(0.1f, -0.4f); glVertex2f(0.18f, -0.4f); glVertex2f(0.18f, 0.2f); glVertex2f(0.1f, 0.2f); glEnd();
        break;
      case ORGANIC:
        glBegin(GL_POLYGON); glColor3f(0.9f, 0.1f, 0.1f); for (int i = 0; i < 20; i++) { float ang = 2.0f * M_PI * i / 20.0f; glVertex2f(cos(ang) * 0.4f, sin(ang) * 0.5f); } glEnd();
        glColor3f(0.4f, 0.2f, 0.0f); glBegin(GL_QUADS); glVertex2f(-0.05f, 0.4f); glVertex2f(0.05f, 0.4f); glVertex2f(0.05f, 0.6f); glVertex2f(-0.05f, 0.6f); glEnd();
        glColor3f(0.1f, 0.8f, 0.1f); glBegin(GL_TRIANGLES); glVertex2f(0.05f, 0.5f); glVertex2f(0.3f, 0.7f); glVertex2f(0.1f, 0.4f); glEnd();
        break;
    }
    glPopMatrix();
  }
};

// Estrutura para representar a cesta
struct Basket {
  float x, y;          // Posição do centro da cesta
  float width, height; // Dimensões da cesta
  WASTE_TYPE wasteType; // Tipo de lixo que a cesta aceita
  float speed;         // Velocidade de movimento da cesta

  Basket() {
    width = 0.3f;
    height = 0.2f;
    x = 0.0f;                  // Começa no centro
    y = -0.8f + height / 2.0f; // Posição Y na parte inferior
    wasteType = PAPER;         // Tipo inicial da cesta
    speed = 0.05f;
  }

  void draw() {
    const float* color = COLOR_TABLE[wasteType];
    glBegin(GL_QUADS); glColor3f(color[0] * 0.7f, color[1] * 0.7f, color[2] * 0.7f); glVertex2f(x - width / 2, y - height / 2); glVertex2f(x + width / 2, y - height / 2); glColor3f(color[0], color[1], color[2]); glVertex2f(x + width / 2, y + height / 2); glVertex2f(x - width / 2, y + height / 2); glEnd();
    glColor3f(color[0] * 0.5f, color[1] * 0.5f, color[2] * 0.5f); glBegin(GL_QUADS); glVertex2f(x - width / 2 - 0.02f, y + height / 2); glVertex2f(x + width / 2 + 0.02f, y + height / 2); glVertex2f(x + width / 2 + 0.02f, y + height / 2 + 0.03f); glVertex2f(x - width / 2 - 0.02f, y + height / 2 + 0.03f); glEnd();
    glColor3f(1.0f, 1.0f, 1.0f); float s = 0.05f; glPushMatrix(); glTranslatef(x, y, 0.0f); for(int i = 0; i < 3; ++i) { glRotatef(120.0, 0, 0, 1); glBegin(GL_QUADS); glVertex2f(-s, s); glVertex2f(s, s); glVertex2f(s*1.5, s*1.8); glVertex2f(-s*0.5, s*1.8); glEnd(); } glPopMatrix();
  }

  void move(float direction) { // direction será -1 para esquerda, 1 para direita
    x += direction * speed;
    // Limitar o movimento da cesta dentro da tela
    if (x - width / 2 < screenMinX) {
      x = screenMinX + width / 2;
    }
    if (x + width / 2 > screenMaxX) {
      x = screenMaxX - width / 2;
    }
  }
};

// --- Variáveis Globais ---
std::vector<FallingObject> objects;
std::vector<Building> cityscape;
// const int NUM_OBJECTS = 10; // Removido - agora é dinâmico
Basket basket;
int score = 0;
int misses = 0; // Nova variável para contar misses
const int MAX_MISSES = 8; // Máximo de misses permitidos
bool gameOver = false; // Flag para indicar fim de jogo
int windowWidth = 600;
int windowHeight = 800;

bool key_a_pressed = false;
bool key_d_pressed = false;

// --- Ranking System ---
struct PlayerScore {
  std::string name;
  int score;
};
std::vector<PlayerScore> ranking;
const std::string RANKING_FILENAME = "ranking.txt";
const std::string PLAYER_NAME = "Jogador1"; // Nome do jogador (hardcoded)
const int MAX_RANKING_DISPLAY_ENTRIES = 5; // Quantos scores mostrar na tela de game over

// --- Funções Auxiliares ---
void updateWindowTitle() {
  char title[100];
  sprintf(title, "Coleta Seletiva - Pontos: %d", score);
  glutSetWindowTitle(title);
}

// Função para desenhar texto na tela
void renderText(float x, float y, void *font, const char *string) {
  glRasterPos2f(x, y);
  while (*string) {
    glutBitmapCharacter(font, *string);
    string++;
  }
}

// --- Funções de Ranking ---
void loadRanking() {
  std::ifstream inFile(RANKING_FILENAME);
  if (!inFile.is_open()) {
    // Arquivo não existe ou não pode ser aberto, será criado ao salvar.
    return;
  }
  ranking.clear();
  std::string line;
  while (std::getline(inFile, line)) {
    std::stringstream ss(line);
    std::string name;
    int score_val;
    // Assume formato "Nome Score"
    if (ss >> name >> score_val) {
      ranking.push_back({name, score_val});
    }
  }
  inFile.close();
  std::sort(ranking.begin(), ranking.end(), [](const PlayerScore& a, const PlayerScore& b) {
    return a.score > b.score; // Ordena em ordem decrescente de pontuação
  });
}

void saveRanking(int finalScore) {
  ranking.push_back({PLAYER_NAME, finalScore});
  std::sort(ranking.begin(), ranking.end(), [](const PlayerScore& a, const PlayerScore& b) {
    return a.score > b.score; // Ordena em ordem decrescente de pontuação
  });

  // Opcional: Limitar o número de entradas no ranking (ex: top 10)
  // const int MAX_RANKING_ENTRIES_FILE = 10;
  // if (ranking.size() > MAX_RANKING_ENTRIES_FILE) {
  //   ranking.resize(MAX_RANKING_ENTRIES_FILE);
  // }

  std::ofstream outFile(RANKING_FILENAME);
  if (!outFile.is_open()) {
    fprintf(stderr, "Erro: Nao foi possivel salvar o ranking em %s\n", RANKING_FILENAME.c_str());
    return;
  }
  for (const auto& ps : ranking) {
    outFile << ps.name << " " << ps.score << std::endl;
  }
  outFile.close();
}


// --- Funções de Cenário ---
void initUrbanScenery() {
  cityscape.clear();
  srand(1337);
  float worldWidth = 4.0f;
  float current_x = -worldWidth;
  while(current_x < worldWidth) { Building b; b.x_pos = current_x; b.width = 0.2f + (static_cast<float>(rand()) / RAND_MAX) * 0.3f; b.height = 0.2f + (static_cast<float>(rand()) / RAND_MAX) * 0.6f; float gray = 0.15f + (static_cast<float>(rand()) / RAND_MAX) * 0.1f; b.r = gray; b.g = gray; b.b = gray + 0.05f; b.layer = 0; cityscape.push_back(b); current_x += b.width + 0.05f; }
  current_x = -worldWidth;
  while(current_x < worldWidth) { Building b; b.x_pos = current_x; b.width = 0.3f + (static_cast<float>(rand()) / RAND_MAX) * 0.2f; b.height = -0.2f + (static_cast<float>(rand()) / RAND_MAX) * 0.3f; float gray = 0.25f + (static_cast<float>(rand()) / RAND_MAX) * 0.1f; b.r = gray; b.g = gray; b.b = gray; b.layer = 1; cityscape.push_back(b); current_x += b.width + 0.1f; }
  std::sort(cityscape.begin(), cityscape.end(), [](const Building& a, const Building& b){ return a.layer < b.layer; });
  srand(static_cast<unsigned int>(time(0)));
}

void drawUrbanScenery() {
  float aspect = (float)glutGet(GLUT_WINDOW_WIDTH) / (float)glutGet(GLUT_WINDOW_HEIGHT);
  float worldLeft = -1.0, worldRight = 1.0, worldTop = 1.0, worldBottom = -1.0;
  if (aspect > 1.0) { worldLeft = -aspect; worldRight = aspect; } else { worldTop = 1.0 / aspect; worldBottom = -1.0 / aspect; }
  glBegin(GL_QUADS); glColor3f(0.1f, 0.1f, 0.3f); glVertex2f(worldLeft, worldTop); glVertex2f(worldRight, worldTop); glColor3f(0.9f, 0.7f, 0.4f); glVertex2f(worldRight, -0.7f); glVertex2f(worldLeft, -0.7f); glEnd();
  int time_ms = glutGet(GLUT_ELAPSED_TIME);
  for(const auto& b : cityscape) {
    glColor3f(b.r, b.g, b.b); glBegin(GL_QUADS); glVertex2f(b.x_pos, -0.8f); glVertex2f(b.x_pos + b.width, -0.8f); glVertex2f(b.x_pos + b.width, b.height); glVertex2f(b.x_pos, b.height); glEnd();
    float window_margin = 0.1f * b.width; float window_size = 0.08f * b.width; int num_floors = static_cast<int>((b.height + 0.8f) / 0.1f); int num_windows_per_floor = static_cast<int>((b.width - 2*window_margin) / (window_size*1.5f));
    for(int i = 0; i < num_floors; ++i) { for(int j = 0; j < num_windows_per_floor; ++j) { int window_seed = static_cast<int>(b.x_pos * 100) + i * 13 + j * 7; if(sin( (time_ms / 1000.0f) * 0.2f + window_seed ) > 0.8) { glColor3f(0.9f, 0.9f, 0.6f); } else { glColor3f(b.r*0.5f, b.g*0.5f, b.b*0.5f); } float wx = b.x_pos + window_margin + j * (window_size * 1.5f); float wy = -0.75f + i * 0.1f; glBegin(GL_QUADS); glVertex2f(wx, wy); glVertex2f(wx + window_size, wy); glVertex2f(wx + window_size, wy + 0.05f); glVertex2f(wx, wy + 0.05f); glEnd(); } }
  }
  glColor3f(0.2f, 0.2f, 0.2f); glBegin(GL_QUADS); glVertex2f(worldLeft, -1.0f); glVertex2f(worldRight, -1.0f); glVertex2f(worldRight, -0.8f); glVertex2f(worldLeft, -0.8f); glEnd();
}


// --- Funções OpenGL ---
void initGL() {
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  updateWindowTitle(); // Define o título inicial com a pontuação
  initUrbanScenery();
}

void display() {
  glClear(GL_COLOR_BUFFER_BIT); // Limpa o buffer de cor

  // Se o jogo acabou, mostra tela de game over
  if (gameOver) {
    // Desenha fundo escuro semi-transparente
    glColor4f(0.0f, 0.0f, 0.0f, 0.7f);
    glBegin(GL_QUADS);
    glVertex2f(-2.0f, -2.0f);
    glVertex2f(2.0f, -2.0f);
    glVertex2f(2.0f, 2.0f);
    glVertex2f(-2.0f, 2.0f);
    glEnd();

    // Texto de game over
    glColor3f(1.0f, 0.0f, 0.0f);
    renderText(-0.3f, 0.2f, GLUT_BITMAP_TIMES_ROMAN_24, "GAME OVER!");
    
    char finalScoreText[50];
    sprintf(finalScoreText, "Pontuacao Final: %d", score);
    glColor3f(1.0f, 1.0f, 1.0f);
    renderText(-0.4f, 0.0f, GLUT_BITMAP_HELVETICA_18, finalScoreText);
    
    glColor3f(0.8f, 0.8f, 0.8f);
    renderText(-0.3f, -0.2f, GLUT_BITMAP_HELVETICA_12, "Pressione ESC para sair");

    // Mostra o Ranking
    glColor3f(1.0f, 1.0f, 0.0f); // Amarelo para o título do ranking
    renderText(-0.4f, -0.35f, GLUT_BITMAP_HELVETICA_18, "Ranking:");
    
    float rank_y_offset = -0.45f; // Ajuste a posição Y inicial
    int rank_count = 0;
    for (const auto& ps : ranking) {
        if (rank_count >= MAX_RANKING_DISPLAY_ENTRIES) break; 
        char rankEntry[100];
        sprintf(rankEntry, "%d. %s: %d", rank_count + 1, ps.name.c_str(), ps.score);
        glColor3f(1.0f, 1.0f, 1.0f); // Branco para as entradas do ranking
        renderText(-0.4f, rank_y_offset, GLUT_BITMAP_HELVETICA_12, rankEntry);
        rank_y_offset -= 0.05f; // Espaçamento entre as entradas
        rank_count++;
    }
    
    glutSwapBuffers();
    return;
  }

  drawUrbanScenery();

  // --- Adicionar efeito de "blur" ao fundo ---
  // Calcular as bordas do mundo visível com base no aspect ratio atual da janela
  // para garantir que o quad de "blur" cubra todo o fundo.
  // windowWidth e windowHeight são atualizados em reshape().
  float currentAspect = (float)windowWidth / (float)windowHeight;
  float worldLeft, worldRight, worldTop, worldBottom;

  if (windowWidth <= windowHeight) { // Largura <= Altura (retrato ou quadrado)
    worldLeft = -1.0f;
    worldRight = 1.0f;
    worldBottom = -1.0f / currentAspect;
    worldTop = 1.0f / currentAspect;
  } else { // Largura > Altura (paisagem)
    worldLeft = -1.0f * currentAspect;
    worldRight = 1.0f * currentAspect;
    worldBottom = -1.0f;
    worldTop = 1.0f;
  }

  // Desenha um quad semi-transparente sobre o cenário para dar o efeito de blur/profundidade.
  // GL_BLEND já está habilitado em initGL().
  glColor4f(0.1f, 0.1f, 0.1f, 0.45f); // Cor escura, semi-transparente.
  glBegin(GL_QUADS);
  glVertex2f(worldLeft, worldBottom);
  glVertex2f(worldRight, worldBottom);
  glVertex2f(worldRight, worldTop);
  glVertex2f(worldLeft, worldTop);
  glEnd();
  // --- Fim do efeito de "blur" ---

  // Desenha todos os objetos
  for (size_t i = 0; i < objects.size(); ++i) {
    objects[i].draw();
  }

  // Desenha a cesta
  basket.draw();

  // Desenha a pontuação e misses na tela
  char scoreText[50];
  sprintf(scoreText, "Pontos: %d", score);
  glColor3f(1.0f, 1.0f, 1.0f); // Cor branca para o texto
  renderText(-0.95f, 0.9f, GLUT_BITMAP_HELVETICA_18, scoreText);

  char missesText[50];
  sprintf(missesText, "Misses: %d/%d", misses, MAX_MISSES);
  glColor3f(1.0f, 0.5f, 0.5f); // Cor vermelha clara para os misses
  renderText(-0.95f, 0.85f, GLUT_BITMAP_HELVETICA_18, missesText);

  glutSwapBuffers(); // Troca os buffers (double buffering)
}

void update(int value) {
  // Se o jogo acabou, não atualiza nada
  if (gameOver) {
    return;
  }

  if (key_a_pressed && !key_d_pressed) { // Mover para a esquerda se 'a' estiver pressionada e 'd' não
    basket.move(-1.0f);
  } else if (key_d_pressed && !key_a_pressed) { // Mover para a direita se 'd'
                                                // estiver pressionada e 'a' não
    basket.move(1.0f);
  }

  // Atualiza a posição de todos os objetos
  for (size_t i = 0; i < objects.size(); ++i) {
    objects[i].update();

    // Detecção de colisão com a cesta
    float obj_left = objects[i].x - objects[i].size / 2;
    float obj_right = objects[i].x + objects[i].size / 2;
    float obj_bottom = objects[i].y - objects[i].size / 2;
    float obj_top =
         objects[i].y + 
         objects[i].size / 2; // Não usado na colisão abaixo, mas bom ter

    float basket_left = basket.x - basket.width / 2;
    float basket_right = basket.x + basket.width / 2;
    float basket_top = basket.y + basket.height / 2;
    float basket_bottom = basket.y - basket.height / 2;

    // Verifica se o objeto caiu no chão (miss)
    if (objects[i].y < -0.6f - objects[i].size) {
      misses++;
      objects[i].respawn();
      
      // Verifica se atingiu o máximo de misses
      if (misses >= MAX_MISSES) {
        gameOver = true;
        saveRanking(score); // Salva o ranking quando o jogo termina por misses
        glutPostRedisplay();
        return;
      }
    }
    // Condição de colisão:
    else if (obj_right > basket_left && obj_left < basket_right && 
      obj_bottom <= basket_top && obj_bottom >= basket_top - 0.05f) {
      if (objects[i].wasteType == basket.wasteType) {
         // A última condição evita que conte
          // multiplas vezes se o objeto for muito
          // rápido ou o fps baixo
        score++;
      } else {
        // Coletar objeto da cor errada também conta como miss
        misses++;
        if (misses >= MAX_MISSES) {
          gameOver = true;
          saveRanking(score); // Salva o ranking quando o jogo termina por misses na coleta
          objects[i].respawn();
          updateWindowTitle();
          glutPostRedisplay();
          return;
        }
      }
      objects[i].respawn();
      updateWindowTitle();

      // Lógica para aumentar a dificuldade
      if (score >= scoreForNextDifficultyIncrease) {
        // Aumenta a velocidade dos objetos
        if (currentMinObjectSpeed < MAX_MIN_OBJECT_SPEED) {
          currentMinObjectSpeed += SPEED_INCREASE_AMOUNT;
        }
        if (currentMaxObjectSpeedOffset < MAX_MAX_OBJECT_SPEED_OFFSET) {
          currentMaxObjectSpeedOffset += SPEED_OFFSET_INCREASE_AMOUNT;
        }
        // Adiciona mais um objeto se não atingiu o máximo
        if (objects.size() < MAX_NUM_OBJECTS) {
          objects.push_back(FallingObject());
        }
        scoreForNextDifficultyIncrease += SCORE_INCREMENT_FOR_DIFFICULTY;
      }
    }
  }

  glutPostRedisplay(); // Marca a janela atual para ser redesenhada
  glutTimerFunc(16, update, 
                        0); // Chama update novamente após aprox. 16ms
}

void reshape(GLsizei width, GLsizei height) {
  if (height == 0)
    height = 1;        // Previne divisão por zero
  windowWidth = width; // Atualiza as dimensões globais da janela
  windowHeight = height;
  float aspect = (float)width / (float)height;

  glViewport(0, 0, width, height);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();

  // Ajusta as coordenadas do mundo para manter a proporção
  // e atualiza os limites da tela para a cesta
  if (width <= height) {
    //screenMinX = -1.0f;
    //screenMaxX = 1.0f;
    gluOrtho2D( -1.0f, 1.0f, -1.0f / aspect, 1.0f / aspect);
  } else {
    // screenMinY = -1.0f * aspect
    // screenMaxY = 1.0f * aspect;
    gluOrtho2D(-1.0f * aspect, 1.0f * aspect, -1.0f, 1.0f);
  }
 // Garante que a cesta seja reposicionada corretamente se a tela for
  // redimensionada e ela sair dos limites
  basket.move(0); // Garante que a cesta esteja dentro dos limites
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
}

void keyboard(unsigned char key, int x, int y) {
  switch (key) {
    case 27:   // Tecla ESC
      exit(0); // Sai do programa
      break;
    case '1': // Papel (Azul)
      basket.wasteType = PAPER;
      break;
    case '2': // Plástico (Vermelho)
      basket.wasteType = PLASTIC;
      break;
    case '3': // Metal (Amarelo)
      basket.wasteType = METAL;
      break;
    case '4': // Vidro (Verde)
      basket.wasteType = GLASS;
      break;
    case '5': // Orgânico (Marrom)
      basket.wasteType = ORGANIC;
      break;
  }
  glutPostRedisplay(); // Redesenha para mostrar a nova cor da cesta ou outras
                       // mudanças imediatas
}

void keyboardUp(unsigned char key, int x, int y) {
    switch (key) {}
}

// Nova função para teclas especiais (setas) pressionadas
void specialKeyboard(int key, int x, int y) {
  switch (key) {
    case GLUT_KEY_LEFT:
      key_a_pressed = true;
      break;
    case GLUT_KEY_RIGHT:
      key_d_pressed = true;
      break;
  }
  glutPostRedisplay();
}

// Nova função para teclas especiais (setas) soltas
void specialKeyboardUp(int key, int x, int y) {
  switch (key) {
    case GLUT_KEY_LEFT:
      key_a_pressed = false;
      break;
    case GLUT_KEY_RIGHT:
      key_d_pressed = false;
      break;
  }
}

int main(int argc, char **argv) {
  srand(static_cast<unsigned int>(time(0))); // Inicializa o gerador de números aleatórios
  loadRanking(); // Carrega o ranking do arquivo

  // Inicializa os objetos - começa com 1 objeto
  objects.push_back(FallingObject());

  // Inicializa as velocidades com os valores iniciais definidos
  currentMinObjectSpeed = INITIAL_MIN_OBJECT_SPEED;
  currentMaxObjectSpeedOffset = INITIAL_MAX_OBJECT_SPEED_OFFSET;

  // A cesta é inicializada por seu construtor
  glutInit(&argc, argv); // Inicializa o GLUT
  glutInitDisplayMode(GLUT_DOUBLE | 
                    GLUT_RGB | 
                    GLUT_ALPHA);
  glutInitWindowSize(windowWidth, 
                        windowHeight); // Define o tamanho inicial da janela
  glutInitWindowPosition(50, 50);  // Posição inicial da janela
   // O título da janela é definido em initGL e atualizado em updateWindowTitle
  glutCreateWindow("Coleta Seletiva");

  // Registra as funções de callback
  glutDisplayFunc(display); // Registra a função de callback para desenhar
  glutReshapeFunc(reshape); // Registra a função de callback para
                            // redimensionamento da janela
  glutKeyboardFunc(keyboard); // Registra a função de callback para o teclado normal
  glutKeyboardUpFunc(keyboardUp); // Adicione esta linha para registrar a função de tecla solta
  glutSpecialFunc(specialKeyboard);  // Registra a função para teclas especiais pressionadas
  glutSpecialUpFunc(specialKeyboardUp); // Registra a função para teclas especiais soltas
  glutTimerFunc(
                0, update, 
                0);  // Registra a função de callback para atualização da lógica do jogo

  initGL(); // Realiza inicializações do OpenGL
  glutMainLoop(); // Entra no loop principal de eventos do GLUT

  return 0;
}
