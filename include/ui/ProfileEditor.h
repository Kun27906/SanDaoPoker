#pragma once
#include <SFML/Graphics.hpp>
#include "ui/Avatar.h"
#include "ui/NicknameDialog.h"
#include "ui/AvatarCropDialog.h"

// ====== ProfileEditor 自设昵称/头像控制器(成员C) ======
// 把"点击本人昵称名牌改名 + 点击本人头像圆上传裁剪头像"封装在一处, 供【大厅】与【选房】共用。
// 局内场景(发牌/组牌/比牌/结算)不持有本控制器 -> 局内自然禁用修改且无任何提示(不弹窗)。
// 用法(场景内):
//   profile_.bind(&selfAvatar_);                  // 构造时绑定本人头像控件
//   if (profile_.handleEvent(e, win)) return;     // 事件: 已消费(弹窗独占 / 命中点击)
//   profile_.update(dt);   profile_.draw(win);
class ProfileEditor {
public:
    void bind(Avatar* selfAvatar);      // 绑定本人头像(命中检测 + 改名/换头像后即时刷新)
    bool handleEvent(const sf::Event& e, const sf::RenderWindow& win);   // true = 已消费
    void update(float dt);
    void draw(sf::RenderWindow& win);
    bool isOpen() const { return nick_.isOpen() || crop_.isOpen(); }

private:
    NicknameDialog   nick_;             // 自设昵称弹窗
    AvatarCropDialog crop_;             // 自设头像弹窗(选图 + 裁剪)
    Avatar* self_ = nullptr;            // 本人头像控件(弱引用, 由场景持有)
};
