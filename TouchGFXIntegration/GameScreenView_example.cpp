#include <gui/game_screen/GameScreenView.hpp>
#include "../CoreGame/GameConfig.hpp"
#include "../CoreGame/GameTypes.hpp"

GameScreenView::GameScreenView()
{
}

void GameScreenView::setupScreen()
{
    GameScreenViewBase::setupScreen();
}

void GameScreenView::tearDownScreen()
{
    GameScreenViewBase::tearDownScreen();
}

void GameScreenView::handleTickEvent()
{
    renderGame();
}

static void setBox(touchgfx::Box& box, const si::Rect& r)
{
    box.setVisible(r.active);
    if (r.active)
    {
        box.moveTo(r.x, r.y);
        box.setWidthHeight(r.w, r.h);
    }
    box.invalidate();
}

void GameScreenView::renderGame()
{
    const si::RenderState& s = presenter->getGameState();

    setBox(playerBox, s.player.box);

    touchgfx::Box* playerBullets[si::MAX_PLAYER_BULLETS] = {
        &bulletBox0, &bulletBox1, &bulletBox2, &bulletBox3, &bulletBox4,
        &bulletBox5, &bulletBox6, &bulletBox7, &bulletBox8, &bulletBox9
    };
    for (int i = 0; i < si::MAX_PLAYER_BULLETS; ++i)
    {
        setBox(*playerBullets[i], s.playerBullets[i].box);
        if (s.playerBullets[i].type == si::BULLET_POWER)
        {
            playerBullets[i]->setColor(touchgfx::Color::getColorFromRGB(255, 200, 50));
        }
        else
        {
            playerBullets[i]->setColor(touchgfx::Color::getColorFromRGB(80, 220, 255));
        }
    }

    touchgfx::Box* enemyBullets[si::MAX_ENEMY_BULLETS] = {
        &enemyBulletBox0, &enemyBulletBox1, &enemyBulletBox2, &enemyBulletBox3,
        &enemyBulletBox4, &enemyBulletBox5, &enemyBulletBox6, &enemyBulletBox7
    };
    for (int i = 0; i < si::MAX_ENEMY_BULLETS; ++i)
    {
        setBox(*enemyBullets[i], s.enemyBullets[i].box);
    }

    touchgfx::Box* enemies[si::MAX_ENEMIES] = {
        &enemyBox0, &enemyBox1, &enemyBox2, &enemyBox3, &enemyBox4, &enemyBox5,
        &enemyBox6, &enemyBox7, &enemyBox8, &enemyBox9, &enemyBox10, &enemyBox11,
        &enemyBox12, &enemyBox13, &enemyBox14, &enemyBox15, &enemyBox16, &enemyBox17,
        &enemyBox18, &enemyBox19, &enemyBox20, &enemyBox21, &enemyBox22, &enemyBox23
    };
    for (int i = 0; i < si::MAX_ENEMIES; ++i)
    {
        setBox(*enemies[i], s.enemies[i].box);
        enemies[i]->setColor(s.enemies[i].boss
            ? touchgfx::Color::getColorFromRGB(230, 60, 100)
            : touchgfx::Color::getColorFromRGB(70, 220, 120));
    }

    touchgfx::Box* explosions[si::MAX_EXPLOSIONS] = {
        &explosionBox0, &explosionBox1, &explosionBox2, &explosionBox3,
        &explosionBox4, &explosionBox5, &explosionBox6, &explosionBox7,
        &explosionBox8, &explosionBox9, &explosionBox10, &explosionBox11
    };
    for (int i = 0; i < si::MAX_EXPLOSIONS; ++i)
    {
        setBox(*explosions[i], s.explosions[i].box);
        explosions[i]->setColor(touchgfx::Color::getColorFromRGB(255, 130, 30));
    }

    touchgfx::Box* items[si::MAX_ITEMS] = {
        &itemBox0, &itemBox1, &itemBox2, &itemBox3, &itemBox4
    };
    for (int i = 0; i < si::MAX_ITEMS; ++i)
    {
        setBox(*items[i], s.items[i].box);
        items[i]->setColor(s.items[i].type == si::ITEM_HEAL
            ? touchgfx::Color::getColorFromRGB(50, 255, 100)
            : touchgfx::Color::getColorFromRGB(255, 220, 40));
    }

    Unicode::snprintf(scoreTextBuffer, SCORETEXT_SIZE, "%d", s.score);
    Unicode::snprintf(levelTextBuffer, LEVELTEXT_SIZE, "%d", s.level);
    Unicode::snprintf(hpTextBuffer, HPTEXT_SIZE, "%d", s.player.hp);
    Unicode::snprintf(bossHpTextBuffer, BOSSHP_TEXT_SIZE, "%d", s.bossHp);
    scoreText.invalidate();
    levelText.invalidate();
    hpText.invalidate();
    bossHpText.invalidate();
}
