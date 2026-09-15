#pragma once
#include <SFML/Graphics.hpp>
#include <functional>
#include <string>
#include "ui/Button.h"

// 打开方式: 在大厅点击本人昵称名牌。
// 允许字符: 中文 / 英文字母 / 下划线 / 数字; 最长 8 个字。
// 确定 -> 回调新昵称。
// 用法:
// NicknameDialog dlg;
// dlg.setOnConfirm{ Account::instance.setNickname; });
// dlg.open; // 打开并预填
// 事件循环: if ) { dlg.handleEvent; return; } // 独占输入
// dlg.update; dlg.draw;
class NicknameDialog {
public:
    static constexpr int MAX_CHARS = 8;   // 最长字数

    NicknameDialog();
    void open(const std::string& current);   // 打开并预填当前昵称
    bool isOpen() const { return open_; }
    void setOnConfirm(std::function<void(const std::string&)> cb) { onConfirm_ = std::move(cb); }

    void handleEvent(const sf::Event& e, const sf::RenderWindow& win);
    void update(float dt);
    void draw(sf::RenderWindow& win);

private:
    bool acceptCp(unsigned cp) const;   // 允许的字符集
    void refreshInput();                // 同步输入文字与插入符位置
    void commit();                      // 校验 -> 回调 -> 关闭

    bool open_ = false;
    std::string text_;                  // 输入内容
    float caretT_ = 0.f;                // 插入符闪烁计时

    sf::RectangleShape overlay_, panel_, inputBox_, caret_;
    sf::Text title_, inputText_, hint_, warn_;
    Button btnOk_, btnCancel_;
    std::function<void(const std::string&)> onConfirm_;
};
