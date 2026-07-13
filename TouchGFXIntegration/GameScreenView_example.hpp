#ifndef GAME_SCREEN_VIEW_EXAMPLE_HPP
#define GAME_SCREEN_VIEW_EXAMPLE_HPP

#include <gui_generated/game_screen/GameScreenViewBase.hpp>
#include <gui/game_screen/GameScreenPresenter.hpp>

// This is a template for TouchGFX/gui/include/gui/game_screen/GameScreenView.hpp.
// In Designer, create these widgets and name them exactly:
// playerBox, bossHpText, scoreText, levelText, hpText
// bulletBox0..bulletBox9, enemyBulletBox0..enemyBulletBox7
// enemyBox0..enemyBox23, explosionBox0..explosionBox11, itemBox0..itemBox4

class GameScreenView : public GameScreenViewBase
{
public:
    GameScreenView();
    virtual ~GameScreenView() {}
    virtual void setupScreen();
    virtual void tearDownScreen();
    virtual void handleTickEvent();

private:
    void renderGame();
};

#endif
