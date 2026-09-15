#pragma once
#include <SFML/Graphics.hpp>
#include <functional>
#include <string>
#include "ui/Button.h"

// 大厅点本人昵称名牌打开; 限中文/字母/下划线/数字, 最长 8 字
class NicknameDialog {
public:
    static constexpr int MAX_CHARS = 8;

    NicknameDialog();
    void open(const std::string& current);
    bool isOpen() const { return open_; }
    void setOnConfirm(std::function<void(const std::string&)> cb) { onConfirm_ = std::move(cb); }

    void handleEvent(const sf::Event& e, const sf::RenderWindow& win);
    void update(float dt);
    void draw(sf::RenderWindow& win);

private:
    bool acceptCp(unsigned cp) const;
    void refreshInput();
    void commit();

    bool open_ = false;
    std::string text_;
    float caretT_ = 0.f;

    sf::RectangleShape overlay_, panel_, inputBox_, caret_;
    sf::Text title_, inputText_, hint_, warn_;
    Button btnOk_, btnCancel_;
    std::function<void(const std::string&)> onConfirm_;
};
