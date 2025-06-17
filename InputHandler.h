#ifndef INPUT_HANDLER_H
#define INPUT_HANDLER_H

// Funções de entrada
void convertMouseToGameCoords(int mouseX, int mouseY, float &gameX,
                              float &gameY);
void keyboard(unsigned char key, int x, int y);
void specialKeyboard(int key, int x, int y);
void specialKeyboardUp(int key, int x, int y);
void mouseClick(int button, int state, int x, int y);
void mousePassiveMotion(int x, int y);

#endif // INPUT_HANDLER_H
