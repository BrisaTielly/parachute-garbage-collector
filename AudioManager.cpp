#include "AudioManager.h"
#include <SDL2/SDL.h>
#include <iostream>

// Instância global
AudioManager audioManager;

AudioManager::AudioManager()
    : musicVolume(64), soundVolume(128), isInitialized(false) {}

AudioManager::~AudioManager() { cleanup(); }

bool AudioManager::initialize() {
  // Inicializar SDL Audio se ainda não foi inicializado
  if (SDL_WasInit(SDL_INIT_AUDIO) == 0) {
    if (SDL_Init(SDL_INIT_AUDIO) < 0) {
      std::cerr << "SDL Audio could not initialize! SDL Error: "
                << SDL_GetError() << std::endl;
      return false;
    }
  }

  // Inicializar SDL_mixer
  if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
    std::cerr << "SDL_mixer could not initialize! SDL_mixer Error: "
              << Mix_GetError() << std::endl;
    return false;
  }

  isInitialized = true;
  return true;
}

void AudioManager::cleanup() {
  if (!isInitialized)
    return;

  // Parar toda música
  Mix_HaltMusic();
  Mix_HaltChannel(-1);

  // Limpar música
  for (auto &pair : music) {
    if (pair.second) {
      Mix_FreeMusic(pair.second);
    }
  }
  music.clear();

  // Limpar sons
  for (auto &pair : sounds) {
    if (pair.second) {
      Mix_FreeChunk(pair.second);
    }
  }
  sounds.clear();

  Mix_CloseAudio();
  isInitialized = false;
}

bool AudioManager::loadMusic(const std::string &name,
                             const std::string &filepath) {
  if (!isInitialized)
    return false;

  Mix_Music *newMusic = Mix_LoadMUS(filepath.c_str());
  if (newMusic == nullptr) {
    std::cerr << "Failed to load music: " << filepath
              << " SDL_mixer Error: " << Mix_GetError() << std::endl;
    return false;
  }

  music[name] = newMusic;
  return true;
}

bool AudioManager::loadSound(const std::string &name,
                             const std::string &filepath) {
  if (!isInitialized)
    return false;

  Mix_Chunk *newSound = Mix_LoadWAV(filepath.c_str());
  if (newSound == nullptr) {
    std::cerr << "Failed to load sound: " << filepath
              << " SDL_mixer Error: " << Mix_GetError() << std::endl;
    return false;
  }

  sounds[name] = newSound;
  return true;
}

void AudioManager::playMusic(const std::string &name, int loops) {
  if (!isInitialized)
    return;

  auto it = music.find(name);
  if (it != music.end()) {
    Mix_PlayMusic(it->second, loops); // -1 = loop infinito
  }
}

void AudioManager::playMusicWithVolume(const std::string &name, int volume,
                                       int loops) {
  if (!isInitialized)
    return;

  auto it = music.find(name);
  if (it != music.end()) {
    Mix_VolumeMusic(volume); // Ajustar volume temporariamente
    Mix_PlayMusic(it->second, loops);
  }
}

void AudioManager::playSound(const std::string &name) {
  if (!isInitialized)
    return;

  auto it = sounds.find(name);
  if (it != sounds.end()) {
    Mix_PlayChannel(-1, it->second, 0);
  }
}

void AudioManager::pauseMusic() {
  if (!isInitialized)
    return;
  Mix_PauseMusic();
}

void AudioManager::resumeMusic() {
  if (!isInitialized)
    return;
  Mix_ResumeMusic();
}

void AudioManager::stopMusic() {
  if (!isInitialized)
    return;
  Mix_HaltMusic();
}

void AudioManager::setMusicVolume(int volume) {
  if (!isInitialized)
    return;
  musicVolume = volume;
  Mix_VolumeMusic(volume); // 0-128
}

void AudioManager::setSoundVolume(int volume) {
  if (!isInitialized)
    return;
  soundVolume = volume;
  // Aplicar a todos os canais de som
  Mix_Volume(-1, volume);
}

bool AudioManager::isMusicPlaying() {
  if (!isInitialized)
    return false;
  return Mix_PlayingMusic() == 1;
}
