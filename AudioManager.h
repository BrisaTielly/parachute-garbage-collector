#ifndef AUDIOMANAGER_H
#define AUDIOMANAGER_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include <string>
#include <unordered_map>

class AudioManager {
private:
  std::unordered_map<std::string, Mix_Music *> music;
  std::unordered_map<std::string, Mix_Chunk *> sounds;
  int musicVolume;
  int soundVolume;
  bool isInitialized;

public:
  AudioManager();
  ~AudioManager();

  bool initialize();
  void cleanup();

  bool loadMusic(const std::string &name, const std::string &filepath);
  bool loadSound(const std::string &name, const std::string &filepath);

  void playMusic(const std::string &name, int loops = -1);
  void playMusicWithVolume(const std::string &name, int volume, int loops = -1);
  void playSound(const std::string &name);

  void pauseMusic();
  void resumeMusic();
  void stopMusic();

  void setMusicVolume(int volume);
  void setSoundVolume(int volume);

  bool isMusicPlaying();
  bool isEnabled() const { return isInitialized; }
};

// Instância global do AudioManager
extern AudioManager audioManager;

#endif // AUDIOMANAGER_H
