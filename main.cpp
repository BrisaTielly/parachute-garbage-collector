#include <GL/glut.h>
#include <cstdio> // Para sprintf (para exibir a pontuação no título)
#include <cstdlib> // Para rand() e srand()
#include <ctime>   // Para time()
#include <string>  // Para std::to_string (alternativa para sprintf)
#include <vector>

// --- Variáveis Globais para limites da tela ---
float screenMinX = -1.0f;
float screenMaxX = 1.0f;
// float screenMinY = -1.0f; // Não usado diretamente neste exemplo, mas bom
// saber float screenMaxY = 1.0f; // Não usado diretamente neste exemplo, mas
// bom saber

// Estrutura para representar um objeto caindo
struct FallingObject {
  float x, y;    // Posição
  float size;    // Tamanho do objeto
  float speed;   // Velocidade de queda
  float r, g, b; // Cor

  FallingObject() {
    size = 0.05f; // Tamanho fixo para este exemplo
    respawn();    // Chama respawn para definir posição inicial e cor
  }

  void respawn() {
    x = (static_cast<float>(rand()) / static_cast<float>(RAND_MAX)) * 2.0f -
        1.0f; // Posição X aleatória entre -1.0 e 1.0
    y = 1.0f + (static_cast<float>(rand()) / static_cast<float>(RAND_MAX)) *
                   0.5f; // Começa um pouco acima da tela
    speed =
        0.002f +
        (static_cast<float>(rand()) / static_cast<float>(RAND_MAX)) * 0.005f;
    // Cor aleatória no respawn também
    r = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
    g = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
    b = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
  }

  void update() {
    y -= speed;
    // Se saiu da tela por baixo (não foi pego pela cesta)
    if (y < -1.0f - size) {
      respawn();
    }
  }

  void draw() {
    glColor3f(r, g, b);
    glBegin(GL_QUADS);
    glVertex2f(x - size / 2, y - size / 2);
    glVertex2f(x + size / 2, y - size / 2);
    glVertex2f(x + size / 2, y + size / 2);
    glVertex2f(x - size / 2, y + size / 2);
    glEnd();
  }
};

// Estrutura para representar a cesta
struct Basket {
  float x, y;          // Posição do centro da cesta
  float width, height; // Dimensões da cesta
  float r, g, b;       // Cor da cesta
  float speed;         // Velocidade de movimento da cesta

  Basket() {
    width = 0.25f;
    height = 0.08f;
    x = 0.0f;                  // Começa no centro
    y = -0.9f + height / 2.0f; // Posição Y fixa na parte inferior
    r = 0.8f;
    g = 0.8f;
    b = 0.8f; // Cor cinza claro
    speed = 0.05f;
  }

  void draw() {
    glColor3f(r, g, b);
    glBegin(GL_QUADS);
    glVertex2f(x - width / 2, y - height / 2);
    glVertex2f(x + width / 2, y - height / 2);
    glVertex2f(x + width / 2, y + height / 2);
    glVertex2f(x - width / 2, y + height / 2);
    glEnd();
  }

  void
  move(float direction) { // direction será -1 para esquerda, 1 para direita
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
const int NUM_OBJECTS = 10; // Aumentei um pouco para mais diversão
Basket basket;
int score = 0;
int windowWidth = 600;
int windowHeight = 800;

bool key_a_pressed = false;
bool key_d_pressed = false;

// --- Funções Auxiliares ---
void updateWindowTitle() {
  char title[100];
  sprintf(title, "Objetos Caindo - Pontos: %d", score);
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

// --- Funções OpenGL ---

void initGL() {
  glClearColor(0.1f, 0.1f, 0.2f, 1.0f); // Cor de fundo azul escuro
  updateWindowTitle(); // Define o título inicial com a pontuação
}

void display() {
  glClear(GL_COLOR_BUFFER_BIT); // Limpa o buffer de cor

  // Desenha todos os objetos
  for (size_t i = 0; i < objects.size(); ++i) {
    objects[i].draw();
  }

  // Desenha a cesta
  basket.draw();

  // Desenha a pontuação na tela
  char scoreText[50];
  sprintf(scoreText, "Pontos: %d", score);
  glColor3f(1.0f, 1.0f, 1.0f); // Cor branca para o texto
  renderText(-0.95f, 0.9f, GLUT_BITMAP_HELVETICA_18, scoreText);

  glutSwapBuffers(); // Troca os buffers (double buffering)
}

void update(int value) {
  if (key_a_pressed && !key_d_pressed) { // Mover para a esquerda se 'a' estiver
                                         // pressionada e 'd' não
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

    // Condição de colisão:
    // O objeto está horizontalmente sobre a cesta E
    // a base do objeto tocou ou passou um pouco abaixo do topo da cesta E
    // o objeto ainda não passou completamente pela cesta (seu topo está acima
    // da base da cesta)
    if (obj_right > basket_left && obj_left < basket_right &&
        obj_bottom <= basket_top &&
        obj_bottom >=
            basket_bottom -
                objects[i].size) { // A última condição evita que conte
                                   // multiplas vezes se o objeto for muito
                                   // rápido ou o fps baixo
      objects[i].respawn();
      score++;
      updateWindowTitle();
    }
  }

  glutPostRedisplay(); // Marca a janela atual para ser redesenhada
  glutTimerFunc(16, update,
                0); // Chama update novamente após aprox. 16ms (visando ~60 FPS)
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
    screenMinX = -1.0f;
    screenMaxX = 1.0f;
    // screenMinY = -1.0f * (GLfloat)height / (GLfloat)width;
    // screenMaxY = 1.0f * (GLfloat)height / (GLfloat)width;
    gluOrtho2D(screenMinX, screenMaxX, -1.0f * (GLfloat)height / (GLfloat)width,
               1.0f * (GLfloat)height / (GLfloat)width);
  } else {
    // screenMinY = -1.0f;
    // screenMaxY = 1.0f;
    screenMinX = -1.0f * (GLfloat)width / (GLfloat)height;
    screenMaxX = 1.0f * (GLfloat)width / (GLfloat)height;
    gluOrtho2D(screenMinX, screenMaxX, -1.0f, 1.0f);
  }
  // Garante que a cesta seja reposicionada corretamente se a tela for
  // redimensionada e ela sair dos limites
  basket.move(0);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
}

void keyboard(unsigned char key, int x, int y) {
  switch (key) {
  case 27:   // Tecla ESC
    exit(0); // Sai do programa
    break;
  case '1': // Azul
    basket.r = 0.0f;
    basket.g = 0.0f;
    basket.b = 1.0f;
    break;
  case '2': // Vermelho
    basket.r = 1.0f;
    basket.g = 0.0f;
    basket.b = 0.0f;
    break;
  case '3': // Amarelo
    basket.r = 1.0f;
    basket.g = 1.0f;
    basket.b = 0.0f;
    break;
  case '4': // Verde
    basket.r = 0.0f;
    basket.g = 1.0f;
    basket.b = 0.0f;
    break;
  case '5': // Cinza (cor original)
    basket.r = 0.8f;
    basket.g = 0.8f;
    basket.b = 0.8f;
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
  srand(static_cast<unsigned int>(
      time(0))); // Inicializa o gerador de números aleatórios

  // Inicializa os objetos
  for (int i = 0; i < NUM_OBJECTS; ++i) {
    objects.push_back(FallingObject());
  }
  // A cesta é inicializada por seu construtor

  glutInit(&argc, argv); // Inicializa o GLUT
  glutInitDisplayMode(GLUT_DOUBLE |
                      GLUT_RGB); // Habilita double buffering e cores RGB
  glutInitWindowSize(windowWidth,
                     windowHeight); // Define o tamanho inicial da janela
  glutInitWindowPosition(50, 50);   // Posição inicial da janela
  // O título da janela é definido em initGL e atualizado em updateWindowTitle
  glutCreateWindow("Objetos Caindo");

  glutDisplayFunc(display); // Registra a função de callback para desenhar
  glutReshapeFunc(reshape); // Registra a função de callback para
                            // redimensionamento da janela
  glutKeyboardFunc(
      keyboard); // Registra a função de callback para o teclado normal
  glutKeyboardUpFunc(
      keyboardUp); // Adicione esta linha para registrar a função de tecla solta
  glutSpecialFunc(
      specialKeyboard); // Registra a função para teclas especiais pressionadas
  glutSpecialUpFunc(
      specialKeyboardUp); // Registra a função para teclas especiais soltas
  glutTimerFunc(
      0, update,
      0); // Registra a função de callback para atualização da lógica do jogo

  initGL(); // Realiza inicializações do OpenGL que não mudam

  glutMainLoop(); // Entra no loop principal de eventos do GLUT

  return 0;
}