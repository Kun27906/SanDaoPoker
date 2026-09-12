#pragma once
#include <SFML/Audio.hpp>
#include <string>

// ====== SoundManager 音效管理器(阶段:素材集成) ======
// 单例。统一加载 assets/sounds/ 下的 CC0 音效(Kenney casino-audio/interface-sounds)
// 素材清单: deal发牌 / flip翻牌 / chip筹码 / win胜利 / lose失败 /
//           bet下注 / coins金币结算 / error禁用提示 / count_down_clock倒计时 / click按钮
// 用法:
//   SoundManager::instance().loadAll();   // GameApp 启动时调用
//   SoundManager::instance().playDeal();  // 需要时播放
class SoundManager {
public:
    static SoundManager& instance();

    // 加载全部音效(可重复调用,有保护)
    void loadAll();

    // ---- 播放接口(播放同名音效;加载失败则静默) ----
    void playClick();     // 按钮点击
    void playDeal();      // 发牌
    void playFlip();      // 翻牌
    void playChip();      // 筹码
    void playWin();       // 胜利
    void playLose();      // 失败
    void playBet();       // 下注
    void playCoins();     // 金币结算(独立通道: 可与 win/lose 同时播放)
    void playError();     // 操作不可用提示音(如组牌界面禁用开发者模式)

    // ---- 倒计时时钟(红色区循环播放, 直到交牌或时间耗尽) ----
    void startClock();    // 开始循环播放 count_down_clock
    void stopClock();     // 停止
    bool clockOn() const { return clock_.getStatus() == sf::Sound::Playing; }

    // ---- BGM 循环(主菜单/对局,MP3 整曲循环) ----
    void playBgmMenu();   // 主菜单循环
    void playBgmGame();   // 对局循环
    void toggleBgm();     // 背景音乐开关(不影响音效 click 等)
    bool bgmOn() const { return bgmOn_; }

    // 主短音通道是否正在播放(用于"等音效播完再继续"的时序控制)
    bool isPlaying() const { return sound_.getStatus() == sf::Sound::Playing; }

    // ---- 全局音量(0~100,与系统音量刻度一致) ----
    // 用 sf::Listener::setGlobalVolume:对所有 Sound/Music 生效(音效+BGM),
    // 可移植(不依赖具体系统音量 API)
    void setVolume(int v);
    int volume() const { return volume_; }

private:
    SoundManager() = default;
    bool loadBuffer(int idx, const char* path);
    void play(int idx);
    void play2(int idx);          // 第二短音通道(coins 用, 不打断 win/lose)
    void playBgm(int idx);

    enum { CLICK, DEAL, FLIP, CHIP, WIN, LOSE, BET, COINS, ERROR, CLOCK,
           BGM_MENU, BGM_GAME, COUNT };
    sf::SoundBuffer bufs_[COUNT];
    sf::Sound sound_;      // 主短音通道(按钮/发牌/翻牌/胜负/下注 串行)
    sf::Sound sound2_;     // 副短音通道(金币结算音, 与主通道叠加)
    sf::Sound clock_;      // 倒计时时钟(循环)
    sf::Sound bgm_;        // BGM 循环播放器
    bool loaded_ = false;
    bool bgmOn_ = true;             // 背景音乐总开关
    int curBgm_ = BGM_MENU;         // 当前应播放的 BGM 曲目
    int volume_ = 100;              // 全局音量 0~100
};
