#pragma once
#include <SFML/Graphics.hpp>
#include "ui/IconButton.h"
#include "ui/Button.h"
#include "ui/TextBox.h"

// ====== GlobalHud 全局悬浮工具栏(成员C, 阶段9) ======
// 所有界面左上角常驻: [menuList] [music] [wrench] [home](按场景显隐)
//   - menuList: 打开游戏内大弹窗(右上角圆圈+close 关闭)
//     · 主菜单页: 音量(soundSetting 图标可点击=静音切换, 滑动条 slider 拖动
//       调 0~100 全局音量, 音量 0 时图标变 soundOff) + [游戏规则]按钮
//     · 规则页(同尺寸): 标题"游戏规则" + 速览正文 + [返回]
//   - music:    背景音乐开关(不影响音效), 默认开(musicOn)
//   - wrench:   占位(后续功能)
//   - home:     转发给当前场景(选房返回大厅 / 结算=逃跑或返回大厅; 组牌比牌时隐藏)
// 由 SceneManager 持有; 弹窗打开时会拦截事件(不穿透到场景)。
class SceneManager;

class GlobalHud {
public:
    explicit GlobalHud(SceneManager* mgr);

    // 返回 true 表示事件已被本层消费(弹窗打开时), 场景不应再处理
    bool handleEvent(const sf::Event& e, const sf::RenderWindow& win);
    void draw(sf::RenderWindow& win);

private:
    void openPopup();
    void closePopup();
    void updateKnob();               // 滑块位置 = 当前音量
    void setVolumeFromMouse(float mx);
    void setVolume(int v);           // 设音量并同步滑块/图标
    void toggleMute();               // 点音量图标: 静音 <-> 恢复
    void showRulesPage();            // 切到"游戏规则"页
    void showMainPage();             // 回到主菜单页

    SceneManager* mgr_;
    IconButton btnMenu_, btnMusic_, btnWrench_, btnHome_;
    bool popupOpen_ = false;
    bool showRules_ = false;         // 弹窗当前页: false=主菜单 true=游戏规则

    // 弹窗部件
    sf::RectangleShape overlay_;
    sf::RectangleShape panel_;
    sf::RectangleShape track_;       // 音量滑轨(底)
    sf::RectangleShape trackFill_;   // 音量滑轨(金色已填充部分)
    IconButton btnClose_;            // 右上角关闭(圆内)
    IconButton btnVolIcon_;          // soundSetting / soundOff(点击=静音切换)
    IconButton knob_;                // slider 指示点(可拖动)
    Button btnRules_;                // 主菜单页: 游戏规则
    TextBox rulesTitle_;             // 规则页标题
    TextBox rulesText_;              // 规则页正文(多行)
    Button btnRulesBack_;            // 规则页: 返回
    sf::FloatRect trackRect_;        // 滑轨几何(像素)
    float vol_ = 100.f;
    float savedVol_ = 100.f;         // 静音前音量(点 soundOff 恢复)
    bool dragging_ = false;
};
