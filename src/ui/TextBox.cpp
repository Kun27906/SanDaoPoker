#include "ui/TextBox.h"
#include "ui/FontUtil.h"

TextBox::TextBox(const std::string& text, const sf::Vector2f& pos, unsigned size) {
    setText(text);
    setPosition(pos);
    setCharacterSize(size);
}

void TextBox::setText(const std::string& t) {
    text_.setFont(font_util::defaultFont());
    // 中文必须显式 UTF-8 转换,否则 SFML 按 ANSI 解码会乱码
    text_.setString(str_util::utf8(t.c_str()));
}

void TextBox::setPosition(const sf::Vector2f& p) {
    text_.setPosition(p);
}

void TextBox::setCharacterSize(unsigned size) {
    text_.setCharacterSize(size);
}

void TextBox::setColor(const sf::Color& c) {
    text_.setFillColor(c);
}

void TextBox::centerOrigin() {
    sf::FloatRect b = text_.getLocalBounds();
    text_.setOrigin(b.left + b.width / 2.f, b.top + b.height / 2.f);
}

void TextBox::draw(sf::RenderWindow& win) const {
    win.draw(text_);
}
