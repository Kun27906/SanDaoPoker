#pragma once
#include <SFML/Graphics.hpp>
#include "ui/Avatar.h"
#include "ui/NicknameDialog.h"
#include "ui/AvatarCropDialog.h"

// 把"点击本人昵称名牌改名 + 点击本人头像圆上传裁剪头像"封装在一处, 供【大厅】与【选房】共用。
// 局内场景不持有本控制器 -> 局内自然禁用修改且无任何提示。
// 用法:
// profile_.bind; // 构造时绑定本人头像控件
// if ) return; // 事件: 已消费
// profile_.update; profile_.draw;
class ProfileEditor {
public:
    void bind(Avatar* selfAvatar);      // 绑定本人头像
    bool handleEvent(const sf::Event& e, const sf::RenderWindow& win);   // true = 已消费
    void update(float dt);
    void draw(sf::RenderWindow& win);
    bool isOpen() const { return nick_.isOpen() || crop_.isOpen(); }

private:
    NicknameDialog   nick_;             // 自设昵称弹窗
    AvatarCropDialog crop_;             // 自设头像弹窗
    Avatar* self_ = nullptr;            // 本人头像控件
};
