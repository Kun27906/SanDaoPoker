#include "ui/VersionBadge.h"
#include "ui/FontUtil.h"
#include "render/AssetManager.h"
#include "render/SoundManager.h"
#include "core/VersionInfo.h"
#include <algorithm>

namespace {
constexpr float WW = 1280.f;
constexpr float WH = 800.f;

constexpr float PW = 800.f;              // 弹窗宽
constexpr float PH = 470.f;              // 弹窗高
constexpr float PL = (WW - PW) / 2.f;
constexpr float PT = (WH - PH) / 2.f;
constexpr float ROW_H = 34.f;            // 每条版本行高

const sf::Color C_BG(30, 40, 70);
const sf::Color C_EDGE(255, 215, 0);
const sf::Color C_LIST_BG(20, 26, 46);
const sf::Color C_VER(255, 215, 120);
const sf::Color C_DESC(225, 230, 240);

void centerText(sf::Text& t, float cx, float y) {
    sf::FloatRect b = t.getLocalBounds();
    t.setPosition(cx - b.width / 2.f - b.left, y);
}
}

VersionBadge::VersionBadge(bool clickable) : clickable_(clickable) {
    badge_.setFont(font_util::defaultFont());
    badge_.setCharacterSize(20);          // 版本号字体(略大)
    badge_.setFillColor(sf::Color(185, 185, 185));
    badge_.setString(str_util::utf8(GAME_VERSION));

    // ---- 弹窗 ----
    overlay_.setSize(sf::Vector2f(WW, WH));
    overlay_.setFillColor(sf::Color(0, 0, 0, 170));
    panel_.setSize(sf::Vector2f(PW, PH));
    panel_.setPosition(sf::Vector2f(PL, PT));
    panel_.setFillColor(C_BG);
    panel_.setOutlineColor(C_EDGE);
    panel_.setOutlineThickness(3.f);

    title_.setFont(font_util::defaultFont());
    title_.setCharacterSize(28);
    title_.setFillColor(sf::Color(255, 220, 130));
    title_.setString(str_util::utf8("版本历史"));
    centerText(title_, WW / 2.f, PT + 24.f);

    // 关闭键: 圆圈 + close 图标
    closeRing_.setRadius(22.f);
    closeRing_.setPosition(PL + PW - 66.f, PT + 22.f);
    closeRing_.setFillColor(sf::Color(255, 215, 0, 30));
    closeRing_.setOutlineColor(C_EDGE);
    closeRing_.setOutlineThickness(3.f);
    closeRect_ = sf::FloatRect(PL + PW - 66.f, PT + 22.f, 44.f, 44.f);
    if (const sf::Texture* ic = AssetManager::instance().icon("close")) {
        float s = 30.f / static_cast<float>(ic->getSize().x);
        closeIcon_.setTexture(*ic);
        closeIcon_.setScale(s, s);
        closeIcon_.setPosition(PL + PW - 66.f + 7.f, PT + 22.f + 7.f);
    }

    // 列表区(去掉提示行后上移, 可视行数更多)
    listRect_ = sf::FloatRect(PL + 30.f, PT + 78.f, PW - 60.f, PH - 78.f - 28.f);
    listBg_.setSize(sf::Vector2f(listRect_.width, listRect_.height));
    listBg_.setPosition(listRect_.left, listRect_.top);
    listBg_.setFillColor(C_LIST_BG);
    listBg_.setOutlineColor(sf::Color(90, 105, 140));
    listBg_.setOutlineThickness(1.f);

    buildEntries();

    listView_.setSize(listRect_.width, listRect_.height);
    listView_.setCenter(listRect_.left + listRect_.width / 2.f,
                        listRect_.top + listRect_.height / 2.f);
    listView_.setViewport(sf::FloatRect(listRect_.left / WW, listRect_.top / WH,
                                        listRect_.width / WW, listRect_.height / WH));
}

void VersionBadge::buildEntries() {
    verTexts_.clear();
    descTexts_.clear();
    for (int i = 0; i < VERSION_HISTORY_COUNT; i++) {
        sf::Text v;
        v.setFont(font_util::defaultFont());
        v.setCharacterSize(17);
        v.setFillColor(C_VER);
        v.setString(str_util::utf8(VERSION_HISTORY[i].ver));

        sf::Text d;
        d.setFont(font_util::defaultFont());
        d.setCharacterSize(17);
        d.setFillColor(C_DESC);
        d.setString(str_util::utf8(VERSION_HISTORY[i].desc));

        verTexts_.push_back(std::move(v));
        descTexts_.push_back(std::move(d));
    }
    contentH_ = VERSION_HISTORY_COUNT * ROW_H;
}

float VersionBadge::maxScroll() const {
    float m = contentH_ - listRect_.height;
    return m > 0.f ? m : 0.f;
}

void VersionBadge::clampScroll() {
    float m = maxScroll();
    if (scroll_ < 0.f) scroll_ = 0.f;
    if (scroll_ > m) scroll_ = m;
}

void VersionBadge::setPosition(const sf::Vector2f& p) {
    badge_.setPosition(p);
    sf::FloatRect b = badge_.getLocalBounds();
    badgeRect_ = sf::FloatRect(p.x, p.y, b.width + 8.f, b.height + 8.f);
}

bool VersionBadge::handleEvent(const sf::Event& e, const sf::RenderWindow& win) {
    if (open_) {
        // 弹窗打开: 吞掉全部事件
        if (e.type == sf::Event::MouseWheelScrolled) {
            scroll_ -= e.mouseWheelScroll.delta * (ROW_H * 1.2f);
            clampScroll();
        } else if (e.type == sf::Event::MouseButtonPressed &&
                   e.mouseButton.button == sf::Mouse::Left) {
            sf::Vector2f mp = win.mapPixelToCoords(
                sf::Vector2i(e.mouseButton.x, e.mouseButton.y));
            if (closeRect_.contains(mp)) {
                open_ = false;
                SoundManager::instance().playClick();
            }
        }
        return true;
    }

    if (!clickable_) return false;

    if (e.type == sf::Event::MouseMoved) {
        sf::Vector2f mp = win.mapPixelToCoords(sf::Vector2i(e.mouseMove.x, e.mouseMove.y));
        hovered_ = badgeRect_.contains(mp);
    } else if (e.type == sf::Event::MouseButtonPressed &&
               e.mouseButton.button == sf::Mouse::Left) {
        sf::Vector2f mp = win.mapPixelToCoords(
            sf::Vector2i(e.mouseButton.x, e.mouseButton.y));
        if (badgeRect_.contains(mp)) {
            open_ = true;
            scroll_ = maxScroll();          // 打开时滚到最新版本
            SoundManager::instance().playClick();
        }
    }
    return false;
}

void VersionBadge::draw(sf::RenderWindow& win) {
    // 左下角版本号(hover 提亮)
    badge_.setFillColor(hovered_ ? sf::Color(240, 240, 240) : sf::Color(185, 185, 185));
    win.draw(badge_);
    if (!open_) return;

    // ---- 弹窗 ----
    win.draw(overlay_);
    win.draw(panel_);
    win.draw(title_);
    win.draw(closeRing_);
    if (!closeIcon_.getTexture()) {          // 惰性获取(场景可能在素材加载前构造)
        if (const sf::Texture* ic = AssetManager::instance().icon("close")) {
            float s = 30.f / static_cast<float>(ic->getSize().x);
            closeIcon_.setTexture(*ic);
            closeIcon_.setScale(s, s);
            closeIcon_.setPosition(PL + PW - 66.f + 7.f, PT + 22.f + 7.f);
        }
    }
    if (closeIcon_.getTexture()) win.draw(closeIcon_);
    win.draw(listBg_);

    // ---- 列表(子视图裁剪 + 滚轮滚动) ----
    sf::View prev = win.getView();
    win.setView(listView_);
    const float xVer = listRect_.left + 16.f;
    const float xDesc = listRect_.left + 112.f;
    for (int i = 0; i < VERSION_HISTORY_COUNT; i++) {
        float y = listRect_.top + i * ROW_H + 7.f - scroll_;
        verTexts_[i].setPosition(xVer, y);
        descTexts_[i].setPosition(xDesc, y);
        win.draw(verTexts_[i]);
        win.draw(descTexts_[i]);
    }
    win.setView(prev);

    // ---- 滚动条(内容超出时) ----
    float ms = maxScroll();
    if (ms > 0.f) {
        float trackH = listRect_.height - 8.f;
        float thumbH = std::max(28.f, trackH * listRect_.height / contentH_);
        float ty = listRect_.top + 4.f + (scroll_ / ms) * (trackH - thumbH);
        sf::RectangleShape track(sf::Vector2f(6.f, trackH));
        track.setPosition(listRect_.left + listRect_.width - 12.f, listRect_.top + 4.f);
        track.setFillColor(sf::Color(60, 72, 105));
        win.draw(track);
        sf::RectangleShape thumb(sf::Vector2f(6.f, thumbH));
        thumb.setPosition(listRect_.left + listRect_.width - 12.f, ty);
        thumb.setFillColor(sf::Color(150, 170, 220));
        win.draw(thumb);
    }
}
