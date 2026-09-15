#pragma once
#include <SFML/Graphics.hpp>
#include <functional>
#include <string>
#include "ui/Button.h"

// ====== 自设昵称弹窗(成员C) ======
// 打开方式: 在大厅点击本人昵称名牌(SceneLobby 里 Avatar::plateBounds 命中)。
// 允许字符: 中文 / 英文字母 / 下划线 / 数字; 最长 8 个字(名牌宽度可容纳)。
// 确定 -> 回调新昵称(由调用方写入 Account 并存档)。
// 用法:
//   NicknameDialog dlg;
//   dlg.setOnConfirm([](const std::string& n){ Account::instance().setNickname(n); });
//   dlg.open(cur);            // 打开并预填
//   事件循环: if (dlg.isOpen()) { dlg.handleEvent(e, win); return; }   // 独占输入
//   dlg.update(dt);  dlg.draw(win);
class NicknameDialog {
public:
    static constexpr int MAX_CHARS = 8;   // 最长字数(按字符数, 非字节)

    NicknameDialog();
    void open(const std::string& current);   // 打开并预填当前昵称
    bool isOpen() const { return open_; }
    void setOnConfirm(std::function<void(const std::string&)> cb) { onConfirm_ = std::move(cb); }

    void handleEvent(const sf::Event& e, const sf::RenderWindow& win);
    void update(float dt);
    void draw(sf::RenderWindow& win);

private:
    bool acceptCp(unsigned cp) const;   // 允许的字符集(中文/字母/下划线/数字)
    void refreshInput();                // 同步输入文字与插入符位置
    void commit();                      // 校验 -> 回调 -> 关闭

    bool open_ = false;
    std::string text_;                  // 输入内容(UTF-8)
    float caretT_ = 0.f;                // 插入符闪烁计时

    sf::RectangleShape overlay_, panel_, inputBox_, caret_;
    sf::Text title_, inputText_, hint_, warn_;
    Button btnOk_, btnCancel_;
    std::function<void(const std::string&)> onConfirm_;
};
