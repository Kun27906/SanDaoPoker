#include "render/GlobalHud.h"
#include "render/AssetManager.h"
#include "render/SoundManager.h"
#include "render/SceneManager.h"
#include "ui/FontUtil.h"

namespace {
constexpr float WW = 1280.f;
constexpr float WH = 800.f;
constexpr float BTN = 40.f;          // 左上角键尺寸
constexpr float PAD = 48.f;          // 键间距

// 弹窗几何
constexpr float PW = 680.f;
constexpr float PH = 440.f;
constexpr float PL = (WW - PW) / 2.f;   // 300
constexpr float PT = (WH - PH) / 2.f;   // 180

// 音量行几何(弹窗内)
constexpr float ROW_Y = PT + 150.f;          // 音量行中心 y
constexpr float VOL_X = PL + 64.f;           // soundSetting 图标中心 x
constexpr float TRACK_L = PL + 150.f;        // 滑轨左端
constexpr float TRACK_W = 410.f;             // 滑轨长度
constexpr float KNOB = 34.f;                 // slider 显示尺寸
}

GlobalHud::GlobalHud(SceneManager* mgr) : mgr_(mgr) {
    AssetManager& am = AssetManager::instance();

    // ---- 左上角四键 ----
    btnMenu_.setTexture(am.icon("menuList"));
    btnMenu_.setPosition(sf::Vector2f(12.f, 12.f));
    btnMenu_.setSize(BTN);
    btnMenu_.setCallback([this]() { openPopup(); });

    btnMusic_.setTexture(am.icon("musicOn"));
    btnMusic_.setPosition(sf::Vector2f(12.f + PAD, 12.f));
    btnMusic_.setSize(BTN);
    btnMusic_.setCallback([this]() {
        SoundManager::instance().toggleBgm();   // 只开关背景音乐,音效不受影响
        btnMusic_.setTexture(AssetManager::instance().icon(
            SoundManager::instance().bgmOn() ? "musicOn" : "musicOff"));
    });

    btnWrench_.setTexture(am.icon("wrench"));
    btnWrench_.setPosition(sf::Vector2f(12.f + PAD * 2.f, 12.f));
    btnWrench_.setSize(BTN);
    // wrench: 占位,后续接入具体功能(点击已有 click 反馈)

    btnHome_.setTexture(am.icon("home"));
    btnHome_.setPosition(sf::Vector2f(12.f + PAD * 3.f, 12.f));
    btnHome_.setSize(BTN);
    btnHome_.setCallback([this]() { if (mgr_) mgr_->onHomePressed(); });

    // ---- 弹窗 ----
    overlay_.setSize(sf::Vector2f(WW, WH));
    overlay_.setFillColor(sf::Color(0, 0, 0, 160));
    panel_.setSize(sf::Vector2f(PW, PH));
    panel_.setPosition(sf::Vector2f(PL, PT));
    panel_.setFillColor(sf::Color(30, 40, 70));
    panel_.setOutlineColor(sf::Color(255, 215, 0));
    panel_.setOutlineThickness(3.f);

    // 右上角关闭键(画圆: btnClose_ 下方衬一个圆底)
    btnClose_.setTexture(am.icon("close"));
    btnClose_.setSize(40.f);
    btnClose_.setPosition(sf::Vector2f(PL + PW - 60.f, PT + 20.f));
    btnClose_.setCallback([this]() { closePopup(); });

    // 音量行
    btnVolIcon_.setTexture(am.icon("soundSetting"));
    btnVolIcon_.setSize(46.f);
    btnVolIcon_.setPosition(sf::Vector2f(VOL_X - 23.f, ROW_Y - 23.f));

    trackRect_ = sf::FloatRect(TRACK_L, ROW_Y - 5.f, TRACK_W, 10.f);
    track_.setSize(sf::Vector2f(TRACK_W, 10.f));
    track_.setPosition(sf::Vector2f(TRACK_L, ROW_Y - 5.f));
    track_.setFillColor(sf::Color(46, 58, 96));
    track_.setOutlineColor(sf::Color(120, 140, 190));
    track_.setOutlineThickness(1.f);
    trackFill_.setSize(sf::Vector2f(0.f, 10.f));
    trackFill_.setPosition(sf::Vector2f(TRACK_L, ROW_Y - 5.f));
    trackFill_.setFillColor(sf::Color(255, 215, 0));

    knob_.setTexture(am.icon("slider"));
    knob_.setSize(KNOB);

    // 初始音量(默认 100,与系统音量 0~100 刻度一致)
    vol_ = static_cast<float>(SoundManager::instance().volume());
    updateKnob();
}

bool GlobalHud::handleEvent(const sf::Event& e, const sf::RenderWindow& win) {
    // home 键显隐随场景(组牌/比牌/启动页隐藏)
    if (mgr_) btnHome_.setVisible(mgr_->homeVisible());

    if (!popupOpen_) {
        // 常态: 四键 -> 场景
        btnMenu_.handleEvent(e, win);
        btnMusic_.handleEvent(e, win);
        btnWrench_.handleEvent(e, win);
        btnHome_.handleEvent(e, win);
        return false;
    }

    // ---- 弹窗打开: 拦截一切,只处理弹窗 ----
    if (e.type == sf::Event::MouseButtonPressed &&
        e.mouseButton.button == sf::Mouse::Left) {
        sf::Vector2f mp(static_cast<float>(e.mouseButton.x),
                        static_cast<float>(e.mouseButton.y));
        // 点击滑轨/滑块 -> 开始拖动(并跳转到该位置)
        float hw = KNOB / 2.f + 8.f;
        sf::FloatRect hit(trackRect_.left - hw, ROW_Y - hw, trackRect_.width + hw * 2.f, hw * 2.f);
        if (hit.contains(mp)) {
            dragging_ = true;
            setVolumeFromMouse(mp.x);
        }
    } else if (e.type == sf::Event::MouseButtonReleased &&
               e.mouseButton.button == sf::Mouse::Left) {
        dragging_ = false;
    } else if (e.type == sf::Event::MouseMoved && dragging_) {
        setVolumeFromMouse(static_cast<float>(e.mouseMove.x));
    }
    // 音量图标视觉随音量切换(0 -> soundOff)
    AssetManager& am = AssetManager::instance();
    btnVolIcon_.setTexture(am.icon(vol_ <= 0.f ? "soundOff" : "soundSetting"));

    btnClose_.handleEvent(e, win);
    return true;   // 事件已消耗
}

void GlobalHud::draw(sf::RenderWindow& win) {
    if (mgr_) btnHome_.setVisible(mgr_->homeVisible());
    btnMenu_.draw(win);
    btnMusic_.draw(win);
    btnWrench_.draw(win);
    btnHome_.draw(win);
    if (!popupOpen_) return;

    // 弹窗层
    win.draw(overlay_);
    win.draw(panel_);

    // 右上角圆圈 + 关闭
    sf::CircleShape ring(24.f);
    ring.setPosition(sf::Vector2f(PL + PW - 60.f + 20.f - 24.f,
                                  PT + 20.f + 20.f - 24.f));
    ring.setFillColor(sf::Color(255, 215, 0, 30));
    ring.setOutlineColor(sf::Color(255, 215, 0));
    ring.setOutlineThickness(3.f);
    win.draw(ring);
    btnClose_.draw(win);

    // 标题
    TextBox ttl("菜单", sf::Vector2f(WW / 2.f, PT + 42.f), 28);
    ttl.setColor(sf::Color(255, 220, 130));
    ttl.centerOrigin();
    ttl.draw(win);

    // 音量行: 图标(0 音量 -> soundOff) + 滑轨(金色已填充) + slider 滑块
    btnVolIcon_.setTexture(AssetManager::instance().icon(
        vol_ <= 0.f ? "soundOff" : "soundSetting"));
    btnVolIcon_.draw(win);
    win.draw(track_);
    trackFill_.setSize(sf::Vector2f(vol_ / 100.f * TRACK_W, 10.f));
    win.draw(trackFill_);
    knob_.draw(win);

    // 说明文字
    TextBox tip("拖动滑块调节所有声音大小", sf::Vector2f(TRACK_L + TRACK_W / 2.f, ROW_Y + 70.f), 18);
    tip.setColor(sf::Color(215, 215, 215));
    tip.centerOrigin();
    tip.draw(win);
}

void GlobalHud::openPopup() {
    popupOpen_ = true;
}

void GlobalHud::closePopup() {
    popupOpen_ = false;
    dragging_ = false;
}

void GlobalHud::updateKnob() {
    float x = TRACK_L + vol_ / 100.f * TRACK_W - KNOB / 2.f;
    knob_.setPosition(sf::Vector2f(x, ROW_Y - KNOB / 2.f));
}

void GlobalHud::setVolumeFromMouse(float mx) {
    float t = (mx - TRACK_L) / TRACK_W;
    if (t < 0.f) t = 0.f;
    if (t > 1.f) t = 1.f;
    vol_ = t * 100.f;
    SoundManager::instance().setVolume(static_cast<int>(vol_));
    updateKnob();
}
