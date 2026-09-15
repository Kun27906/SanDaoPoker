#pragma once
#include <SFML/Graphics.hpp>
#include "ui/IconButton.h"
#include "ui/Button.h"
#include "ui/TextBox.h"

// 所有界面左上角常驻: [menuList] [music] [wrench] [home]
// - menuList: 打开游戏内大弹窗
// · 主菜单页: 音量(soundSetting 图标可点击=静音切换, 滑动条 slider 拖动
// 调 0~100 全局音量, 音量 0 时图标变 soundOff) + [游戏规则]按钮
// · 规则页: 标题"游戏规则" + 速览正文 + [返回]
// - music: 背景音乐开关, 默认开
// - wrench: 开发者模式弹窗:
// · 未启用: [启用开发者模式] 两段确认
// · 已启用: 居中标注""; 输入框
// + 左筹码图标 + 右滑条 0~100000; [关闭开发者模式]
// - home: 转发给当前场景
// 由 SceneManager 持有; 弹窗打开时会拦截事件。
class SceneManager;

class GlobalHud {
public:
    explicit GlobalHud(SceneManager* mgr);

    // 返回 true 表示事件已被本层消费, 场景不应再处理
    bool handleEvent(const sf::Event& e, const sf::RenderWindow& win);
    void update(float dt);       // 两段确认计时复原
    void draw(sf::RenderWindow& win);

    // 局内"一局未结束"或结算界面但本场还有下一局时弹确认窗;
    // 返回 true = 已拦截
    bool onCloseRequested(bool isEscape);
    bool shouldClose() const { return exitConfirmed_; }   // 已确认退出 -> 关闭窗口

private:
    void openPopup();            // menuList 打开菜单弹窗
    void closePopup();
    void updateKnob();           // 音量滑块位置 = 当前音量
    void setVolumeFromMouse(float mx);
    void setVolume(int v);       // 设音量并同步滑块/图标
    void toggleMute();           // 点音量图标: 静音 <-> 恢复
    void showRulesPage();        // 切到"游戏规则"页
    void showMainPage();         // 回到主菜单页
    void openDevPopup();         // wrench 打开开发者模式弹窗
    void refreshDevToggle();     // 统一刷新"启用/关闭开发者模式"按钮外观
    void drawDevPopup(sf::RenderWindow& win);   // 绘制开发者模式弹窗
    void updateDevKnob();        // dev 滑块位置 = 当前余额
    void setBalanceFromMouse(float mx);
    void applyDevInput();        // 输入框内容 -> 余额
    void applyDevBalance(int v); // 统一写余额: 账号 + 本人筹码, 防两处脱节
    void handleDevText(const sf::Event& e);
    void openExitPopup(bool inRound);   // 打开退出确认弹窗
    void confirmExit();          // [确定]: 按逃跑提前结算 + 标记退出

    SceneManager* mgr_;
    IconButton btnMenu_, btnMusic_, btnWrench_, btnHome_;
    bool popupOpen_ = false;
    bool showRules_ = false;     // 菜单弹窗当前页: false=主菜单 true=游戏规则

    // 开发者模式
    bool devOn_ = false;
    bool devArm_ = false;        // "再点一次确认启用"状态
    float devArmTimer_ = 0.f;

    // 弹窗部件
    sf::RectangleShape overlay_;
    sf::RectangleShape panel_;
    sf::RectangleShape track_;       // 音量滑轨
    sf::RectangleShape trackFill_;   // 音量滑轨
    IconButton btnClose_;            // 右上角关闭
    IconButton btnVolIcon_;          // soundSetting / soundOff
    IconButton knob_;                // slider 指示点
    Button btnRules_;                // 主菜单页: 游戏规则
    TextBox rulesTitle_;             // 规则页标题
    TextBox rulesText_;              // 规则页正文
    Button btnRulesBack_;            // 规则页: 返回
    sf::FloatRect trackRect_;        // 滑轨几何
    float vol_ = 100.f;
    float savedVol_ = 100.f;         // 静音前音量
    bool dragging_ = false;

    Button btnDevToggle_;            // 启用/关闭 开发者模式
    sf::RectangleShape devTrack_;    // dev 滑轨
    sf::RectangleShape devFill_;     // dev 滑轨
    IconButton devKnob_;             // dev 滑块
    sf::FloatRect devTrackRect_;     // dev 滑轨几何
    bool devDrag_ = false;
    // 输入框
    sf::RectangleShape devInputBox_;
    bool devInputFocus_ = false;
    std::string devInputStr_;        // 聚焦编辑中的数字串
    bool devPopupOpen_ = false;      // dev 弹窗是否打开

    bool exitPopupOpen_ = false;     // 确认窗是否打开
    bool exitConfirmed_ = false;     // 玩家点[确定] -> GameApp 关闭窗口
    sf::RectangleShape exitDialog_;  // 独立小面板
    TextBox exitText_;
    Button btnExitOk_, btnExitCancel_;
};
