#include "render/SoundManager.h"
#include <cstdio>

SoundManager& SoundManager::instance() {
    static SoundManager inst;
    return inst;
}

bool SoundManager::loadBuffer(int idx, const char* path) {
    if (bufs_[idx].loadFromFile(path)) {
        return true;
    }
    std::fprintf(stderr, "[SoundManager] 加载失败: %s\n", path);
    return false;
}

void SoundManager::loadAll() {
    if (loaded_) return;
    loaded_ = true;
    loadBuffer(CLICK,   "assets/sounds/click.ogg");
    loadBuffer(DEAL,    "assets/sounds/deal.ogg");
    loadBuffer(FLIP,    "assets/sounds/flip.ogg");
    loadBuffer(CHIP,    "assets/sounds/chip.ogg");
    loadBuffer(WIN,     "assets/sounds/win.ogg");
    loadBuffer(LOSE,    "assets/sounds/lose.ogg");
    loadBuffer(SHUFFLE, "assets/sounds/shuffle.ogg");
    loadBuffer(BET,     "assets/sounds/bet.ogg");
    // BGM(MP3 整曲,SoundBuffer 整曲载入后循环)
    loadBuffer(BGM_MENU, "assets/sounds/bgm_menu.mp3");
    loadBuffer(BGM_GAME, "assets/sounds/bgm_game.mp3");
}

void SoundManager::play(int idx) {
    if (bufs_[idx].getSampleCount() == 0) return;  // 未加载成功
    sound_.setBuffer(bufs_[idx]);
    sound_.play();
}

void SoundManager::playClick()   { play(CLICK); }
void SoundManager::playDeal()    { play(DEAL); }
void SoundManager::playFlip()    { play(FLIP); }
void SoundManager::playChip()    { play(CHIP); }
void SoundManager::playWin()     { play(WIN); }
void SoundManager::playLose()    { play(LOSE); }
void SoundManager::playShuffle() { play(SHUFFLE); }
void SoundManager::playBet()     { play(BET); }

// ---- BGM ----
void SoundManager::playBgm(int idx) {
    if (bufs_[idx].getSampleCount() == 0) return;  // 未加载成功
    bgm_.stop();
    bgm_.setBuffer(bufs_[idx]);
    bgm_.setLoop(true);   // 循环播放
    bgm_.play();
}

void SoundManager::playBgmMenu() { playBgm(BGM_MENU); }
void SoundManager::playBgmGame() { playBgm(BGM_GAME); }
void SoundManager::stopBgm()     { bgm_.stop(); }
