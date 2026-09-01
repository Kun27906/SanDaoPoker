#include "ui/Button.h"
#include "ui/FontUtil.h"

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
    rect_.setPosition(p);
    centerText();
}

void Button::setSize(const sf::Vector2f& s) {
    rect_.setSize(s);
    centerText();
}

void Button::setCharacterSize(unsigned size) {
    text_.setCharacterSize(size);
    centerText();
}

void Button::setCallback(std::function<void()> cb) {
    callback_ = std::move(cb);
}

void Button::setColors(sf::Color normal, sf::Color hover, sf::Color pressed) {
    normal_ = normal;
    hover_ = hover;
    pressed_ = pressed;
}

bool Button::contains(const sf::Vector2f& point) const {
    return rect_.getGlobalBounds().contains(point);
}

void Button::handleEvent(const sf::Event& e, const sf::RenderWindow& win) {
    // 鼠标位置(窗口坐标 -> 世界坐标)
    sf::Vector2f mpos = win.mapPixelToCoords(sf::Mouse::getPosition(win));
    hovered_ = contains(mpos);

    // 左键按下瞬间触发回调
    if (e.type == sf::Event::MouseButtonPressed &&
        e.mouseButton.button == sf::Mouse::Left && hovered_) {
        if (callback_) {
            callback_();
        }
    }
}

void Button::draw(sf::RenderWindow& win) {
    // 三态颜色
    sf::Color c = normal_;
    if (hovered_) c = hover_;
    rect_.setFillColor(c);
    win.draw(rect_);
    win.draw(text_);
}

void Button::centerText() {
    // 以按钮中心为锚点居中文字
    sf::FloatRect tb = text_.getLocalBounds();
    sf::Vector2f pos = rect_.getPosition();
    sf::Vector2f size = rect_.getSize();
    text_.setOrigin(tb.left + tb.width / 2.f, tb.top + tb.height / 2.f);
    text_.setPosition(pos.x + size.x / 2.f, pos.y + size.y / 2.f);
}
