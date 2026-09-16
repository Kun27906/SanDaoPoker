#pragma once
#include <SFML/Graphics.hpp>
#include "ui/IconButton.h"
#include "ui/Button.h"
#include "ui/TextBox.h"

// 左上角常驻键与弹窗
class SceneManager;

class GlobalHud {
public:
    explicit GlobalHud(SceneManager* mgr);

    bool handleEvent(const sf::Event& e, const sf::RenderWindow& win);
    void update(float dt);
    void draw(sf::RenderWindow& win);

    bool onCloseRequested(bool isEscape);
    bool shouldClose() const { return exitConfirmed_; }

private:
    void openPopup();
    void closePopup();
    void updateKnob();
    void setVolumeFromMouse(float mx);
    void setVolume(int v);
    void toggleMute();
    void showRulesPage();
    void showMainPage();
    void rebuildRulesText();
    void drawRulesText(sf::RenderWindow& win);
    void openDevPopup();
    void refreshDevToggle();
    void drawDevPopup(sf::RenderWindow& win);
    void updateDevKnob();
    void setBalanceFromMouse(float mx);
    void applyDevInput();
    void applyDevBalance(int v);
    void handleDevText(const sf::Event& e);
    void openExitPopup(bool inRound);
    void confirmExit();

    SceneManager* mgr_;
    IconButton btnMenu_, btnMusic_, btnWrench_, btnHome_;
    bool popupOpen_ = false;
    bool showRules_ = false;

    bool devOn_ = false;
    bool devArm_ = false;
    float devArmTimer_ = 0.f;

    sf::RectangleShape overlay_;
    sf::RectangleShape panel_;
    sf::RectangleShape track_;
    sf::RectangleShape trackFill_;
    IconButton btnClose_;
    IconButton btnVolIcon_;
    IconButton knob_;
    Button btnRules_;
    TextBox rulesTitle_;
    sf::Text rulesText_;
    sf::RenderTexture rulesRtex_;
    sf::Sprite rulesSprite_;
    float rulesContentH_ = 0.f;
    float rulesScrollY_ = 0.f;
    float rulesScrollMax_ = 0.f;
    bool rulesRtexDirty_ = true;
    Button btnRulesBack_;
    sf::FloatRect trackRect_;
    float vol_ = 100.f;
    float savedVol_ = 100.f;
    bool dragging_ = false;

    Button btnDevToggle_;
    sf::RectangleShape devTrack_;
    sf::RectangleShape devFill_;
    IconButton devKnob_;
    sf::FloatRect devTrackRect_;
    bool devDrag_ = false;
    sf::RectangleShape devInputBox_;
    bool devInputFocus_ = false;
    std::string devInputStr_;
    bool devPopupOpen_ = false;

    bool exitPopupOpen_ = false;
    bool exitConfirmed_ = false;
    sf::RectangleShape exitDialog_;
    TextBox exitText_;
    Button btnExitOk_, btnExitCancel_;
};
