#include "ui/AvatarCropDialog.h"
#include "render/Layout.h"
#include "ui/FontUtil.h"
#include "ui/PanelFrame.h"
#include <algorithm>
#include <cmath>
#include <fstream>
#include <iterator>
#include <vector>

// WIN32_LEAN_AND_MEAN / NOMINMAX: 避免引入 winsock 与 min/max 宏
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <commdlg.h>

namespace {
constexpr float WW = static_cast<float>(layout::WINDOW_W);
constexpr float WH = static_cast<float>(layout::WINDOW_H);
constexpr float PW = 760.f;                 // 弹窗宽
constexpr float PH = 620.f;                 // 弹窗高
constexpr float SQUARE_TOP = 180.f;         // 裁剪框顶边
const sf::Color C_PANEL(28, 36, 62);
const sf::Color C_GOLD(255, 215, 0);
constexpr const char* AVATAR_OUT = "game_data/avatar.png";

// UTF-8 -> 宽字符
std::wstring toWide(const std::string& u8) {
    if (u8.empty()) return std::wstring();
    int need = MultiByteToWideChar(CP_UTF8, 0, u8.c_str(), -1, nullptr, 0);
    if (need <= 0) return std::wstring();
    std::wstring w(static_cast<size_t>(need - 1), L'\0');
    MultiByteToWideChar(CP_UTF8, 0, u8.c_str(), -1, &w[0], need);
    return w;
}

bool pickImageFile(std::string& outPath, sf::WindowHandle owner) {
    wchar_t buf[MAX_PATH * 4] = L"";
    OPENFILENAMEW ofn;
    std::memset(&ofn, 0, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = static_cast<HWND>(owner);      // 属主 = 游戏窗口
    ofn.lpstrFilter = L"图片文件 (*.png;*.jpg;*.jpeg;*.bmp)\0*.png;*.jpg;*.jpeg;*.bmp\0所有文件 (*.*)\0*.*\0\0";
    ofn.lpstrFile = buf;
    ofn.nMaxFile = MAX_PATH * 4;
    ofn.lpstrTitle = L"选择头像图片";
    // OFN_NOCHANGEDIR: 不改变进程当前目录
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR;
    if (!GetOpenFileNameW(&ofn)) return false;

    int need = WideCharToMultiByte(CP_UTF8, 0, buf, -1, nullptr, 0, nullptr, nullptr);
    if (need <= 1) return false;
    std::string s(static_cast<size_t>(need - 1), '\0');
    WideCharToMultiByte(CP_UTF8, 0, buf, -1, &s[0], need, nullptr, nullptr);
    outPath = s;
    return true;
}

// 读图: 先按宽路径读入内存, 再交给 SFML 解码
bool loadImageFile(const std::string& u8path, sf::Image& out) {
    std::wstring w = toWide(u8path);
    if (w.empty()) return false;
    std::ifstream in(w.c_str(), std::ios::binary);
    if (!in) return false;
    std::vector<char> buf((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    if (buf.empty()) return false;
    return out.loadFromMemory(buf.data(), buf.size());
}
}   // namespace

AvatarCropDialog::AvatarCropDialog() {
    const sf::Font& font = font_util::defaultFont();

    overlay_.setSize(sf::Vector2f(WW, WH));
    overlay_.setFillColor(sf::Color(0, 0, 0, 180));

    const float px = (WW - PW) / 2.f, py = (WH - PH) / 2.f;
    panel_.setSize(sf::Vector2f(PW, PH));
    panel_.setPosition(sf::Vector2f(px, py));
    panel_.setFillColor(C_PANEL);
    panel_.setOutlineColor(C_GOLD);
    panel_.setOutlineThickness(3.f);

    title_.setFont(font);
    title_.setString(str_util::utf8("设置头像"));
    title_.setCharacterSize(30);
    title_.setFillColor(C_GOLD);
    title_.setPosition(sf::Vector2f(px + 36.f, py + 22.f));

    const sf::FloatRect cr = cropRect();
    border_.setSize(sf::Vector2f(cr.width, cr.height));
    border_.setPosition(sf::Vector2f(cr.left, cr.top));
    border_.setFillColor(sf::Color::Transparent);
    border_.setOutlineColor(C_GOLD);
    border_.setOutlineThickness(3.f);

    hint_.setFont(font);
    hint_.setString(str_util::utf8("拖动图片平移 · 滚轮缩放 · 框内即为头像内容"));
    hint_.setCharacterSize(20);
    hint_.setFillColor(sf::Color(200, 205, 225));
    hint_.setPosition(sf::Vector2f(px + 36.f, py + PH - 172.f));

    errText_.setFont(font);
    errText_.setCharacterSize(20);
    errText_.setFillColor(sf::Color(255, 150, 150));
    errText_.setPosition(sf::Vector2f(px + 36.f, py + PH - 142.f));

    btnPick_.setText("选择图片");
    btnPick_.setPosition(sf::Vector2f(px + 36.f, py + PH - 82.f));
    btnPick_.setSize(sf::Vector2f(200.f, 52.f));
    btnPick_.setCallback([this]() {
        if (pickFile() == 1) resetView();        // 选中并解码成功 -> 重置视图
    });

    btnCancel_.setText("取消");
    btnCancel_.setPosition(sf::Vector2f(px + PW - 36.f - 160.f - 20.f - 160.f, py + PH - 82.f));
    btnCancel_.setSize(sf::Vector2f(160.f, 52.f));
    btnCancel_.setCallback([this]() { open_ = false; });

    btnOk_.setText("确定");
    btnOk_.setPosition(sf::Vector2f(px + PW - 36.f - 160.f, py + PH - 82.f));
    btnOk_.setSize(sf::Vector2f(160.f, 52.f));
    btnOk_.setCallback([this]() { confirm(); });
}

sf::FloatRect AvatarCropDialog::cropRect() const {
    const float left = (WW - RT_SIZE) / 2.f;
    return sf::FloatRect(left, SQUARE_TOP, RT_SIZE, RT_SIZE);
}

void AvatarCropDialog::open(sf::WindowHandle owner) {
    owner_ = owner;
    const int r = pickFile();
    if (r == 0) return;                          // 用户取消选择 -> 不打开弹窗
    open_ = true;                                // 成功/失败都打开
    if (r == 1) resetView();
}

// 初始视图: 图片刚好覆盖裁剪框并居中
void AvatarCropDialog::resetView() {
    const sf::FloatRect r = cropRect();
    const float iw = static_cast<float>(img_.getSize().x);
    const float ih = static_cast<float>(img_.getSize().y);
    minZoom_ = std::max(r.width / iw, r.height / ih);
    zoom_ = minZoom_;
    pos_ = sf::Vector2f(r.left + (r.width - iw * zoom_) / 2.f,
                        r.top + (r.height - ih * zoom_) / 2.f);
    clampView();
    renderPreview();
}

// 系统选图: 0=用户取消 1=成功 2=加载失败 与"选择图片"按钮共用)
int AvatarCropDialog::pickFile() {
    std::string path;
    if (!pickImageFile(path, owner_)) return 0;
    if (!loadImageFile(path, img_) || img_.getSize().x == 0 || img_.getSize().y == 0 ||
        !tex_.loadFromImage(img_)) {
        hasImage_ = false;
        errText_.setString(str_util::utf8("图片加载失败, 请重新选择"));
        return 2;
    }
    hasImage_ = true;
    errText_.setString(sf::String());
    return 1;
}

void AvatarCropDialog::clampView() {
    if (!hasImage_) return;
    const float iw = img_.getSize().x * zoom_;
    const float ih = img_.getSize().y * zoom_;
    const sf::FloatRect r = cropRect();
    if (pos_.x > r.left) pos_.x = r.left;                                  // 左不露白
    if (pos_.y > r.top) pos_.y = r.top;                                    // 上不露白
    if (pos_.x + iw < r.left + r.width) pos_.x = r.left + r.width - iw;    // 右不露白
    if (pos_.y + ih < r.top + r.height) pos_.y = r.top + r.height - ih;    // 下不露白
}

sf::IntRect AvatarCropDialog::srcRect() const {
    const sf::FloatRect r = cropRect();
    const float w = static_cast<float>(img_.getSize().x);
    const float h = static_cast<float>(img_.getSize().y);
    int ix = static_cast<int>(std::lround((r.left - pos_.x) / zoom_));
    int iy = static_cast<int>(std::lround((r.top - pos_.y) / zoom_));
    int iw = static_cast<int>(std::lround(r.width / zoom_));
    int ih = static_cast<int>(std::lround(r.height / zoom_));
    if (iw < 1) iw = 1;
    if (ih < 1) ih = 1;
    if (iw > static_cast<int>(w)) iw = static_cast<int>(w);
    if (ih > static_cast<int>(h)) ih = static_cast<int>(h);
    ix = std::max(0, std::min(ix, static_cast<int>(w) - iw));
    iy = std::max(0, std::min(iy, static_cast<int>(h) - ih));
    return sf::IntRect(ix, iy, iw, ih);
}

void AvatarCropDialog::renderPreview() {
    if (preview_.getSize().x != static_cast<unsigned>(RT_SIZE)) {
        preview_.create(static_cast<unsigned>(RT_SIZE), static_cast<unsigned>(RT_SIZE));
    }
    preview_.clear(sf::Color(20, 24, 36));
    if (hasImage_) {
        const sf::IntRect src = srcRect();
        sf::Sprite sp(tex_);
        sp.setTextureRect(src);
        sp.setScale(RT_SIZE / static_cast<float>(src.width), RT_SIZE / static_cast<float>(src.height));
        sp.setPosition(0.f, 0.f);
        preview_.draw(sp);
    }
    preview_.display();
}

void AvatarCropDialog::confirm() {
    if (!hasImage_) return;                      // 未选图: 不响应确定
    sf::RenderTexture rt;
    if (!rt.create(SAVE_SIZE, SAVE_SIZE)) return;
    rt.clear(sf::Color(0, 0, 0, 255));
    const sf::IntRect src = srcRect();
    sf::Sprite sp(tex_);
    sp.setTextureRect(src);
    sp.setScale(SAVE_SIZE / static_cast<float>(src.width), SAVE_SIZE / static_cast<float>(src.height));
    sp.setPosition(0.f, 0.f);
    rt.draw(sp);
    rt.display();
    sf::Image out = rt.getTexture().copyToImage();
    if (!out.saveToFile(AVATAR_OUT)) {
        errText_.setString(str_util::utf8("头像保存失败, 请重试"));
        return;
    }
    open_ = false;
    if (onSaved_) onSaved_();
}

void AvatarCropDialog::handleEvent(const sf::Event& e, const sf::RenderWindow& win) {
    if (!open_) return;

    if (e.type == sf::Event::MouseWheelScrolled && hasImage_) {
        const float old = zoom_;
        zoom_ = (e.mouseWheelScroll.delta > 0.f) ? zoom_ * 1.1f : zoom_ / 1.1f;
        zoom_ = std::max(minZoom_, std::min(zoom_, minZoom_ * 8.f));
        if (zoom_ != old) {
            const sf::FloatRect r = cropRect();
            const sf::Vector2f c(r.left + r.width / 2.f, r.top + r.height / 2.f);   // 以框心为锚点
            pos_ = c - (c - pos_) * (zoom_ / old);
            clampView();
            renderPreview();
        }
        return;
    }

    if (e.type == sf::Event::MouseButtonPressed && e.mouseButton.button == sf::Mouse::Left) {
        const sf::Vector2f mp = win.mapPixelToCoords(sf::Vector2i(e.mouseButton.x, e.mouseButton.y));
        if (hasImage_ && cropRect().contains(mp)) {
            dragging_ = true;
            dragLast_ = mp;
            return;
        }
    }
    if (e.type == sf::Event::MouseMoved && dragging_) {
        const sf::Vector2f mp = win.mapPixelToCoords(sf::Vector2i(e.mouseMove.x, e.mouseMove.y));
        pos_ += mp - dragLast_;
        dragLast_ = mp;
        clampView();
        renderPreview();
        return;
    }
    if (e.type == sf::Event::MouseButtonReleased) dragging_ = false;

    btnPick_.handleEvent(e, win);
    btnOk_.handleEvent(e, win);
    btnCancel_.handleEvent(e, win);
}

void AvatarCropDialog::draw(sf::RenderWindow& win) {
    if (!open_) return;
    win.draw(overlay_);
    win.draw(panel_);
    panel_frame::draw(win, sf::FloatRect(panel_.getPosition(), panel_.getSize()));   // 装饰边框
    win.draw(title_);

    const sf::FloatRect cr = cropRect();
    // 裁剪框内: 预览图; 框外: 变暗
    sf::RectangleShape shade;
    shade.setFillColor(sf::Color(0, 0, 0, 120));
    const float px = panel_.getPosition().x, py = panel_.getPosition().y;
    const float pw = panel_.getSize().x, ph = panel_.getSize().y;
    const struct { float x, y, w, h; } blocks[4] = {
        { px, py, pw, cr.top - py },                                  // 上
        { px, cr.top + cr.height, pw, py + ph - (cr.top + cr.height) },// 下
        { px, cr.top, cr.left - px, cr.height },                      // 左
        { cr.left + cr.width, cr.top, px + pw - (cr.left + cr.width), cr.height }  // 右
    };
    for (const auto& b : blocks) {
        if (b.w <= 0.f || b.h <= 0.f) continue;
        shade.setSize(sf::Vector2f(b.w, b.h));
        shade.setPosition(sf::Vector2f(b.x, b.y));
        win.draw(shade);
    }
    if (hasImage_ && preview_.getSize().x > 0) {
        sf::Sprite pv(preview_.getTexture());
        pv.setPosition(cr.left, cr.top);
        win.draw(pv);
    } else {
        sf::RectangleShape empty(sf::Vector2f(cr.width, cr.height));
        empty.setPosition(cr.left, cr.top);
        empty.setFillColor(sf::Color(20, 24, 36));
        win.draw(empty);
    }
    win.draw(border_);
    win.draw(hint_);
    win.draw(errText_);
    btnPick_.draw(win);
    btnCancel_.draw(win);
    btnOk_.draw(win);
}
