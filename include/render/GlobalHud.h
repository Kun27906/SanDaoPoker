#pragma once
#include <SFML/Graphics.hpp>
#include "ui/IconButton.h"
#include "ui/TextBox.h"

// ====== GlobalHud 全局悬浮工具栏(成员C, 阶段9) ======
// 所有界面左上角常驻: [menuList] [music] [wrench] [home](按场景显隐)
//   - menuList: 打开游戏内大弹窗(右上角圆圈+close 关闭);
//               弹窗内第一个功能行: 音量(soundSetting 图标 + 滑动条,
//               slider 滑块左右拖动调 0~100 全局音量, 最左端图标变 soundOff)
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

    SceneManager* mgr_;
    IconButton btnMenu_, btnMusic_, btnWrench_, btnHome_;
    bool popupOpen_ = false;

    // 弹窗部件
    sf::RectangleShape overlay_;
    sf::RectangleShape panel_;
    sf::RectangleShape track_;       // 音量滑轨(底)
    sf::RectangleShape trackFill_;   // 音量滑轨(金色已填充部分)
    IconButton btnClose_;            // 右上角关闭(圆内)
    IconButton btnVolIcon_;          // soundSetting / soundOff 图标(随音量切换,不响应点击)
    IconButton knob_;                // slider 指示点(可拖动)
    sf::FloatRect trackRect_;        // 滑轨几何(像素)
    float vol_ = 100.f;
    bool dragging_ = false;
};
