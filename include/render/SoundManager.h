#pragma once
#include <SFML/Audio.hpp>
#include <string>

// 音效单例: 发牌 翻牌 筹码 胜负 下注 金币 提示 倒计时 点击 与 BGM
class SoundManager {
public:
    static SoundManager& instance();

    void loadAll();

    void playClick();
    void playDeal();
    void playFlip();
    void playChip();
    void playWin();
    void playLose();
    void playBet();
    void playCoins();
    void playError();

    void startClock();
    void stopClock();

    void playBgmMenu();
    void playBgmGame();
    void toggleBgm();
    bool bgmOn() const { return bgmOn_; }

    // 主短音通道是否在播
    bool isPlaying() const { return sound_.getStatus() == sf::Sound::Playing; }

    void setVolume(int v);
    int volume() const { return volume_; }

private:
    SoundManager() = default;
    bool loadBuffer(int idx, const char* path);
    void play(int idx);
    void play2(int idx);
    void playBgm(int idx);

    enum { CLICK, DEAL, FLIP, CHIP, WIN, LOSE, BET, COINS, ERROR, CLOCK,
           BGM_MENU, BGM_GAME, COUNT };
    sf::SoundBuffer bufs_[COUNT];
    sf::Sound sound_;
    sf::Sound sound2_;
    sf::Sound clock_;
    sf::Sound bgm_;
    bool loaded_ = false;
    bool bgmOn_ = true;
    int curBgm_ = BGM_MENU;
    int volume_ = 100;
};
