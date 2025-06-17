#ifndef SCENERY_H
#define SCENERY_H

#include "GameObjects.h"
#include <vector>

// Funções do cenário
void initUrbanScenery(std::vector<Building> &cityscape);
void drawUrbanScenery(const std::vector<Building> &cityscape, int windowWidth,
                      int windowHeight);

#endif // SCENERY_H
