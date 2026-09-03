#pragma once
#include <SFML/Audio.hpp>
#include <string>

// ====== SoundManager 音效管理器(阶段:素材集成) ======
// 单例。统一加载 assets/sounds/ 下的 CC0 音效(Kenney casino-audio/interface-sounds)
// 素材清单: deal发牌 / flip翻牌 / chip筹码 / win胜利 / lose失败 /
//           shuffle洗牌 / bet下注 / click按钮
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
    void playShuffle();   // 洗牌
    void playBet();       // 下注

private:
    SoundManager() = default;
    bool loadBuffer(int idx, const char* path);
    void play(int idx);

    enum { CLICK, DEAL, FLIP, CHIP, WIN, LOSE, SHUFFLE, BET, COUNT };
    sf::SoundBuffer bufs_[COUNT];
    sf::Sound sound_;      // 单声道播放器(短音效串行足够)
    bool loaded_ = false;
};
