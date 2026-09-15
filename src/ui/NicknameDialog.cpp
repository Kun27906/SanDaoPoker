#include "ui/NicknameDialog.h"
#include "render/Layout.h"
#include "ui/FontUtil.h"
#include "ui/PanelFrame.h"
#include <algorithm>
#include <cmath>

namespace {
constexpr float WW = static_cast<float>(layout::WINDOW_W);
constexpr float WH = static_cast<float>(layout::WINDOW_H);
constexpr float PW = 700.f;  // 弹窗宽
constexpr float PH = 320.f;  // 弹窗高
const sf::Color C_PANEL(28, 36, 62);
const sf::Color C_GOLD(255, 215, 0);
const sf::Color C_HINT(200, 205, 225);

// 统计 UTF-8 字符数
int utf8Count(const std::string& s) {
    int n = 0;
    for (unsigned char c: s) {
        if ((c & 0xC0) != 0x80) n++;
    }
    return n;
}

// 追加一个 Unicode 码点为 UTF-8
void utf8Append(std::string& s, unsigned cp) {
    if (cp < 0x80) {
        s.push_back(static_cast<char>(cp));
    } else if (cp < 0x800) {
        s.push_back(static_cast<char>(0xC0 | (cp >> 6)));
        s.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
    } else if (cp < 0x10000) {
        s.push_back(static_cast<char>(0xE0 | (cp >> 12)));
        s.push_back(static_cast<char>(0x80 | ((cp >> 6) & 0x3F)));
        s.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
    } else {
        s.push_back(static_cast<char>(0xF0 | (cp >> 18)));
        s.push_back(static_cast<char>(0x80 | ((cp >> 12) & 0x3F)));
        s.push_back(static_cast<char>(0x80 | ((cp >> 6) & 0x3F)));
        s.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
    }
}

// 删除最后一个 UTF-8 字符
void utf8PopBack(std::string& s) {
    while (!s.empty() && (static_cast<unsigned char>(s.back()) & 0xC0) == 0x80) {
        s.pop_back();
    }
    if (!s.empty()) s.pop_back();
}
}

NicknameDialog::NicknameDialog() {
    const sf::Font& font = font_util::defaultFont();

    overlay_.setSize(sf::Vector2f(WW, WH));
    overlay_.setFillColor(sf::Color(0, 0, 0, 170));

    const float px = (WW - PW) / 2.f, py = (WH - PH) / 2.f;
    panel_.setSize(sf::Vector2f(PW, PH));
    panel_.setPosition(sf::Vector2f(px, py));
    panel_.setFillColor(C_PANEL);
    panel_.setOutlineColor(C_GOLD);
    panel_.setOutlineThickness(3.f);

    title_.setFont(font);
    title_.setString(str_util::utf8("设置昵称"));
    title_.setCharacterSize(32);
    title_.setFillColor(C_GOLD);
    title_.setPosition(sf::Vector2f(px + 40.f, py + 28.f));

    inputBox_.setSize(sf::Vector2f(PW - 80.f, 64.f));
    inputBox_.setPosition(sf::Vector2f(px + 40.f, py + 92.f));
    inputBox_.setFillColor(sf::Color(16, 22, 40));
    inputBox_.setOutlineColor(sf::Color(150, 170, 220));
    inputBox_.setOutlineThickness(2.f);

    inputText_.setFont(font);
    inputText_.setCharacterSize(28);
    inputText_.setFillColor(sf::Color::White);
    inputText_.setPosition(sf::Vector2f(px + 52.f, py + 110.f));

    caret_.setSize(sf::Vector2f(2.f, 30.f));
    caret_.setFillColor(sf::Color(255, 235, 150));

    hint_.setFont(font);
    hint_.setString(str_util::utf8("可用: 中文 · 英文字母 · 下划线 · 数字    最长 8 个字"));
    hint_.setCharacterSize(20);
    hint_.setFillColor(C_HINT);
    hint_.setPosition(sf::Vector2f(px + 40.f, py + 176.f));

    warn_.setFont(font);
    warn_.setCharacterSize(20);
    warn_.setFillColor(sf::Color(255, 150, 150));
    warn_.setPosition(sf::Vector2f(px + 40.f, py + 206.f));

    btnOk_.setText("确定");
    btnOk_.setPosition(sf::Vector2f(px + PW - 40.f - 180.f - 20.f - 180.f, py + PH - 76.f));
    btnOk_.setSize(sf::Vector2f(180.f, 52.f));
    btnOk_.setCallback([this]() { commit(); });

    btnCancel_.setText("取消");
    btnCancel_.setPosition(sf::Vector2f(px + PW - 40.f - 180.f, py + PH - 76.f));
    btnCancel_.setSize(sf::Vector2f(180.f, 52.f));
    btnCancel_.setCallback([this]() { open_ = false; });
}

void NicknameDialog::open(const std::string& current) {
    open_ = true;
    text_ = current;
    caretT_ = 0.f;
    warn_.setString(sf::String());
    refreshInput();
}

bool NicknameDialog::acceptCp(unsigned cp) const {
    if (cp >= '0' && cp <= '9') return true;
    if (cp >= 'A' && cp <= 'Z') return true;
    if (cp >= 'a' && cp <= 'z') return true;
    if (cp == '_') return true;
    if (cp >= 0x4E00 && cp <= 0x9FFF) return true;  // CJK 统一表意文字
    return false;
}

void NicknameDialog::refreshInput() {
    inputText_.setString(str_util::utf8(text_.c_str()));
    sf::FloatRect b = inputText_.getLocalBounds();
    caret_.setPosition(sf::Vector2f(inputText_.getPosition().x + b.width + 4.f,
                                    inputText_.getPosition().y + 2.f));
}

void NicknameDialog::commit() {
    if (text_.empty()) {
        warn_.setString(str_util::utf8("昵称不能为空"));
        return;
    }
    if (onConfirm_) onConfirm_(text_);
    open_ = false;
}

void NicknameDialog::handleEvent(const sf::Event& e, const sf::RenderWindow& win) {
    if (!open_) return;

    if (e.type == sf::Event::TextEntered) {
        const unsigned cp = e.text.unicode;
        if (cp == 13) {  // 回车 = 确定
            commit();
        } else if (cp == 27) {  // Esc = 取消
            open_ = false;
        } else if (cp == 8) {  // 退格
            utf8PopBack(text_);
            warn_.setString(sf::String());
            refreshInput();
        } else if (cp >= 32) {
            if (!acceptCp(cp)) {
                warn_.setString(str_util::utf8("仅支持中文、英文字母、下划线和数字"));
            } else if (utf8Count(text_) >= MAX_CHARS) {
                warn_.setString(str_util::utf8("最多 8 个字"));
            } else {
                utf8Append(text_, cp);
                warn_.setString(sf::String());
                refreshInput();
            }
        }
        return;
    }

    btnOk_.handleEvent(e, win);
    btnCancel_.handleEvent(e, win);
}

void NicknameDialog::update(float dt) {
    if (!open_) return;
    caretT_ += dt;
}

void NicknameDialog::draw(sf::RenderWindow& win) {
    if (!open_) return;
    win.draw(overlay_);
    win.draw(panel_);
    panel_frame::draw(win, sf::FloatRect(panel_.getPosition(), panel_.getSize()));  // 装饰边框
    win.draw(title_);
    win.draw(inputBox_);
    win.draw(inputText_);
    if (std::fmod(caretT_, 1.0f) < 0.55f) win.draw(caret_);  // 插入符闪烁
    win.draw(hint_);
    win.draw(warn_);
    btnOk_.draw(win);
    btnCancel_.draw(win);
}
