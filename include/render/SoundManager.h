#pragma once
#include <SFML/Audio.hpp>
#include <string>

// 单例。统一加载 assets/sounds/ 下的 CC0 音效
// 素材清单: deal发牌 / flip翻牌 / chip筹码 / win胜利 / lose失败 /
// bet下注 / coins金币结算 / error禁用提示 / count_down_clock倒计时 / click按钮
// 用法:
// SoundManager::instance.loadAll; // GameApp 启动时调用
// SoundManager::instance.playDeal; // 需要时播放
class SoundManager {
public:
    static SoundManager& instance();

    // 加载全部音效
    void loadAll();

    void playClick();     // 按钮点击
    void playDeal();      // 发牌
    void playFlip();      // 翻牌
    void playChip();      // 筹码
    void playWin();       // 胜利
    void playLose();      // 失败
    void playBet();       // 下注
    void playCoins();     // 金币结算
    void playError();     // 操作不可用提示音

    void startClock();    // 开始循环播放 count_down_clock
    void stopClock();     // 停止

    void playBgmMenu();   // 主菜单循环
    void playBgmGame();   // 对局循环
    void toggleBgm();     // 背景音乐开关
    bool bgmOn() const { return bgmOn_; }

    // 主短音通道是否正在播放
    bool isPlaying() const { return sound_.getStatus() == sf::Sound::Playing; }

    // 用 sf::Listener::setGlobalVolume:对所有 Sound/Music 生效,
    // 可移植
    void setVolume(int v);
    int volume() const { return volume_; }

private:
    SoundManager() = default;
    bool loadBuffer(int idx, const char* path);
    void play(int idx);
    void play2(int idx);          // 第二短音通道
    void playBgm(int idx);

    enum { CLICK, DEAL, FLIP, CHIP, WIN, LOSE, BET, COINS, ERROR, CLOCK,
           BGM_MENU, BGM_GAME, COUNT };
    sf::SoundBuffer bufs_[COUNT];
    sf::Sound sound_;      // 主短音通道
    sf::Sound sound2_;     // 副短音通道
    sf::Sound clock_;      // 倒计时时钟
    sf::Sound bgm_;        // BGM 循环播放器
    bool loaded_ = false;
    bool bgmOn_ = true;             // 背景音乐总开关
    int curBgm_ = BGM_MENU;         // 当前应播放的 BGM 曲目
    int volume_ = 100;              // 全局音量 0~100
};
