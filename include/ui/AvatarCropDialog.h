#pragma once
#include <SFML/Graphics.hpp>
#include <functional>
#include <string>
#include "ui/Button.h"

// 打开方式: 在大厅点击本人头像圆(SceneLobby 里 Avatar::hitCircle 命中)。
// 流程: 系统"打开文件"对话框选图(comdlg32) -> 弹窗内拖动平移 / 滚轮缩放 ->
//       方形裁剪框内所见即所得 -> [确定] 输出 100x100 的 game_data/avatar.png。
// 用法:
//   AvatarCropDialog dlg;
//   dlg.setOnSaved([]{ AssetManager::instance().reloadCustomAvatar(); });
//   dlg.open();                                  // 选图并进入裁剪; 用户取消选择则不会打开
//   事件循环: if (dlg.isOpen()) { dlg.handleEvent(e, win); return; }   // 独占输入
//   dlg.draw(win);
class AvatarCropDialog {
public:
    static constexpr unsigned SAVE_SIZE = 100;   // 输出头像尺寸(与 assets/ui/avatars 一致)
    static constexpr float RT_SIZE = 340.f;      // 裁剪框边长(屏幕像素 = 预览分辨率)

    AvatarCropDialog();
    // owner: 游戏窗口句柄(必须传入! 否则原生文件框可能开在窗口后面且模态阻塞 -> 表现为"点了没反应")
    void open(sf::WindowHandle owner = nullptr);   // 选图 + 打开裁剪界面
    bool isOpen() const { return open_; }
    void setOnSaved(std::function<void()> cb) { onSaved_ = std::move(cb); }

    void handleEvent(const sf::Event& e, const sf::RenderWindow& win);
    void draw(sf::RenderWindow& win);

private:
    int  pickFile();                             // 系统选图并解码: 0=用户取消 1=成功 2=加载失败
    void resetView();                            // 初始视图: 图片刚好覆盖裁剪框并居中
    void renderPreview();                        // 按当前缩放/平移刷新裁剪预览
    void clampView();                            // 图像始终覆盖裁剪框
    sf::FloatRect cropRect() const;              // 裁剪框(屏幕坐标)
    sf::IntRect srcRect() const;                 // 裁剪框对应的图像像素区域
    void confirm();                               // 裁剪 -> 保存 PNG -> 回调

    bool open_ = false;
    bool hasImage_ = false;
    sf::WindowHandle owner_ = nullptr;           // 游戏窗口句柄(文件对话框属主)
    sf::Image img_;
    sf::Texture tex_;
    sf::RenderTexture preview_;
    float zoom_ = 1.f;
    float minZoom_ = 1.f;
    sf::Vector2f pos_{0.f, 0.f};                 // 图像左上角(屏幕坐标)
    bool dragging_ = false;
    sf::Vector2f dragLast_{0.f, 0.f};

    sf::RectangleShape overlay_, panel_, border_;
    sf::Text title_, hint_, errText_;
    Button btnPick_, btnOk_, btnCancel_;
    std::function<void()> onSaved_;
};
