#include "ui/ProfileEditor.h"
#include "render/Account.h"
#include "render/AssetManager.h"
#include "render/SoundManager.h"

namespace {
constexpr float HIT_PAD = 8.f;   // 点击容差: 名牌/头像圆的命中区域外扩像素
}

void ProfileEditor::bind(Avatar* selfAvatar) {
    self_ = selfAvatar;

    // 改名确定: 写入账号存档 + 界面即时生效
    nick_.setOnConfirm([this](const std::string& nm) {
        Account::instance().setNickname(nm);
        if (self_) self_->setNickname(nm);
    });

    // 头像保存: 重新读取 game_data/avatar.png + 刷新本人头像纹理 优先自定义)
    crop_.setOnSaved([this]() {
        AssetManager::instance().reloadCustomAvatar();
        if (self_) self_->setTexture(AssetManager::instance().avatarTexture(0));
    });
}

bool ProfileEditor::handleEvent(const sf::Event& e, const sf::RenderWindow& win) {
    // 弹窗打开时独占输入
    if (nick_.isOpen()) {
        nick_.handleEvent(e, win);
        return true;
    }
    if (crop_.isOpen()) {
        crop_.handleEvent(e, win);
        return true;
    }
    if (!self_) return false;

    // 点击本人昵称名牌 -> 改名; 点击本人头像圆 -> 上传裁剪新头像
    // - 坐标用 mapPixelToCoords 换算
    // - 命中区域外扩 HIT_PAD: 名牌是细长条、头像圆也小, 给一点点击容差更好按
    if (e.type == sf::Event::MouseButtonPressed && e.mouseButton.button == sf::Mouse::Left) {
        const sf::Vector2f mp = win.mapPixelToCoords(sf::Vector2i(e.mouseButton.x, e.mouseButton.y));
        sf::FloatRect pb = self_->plateBounds();
        pb.left -= HIT_PAD;
        pb.top -= HIT_PAD;
        pb.width += HIT_PAD * 2.f;
        pb.height += HIT_PAD * 2.f;
        if (pb.contains(mp)) {
            SoundManager::instance().playClick();                    // 点击音效
            nick_.open(Account::instance().ensureNickname());
            return true;
        }
        if (self_->hitCircle(mp, HIT_PAD)) {
            SoundManager::instance().playClick();                    // 点击音效
            crop_.open(win.getSystemHandle());                       // 传窗口句柄: 文件框归属游戏窗口
            return true;
        }
    }
    return false;
}

void ProfileEditor::update(float dt) {
    nick_.update(dt);   // 输入框插入符闪烁
}

void ProfileEditor::draw(sf::RenderWindow& win) {
    nick_.draw(win);
    crop_.draw(win);
}
