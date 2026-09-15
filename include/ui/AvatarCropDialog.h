#pragma once
#include <SFML/Graphics.hpp>
#include <functional>
#include <string>
#include "ui/Button.h"

// 大厅点头像打开; 裁剪框内所见即所得, 输出 100x100 头像
class AvatarCropDialog {
public:
    static constexpr unsigned SAVE_SIZE = 100;
    static constexpr float RT_SIZE = 340.f;

    AvatarCropDialog();
    // owner 为游戏窗口句柄
    void open(sf::WindowHandle owner = nullptr);
    bool isOpen() const { return open_; }
    void setOnSaved(std::function<void()> cb) { onSaved_ = std::move(cb); }

    void handleEvent(const sf::Event& e, const sf::RenderWindow& win);
    void draw(sf::RenderWindow& win);

private:
    int  pickFile();          // 0 取消 1 成功 2 加载失败
    void resetView();
    void renderPreview();
    void clampView();
    sf::FloatRect cropRect() const;
    sf::IntRect srcRect() const;
    void confirm();

    bool open_ = false;
    bool hasImage_ = false;
    sf::WindowHandle owner_ = nullptr;
    sf::Image img_;
    sf::Texture tex_;
    sf::RenderTexture preview_;
    float zoom_ = 1.f;
    float minZoom_ = 1.f;
    sf::Vector2f pos_{0.f, 0.f};
    bool dragging_ = false;
    sf::Vector2f dragLast_{0.f, 0.f};

    sf::RectangleShape overlay_, panel_, border_;
    sf::Text title_, hint_, errText_;
    Button btnPick_, btnOk_, btnCancel_;
    std::function<void()> onSaved_;
};
