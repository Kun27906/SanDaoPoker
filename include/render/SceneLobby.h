#pragma once
#include "render/SceneManager.h"
#include "ui/Button.h"
#include "ui/TextBox.h"
#include "ui/ChipBar.h"
#include "ui/Avatar.h"
#include "ui/VersionBadge.h"
#include "ui/ProfileEditor.h"
#include <array>

// 五个大按钮按人数进入对应房间列表; 右上角实时显示账号筹码。
class SceneLobby: public Scene {
public:
    explicit SceneLobby(SceneManager* mgr);
    void handleEvent(const sf::Event& e, const sf::RenderWindow& win) override;
    void update(float dt) override;
    void draw(sf::RenderWindow& win) override;
    void onHomePressed() override;  // home 键 = 回启动页

private:
    SceneManager* mgr_;
    sf::Sprite bg_;
    TextBox title_;
    std::array<Button, 5> btnSeats_;  // 2~6人
    Button btnReset_;  // 重置账号
    bool resetArmed_ = false;  // 已进入"再点一次确认"状态
    float resetArmTimer_ = 0.f;  // 确认状态超时自动复原
    ChipBar chipBar_;
    Avatar  selfAvatar_;  // 局外: 本人头像
    ProfileEditor profile_;  // 点击本人昵称名牌改名 / 点击本人头像圆上传图片并裁剪
    VersionBadge versionBadge_{true};

  // 破产补充弹窗
    bool pendingTopUp_ = false;
    sf::RectangleShape overlay_;
    sf::RectangleShape dialog_;
    TextBox topUpText_;
    Button btnTopUpOk_;
};
