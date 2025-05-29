#include <GL/glut.h>
#include <cstdlib> // Para rand() e srand()
#include <ctime>   // Para time()
#include <vector>

// Estrutura para representar um objeto caindo
struct FallingObject {
  float x, y;    // Posição
  float size;    // Tamanho do objeto
  float speed;   // Velocidade de queda
  float r, g, b; // Cor

  FallingObject() {
    respawn();
    size = 0.05f; // Tamanho fixo para este exemplo
    r = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
    g = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
    b = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
  }

  void respawn() {
    x = (static_cast<float>(rand()) / static_cast<float>(RAND_MAX)) * 2.0f -
        1.0f; // Posição X aleatória entre -1.0 e 1.0
    y = 1.0f + (static_cast<float>(rand()) / static_cast<float>(RAND_MAX)) *
                   0.5f; // Começa um pouco acima da tela
    speed =
        0.002f +
        (static_cast<float>(rand()) / static_cast<float>(RAND_MAX)) * 0.005f;
  }

  void update() {
    y -= speed;
    if (y < -1.0f - size) { // Se saiu da tela por baixo
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

// --- Variáveis Globais ---
std::vector<FallingObject> objects;
const int NUM_OBJECTS = 15;
int windowWidth = 600;
int windowHeight = 800;

// --- Funções OpenGL ---

void initGL() {
  glClearColor(0.1f, 0.1f, 0.2f, 1.0f); // Cor de fundo azul escuro
}

void display() {
  glClear(GL_COLOR_BUFFER_BIT); // Limpa o buffer de cor

  // Desenha todos os objetos
  for (size_t i = 0; i < objects.size(); ++i) {
    objects[i].draw();
  }

  glutSwapBuffers(); // Troca os buffers (double buffering)
}

void update(int value) {
  // Atualiza a posição de todos os objetos
  for (size_t i = 0; i < objects.size(); ++i) {
    objects[i].update();
  }

  glutPostRedisplay(); // Marca a janela atual para ser redesenhada
  glutTimerFunc(16, update,
                0); // Chama update novamente após aprox. 16ms (visando ~60 FPS)
}

void reshape(GLsizei width, GLsizei height) {
  if (height == 0)
    height = 1; // Previne divisão por zero
  float aspect = (float)width / (float)height;

  glViewport(0, 0, width, height);

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  // Mantém a proporção da cena se a janela for redimensionada
  if (width >= height) {
    // Coordenadas x de -aspect a aspect, y de -1 a 1
    gluOrtho2D(-1.0 * aspect, 1.0 * aspect, -1.0, 1.0);
  } else {
    // Coordenadas x de -1 a 1, y de -1/aspect a 1/aspect
    gluOrtho2D(-1.0, 1.0, -1.0 / aspect, 1.0 / aspect);
  }
  // A origem (0,0) é o centro. Y vai de -1 (baixo) a 1 (topo). X vai de -aspect
  // a aspect. No nosso caso, como o respawn é entre -1 e 1 no X, e o desenho
  // dos objetos também é nesse intervalo, a lógica de gluOrtho2D acima garante
  // que a área visível seja adaptada. Para manter a simplicidade do respawn em
  // x (-1 a 1), é melhor usar glOrtho(-1.0, 1.0, -1.0, 1.0, -1.0, 1.0); e
  // deixar que os objetos fiquem "esticados" ou "achatados" se a janela não for
  // quadrada, ou ajustar o respawn do x para considerar o aspect ratio. Para
  // este protótipo simples, vamos manter o padrão de -1 a 1 que o GLUT
  // estabeleceria por padrão se não chamássemos gluOrtho2D explicitamente,

  // Vamos usar uma projeção ortográfica simples de -1 a 1 para X e Y.
  // Isso significa que o canto inferior esquerdo é (-1,-1) e o superior direito
  // é (1,1).
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  if (width <= height)
    gluOrtho2D(-1.0, 1.0, -1.0 * (GLfloat)height / (GLfloat)width,
               1.0 * (GLfloat)height / (GLfloat)width);
  else
    gluOrtho2D(-1.0 * (GLfloat)width / (GLfloat)height,
               1.0 * (GLfloat)width / (GLfloat)height, -1.0, 1.0);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
}

void keyboard(unsigned char key, int x, int y) {
  switch (key) {
  case 27:   // Tecla ESC
    exit(0); // Sai do programa
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

  glutInit(&argc, argv); // Inicializa o GLUT
  glutInitDisplayMode(GLUT_DOUBLE |
                      GLUT_RGB); // Habilita double buffering e cores RGB
  glutInitWindowSize(windowWidth,
                     windowHeight); // Define o tamanho inicial da janela
  glutInitWindowPosition(50, 50);   // Posição inicial da janela
  glutCreateWindow(
      "Objetos Caindo - Prototipo GLUT"); // Cria a janela com um título

  glutDisplayFunc(display);   // Registra a função de callback para desenhar
  glutReshapeFunc(reshape);   // Registra a função de callback para
                              // redimensionamento da janela
  glutKeyboardFunc(keyboard); // Registra a função de callback para o teclado
  glutTimerFunc(
      0, update,
      0); // Registra a função de callback para atualização da lógica do jogo

  initGL(); // Realiza inicializações do OpenGL que não mudam

  glutMainLoop(); // Entra no loop principal de eventos do GLUT

  return 0;
}