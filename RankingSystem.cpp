#include "RankingSystem.h"
#include "GameConstants.h"
#include <algorithm>
#include <cstdio>
#include <fstream>
#include <sstream>

void loadRanking(std::vector<PlayerScore> &ranking) {
  std::ifstream inFile(RANKING_FILENAME);
  if (!inFile.is_open()) {
    return;
  }

  ranking.clear();
  std::string line;
  while (std::getline(inFile, line)) {
    std::stringstream ss(line);
    std::string name;
    int score_val;
    if (ss >> name >> score_val) {
      ranking.push_back({name, score_val});
    }
  }
  inFile.close();

  std::sort(ranking.begin(), ranking.end(),
            [](const PlayerScore &a, const PlayerScore &b) {
              return a.score > b.score;
            });
}

void saveRanking(const std::string &playerName, int finalScore,
                 std::vector<PlayerScore> &ranking) {
  loadRanking(ranking);
  ranking.push_back({playerName, finalScore});

  std::sort(ranking.begin(), ranking.end(),
            [](const PlayerScore &a, const PlayerScore &b) {
              return a.score > b.score;
            });

  std::ofstream outFile(RANKING_FILENAME);
  if (!outFile.is_open()) {
    fprintf(stderr, "Erro ao salvar ranking.\n");
    return;
  }

  for (const auto &ps : ranking) {
    outFile << ps.name << " " << ps.score << std::endl;
  }
  outFile.close();
}
