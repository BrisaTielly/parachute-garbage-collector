#ifndef RANKING_SYSTEM_H
#define RANKING_SYSTEM_H

#include "GameObjects.h"
#include <vector>

// Funções do sistema de ranking
void loadRanking(std::vector<PlayerScore> &ranking);
void saveRanking(const std::string &playerName, int finalScore,
                 std::vector<PlayerScore> &ranking);

#endif // RANKING_SYSTEM_H
