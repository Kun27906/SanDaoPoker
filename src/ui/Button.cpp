#include "ui/Button.h"
#include "ui/FontUtil.h"
#include "render/AssetManager.h"
#include "render/SoundManager.h"

Button::Button(const std::string& text, const sf::Vector2f& pos, const sf::Vector2f& size) {
    setPosition(pos);
    setSize(size);
    setText(text);
}

void Button::setText(const std::string& t) {
    text_.setFont(font_util::defaultFont());
    // 中文必须显式 UTF-8 转换,否则 SFML 按 ANSI 解码会乱码
    text_.setString(str_util::utf8(t.c_str()));
    text_.setCharacterSize(22);
    text_.setFillColor(sf::Color::White);
    centerText();
}

void Button::setPosition(const sf::Vector2f& p) {
    pos_ = p;
    refreshSprite();
    centerText();
}

void Button::setSize(const sf::Vector2f& s) {
    size_ = s;
    refreshSprite();
    centerText();
}

void Button::setCharacterSize(unsigned size) {
    text_.setCharacterSize(size);
    centerText();
}

void Button::setCallback(std::function<void()> cb) {
    callback_ = std::move(cb);
}

void Button::setSelected(bool s) { selected_ = s; }
void Button::setDisabled(bool d) { disabled_ = d; }
void Button::setTint(sf::Color c) { tint_ = c; }

bool Button::contains(const sf::Vector2f& point) const {
    return point.x >= pos_.x && point.x <= pos_.x + size_.x &&
           point.y >= pos_.y && point.y <= pos_.y + size_.y;
}

void Button::handleEvent(const sf::Event& e, const sf::RenderWindow& win) {
    // 鼠标位置
    sf::Vector2f mpos = win.mapPixelToCoords(sf::Mouse::getPosition(win));
    hovered_ = contains(mpos);

    // 左键按下瞬间触发回调
    if (e.type == sf::Event::MouseButtonPressed &&
        e.mouseButton.button == sf::Mouse::Left && hovered_) {
        SoundManager::instance().playClick();  // 按钮点击音
        if (callback_) {
            callback_();
        }
    }
}

void Button::draw(sf::RenderWindow& win) {
    refreshSprite();
    win.draw(sprite_);
    win.draw(text_);
}

// 状态 -> 贴图: 禁用 > 选中 > 悬停 > 正常
void Button::refreshSprite() {
    const AssetManager& am = AssetManager::instance();
    int state = 0;                  // 0=正常
    if (disabled_)      state = 3;  // 禁用
    else if (selected_) state = 2;  // 选中: 常驻"按下"贴图, 不恢复
    else if (hovered_)  state = 1;  // 悬停
    const sf::Texture* t = am.buttonTexture(state);
    if (!t) return;                 // 贴图缺失: 保持上次状态
    sprite_.setTexture(*t);
    sprite_.setColor(tint_);
    const sf::Vector2u ts = t->getSize();
    if (ts.x == 0 || ts.y == 0) return;
    sprite_.setScale(size_.x / static_cast<float>(ts.x),
                     size_.y / static_cast<float>(ts.y));
    sprite_.setPosition(pos_);
}

void Button::centerText() {
    // 以按钮中心为锚点居中文字
    sf::FloatRect tb = text_.getLocalBounds();
    text_.setOrigin(tb.left + tb.width / 2.f, tb.top + tb.height / 2.f);
    text_.setPosition(pos_.x + size_.x / 2.f, pos_.y + size_.y / 2.f);
}
