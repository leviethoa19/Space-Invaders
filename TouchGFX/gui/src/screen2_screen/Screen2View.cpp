#include <gui/screen2_screen/Screen2View.hpp>
#include <touchgfx/Color.hpp>
#include <touchgfx/Bitmap.hpp>
#include <BitmapDatabase.hpp>
#include "stm32f4xx_hal.h"

static const int16_t PLAYER_Y = 276;
static const int16_t PLAYER_WIDTH = 26;
static const int16_t PLAYER_HEIGHT = 16;
static const int16_t PLAYER_MIN_X = 6;
static const int16_t PLAYER_MAX_X = 208;
static const int16_t PLAYER_SPEED = 4;

static const int16_t BULLET_WIDTH = 3;
static const int16_t BULLET_HEIGHT = 8;
static const int16_t BOSS_BULLET_WIDTH = 9;
static const int16_t BOSS_BULLET_HEIGHT = 16;
static const int16_t PLAYER_BULLET_SPEED = 8;
static const int16_t ENEMY_BULLET_SPEED = 5;

static const int16_t ALIEN_SIZE = 15;
static const int16_t ALIEN_LEFT_MARGIN = 6;
static const int16_t ALIEN_RIGHT_MARGIN = 234;
static const int16_t ALIEN_MOVE_PIXELS = 4;
static const int16_t ALIEN_STEP_DOWN = 8;
static const int16_t ALIEN_GAMEOVER_MARGIN = 6;
static const uint32_t ALIEN_FIRE_COOLDOWN_BASE = 130;
static const uint32_t DIVE_COOLDOWN_BASE = 220;

static const uint16_t SCORE_ALIEN_LARGE = 30;
static const uint16_t SCORE_ALIEN_MEDIUM = 20;
static const uint16_t SCORE_ALIEN_SMALL = 10;
static const uint16_t SCORE_PER_BOSS_HIT = 5;
static const uint16_t SCORE_BOSS_CLEAR_BONUS = 200;

static const int16_t BOSS_WIDTH = 70;
static const int16_t BOSS_HEIGHT = 31;
static const int16_t BOSS_Y = 66;
static const int16_t BOSS_START_X = 85;
static const int16_t BOSS_MIN_X = 10;
static const int16_t BOSS_MAX_X = 160;
static const int16_t BOSS_SPEED = 3;
static const int16_t BOSS_HP_BAR_W = 118;

static const int16_t BUNKER_WIDTH = 28;
static const int16_t BUNKER_HEIGHT = 20;
static const int16_t BUNKER_Y = 220;
static const int16_t BUNKER_X[3] = { 40, 107, 174 };

static const int16_t ITEM_SIZE = 16;
static const int16_t ITEM_SPEED = 3;

static const uint32_t INVULNERABLE_TICKS = 90;
static const uint32_t SHIELD_DURATION = 240;
static const uint32_t EXPLOSION_TICKS = 18;
static const uint32_t EXPLOSION_TICKS_FINAL = 40;
static const uint32_t HIT_SPARK_TICKS = 8;
static const uint32_t DEATH_FLASH_TICKS = 6;
static const uint16_t LED_TICKS_SHORT = 6;
static const uint16_t LED_TICKS_LONG = 15;
static const uint32_t DOUBLE_SHOT_DURATION = 600;
static const uint32_t RAPID_FIRE_DURATION = 600;

/* Buzzer "beep patterns": the buzzer is an active type (fixed internal tone,
   no pitch control), so each SoundId is distinguished purely by its on/off
   rhythm - short/long beeps and gaps - instead of by frequency. No audio
   assets needed, just on/ticks pairs. A new sound always cuts off whatever
   is currently playing, since the buzzer is a single hardware channel and
   can only play one pattern at a time. */
static const uint8_t BEEP_ON = 1;
static const uint8_t BEEP_OFF = 0;

static const Screen2View::ToneStep TONE_SHOOT[] = {
    { BEEP_ON, 3 }
};
static const Screen2View::ToneStep TONE_EXPLOSION[] = {
    { BEEP_ON, 4 }, { BEEP_OFF, 2 }, { BEEP_ON, 4 }, { BEEP_OFF, 2 }, { BEEP_ON, 8 }
};
static const Screen2View::ToneStep TONE_HIT[] = {
    { BEEP_ON, 2 }
};
static const Screen2View::ToneStep TONE_PLAYER_HIT[] = {
    { BEEP_ON, 6 }, { BEEP_OFF, 4 }, { BEEP_ON, 10 }
};
static const Screen2View::ToneStep TONE_GAMEOVER[] = {
    { BEEP_ON, 8 }, { BEEP_OFF, 5 }, { BEEP_ON, 8 }, { BEEP_OFF, 5 }, { BEEP_ON, 16 }
};
static const Screen2View::ToneStep TONE_LEVELUP[] = {
    { BEEP_ON, 3 }, { BEEP_OFF, 3 }, { BEEP_ON, 3 }, { BEEP_OFF, 3 }, { BEEP_ON, 6 }
};
static const Screen2View::ToneStep TONE_POWERUP[] = {
    { BEEP_ON, 3 }, { BEEP_OFF, 2 }, { BEEP_ON, 3 }, { BEEP_OFF, 2 }, { BEEP_ON, 3 }
};

/* Active buzzer drive pin (see MX_GPIO_Init's USER CODE section in main.c) -
   just a plain digital on/off, no PWM/frequency control needed. Using PA5,
   NOT PA1: PA1 is hard-wired on this board to the I3G4250D gyroscope's INT1
   line and fighting that signal keeps the buzzer silent even though the
   wiring/code logic itself is correct. */
static void buzzerSetOn(bool on)
{
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, on ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

Screen2View::Screen2View()
    : alienOffsetX(0),
      alienOffsetY(0),
      alienDirection(1),
      alienMoveInterval(20),
      alienMoveIntervalMin(9),
      alienMoveIntervalBase(20),
      alienFrameToggle(false),
      alienFireCooldown(0),
      alienFireCooldownMax(ALIEN_FIRE_COOLDOWN_BASE),
      diveCooldown(0),
      diveCooldownMax(DIVE_COOLDOWN_BASE),
      bossActive(false),
      bossHp(0),
      bossHpMax(0),
      bossPhase(1),
      bossDirection(1),
      bossFireCooldown(0),
      bossFireCooldownMax(70),
      fireCooldown(0),
      fireCooldownMax(14),
      doubleShotActive(false),
      doubleShotTimer(0),
      rapidFireActive(false),
      rapidFireTimer(0),
      localPlayerX(94),
      invulnerable(false),
      invulnerableTimer(0),
      shieldPowerActive(false),
      itemActive(false),
      itemX(0),
      itemY(0),
      itemType(POWERUP_RAPID_FIRE),
      explosionActive(false),
      explosionTimer(0),
      deathFlashActive(false),
      deathFlashTimer(0),
      phase(PHASE_PLAYING),
      levelClearPending(false),
      tickCount(0),
      rngState(0xACE1U),
      greenLedTicks(0),
      redLedTicks(0),
      buzzerSeq(0),
      buzzerSeqLen(0),
      buzzerSeqIndex(0),
      buzzerStepTicksLeft(0)
{
    aliens[0] = &alien0;
    aliens[1] = &alien1;
    aliens[2] = &alien2;
    aliens[3] = &alien3;
    aliens[4] = &alien4;
    aliens[5] = &alien5;
    aliens[6] = &alien6;
    aliens[7] = &alien7;
    aliens[8] = &alien8;
    aliens[9] = &alien9;
    aliens[10] = &alien10;
    aliens[11] = &alien11;
    aliens[12] = &alien12;
    aliens[13] = &alien13;
    aliens[14] = &alien14;
    aliens[15] = &alien15;
    aliens[16] = &alien16;
    aliens[17] = &alien17;
    aliens[18] = &alien18;
    aliens[19] = &alien19;
    aliens[20] = &alien20;
    aliens[21] = &alien21;
    aliens[22] = &alien22;
    aliens[23] = &alien23;
    aliens[24] = &alien24;
    aliens[25] = &alien25;
    aliens[26] = &alien26;
    aliens[27] = &alien27;
    aliens[28] = &alien28;
    aliens[29] = &alien29;
    aliens[30] = &alien30;
    aliens[31] = &alien31;
    aliens[32] = &alien32;
    aliens[33] = &alien33;
    aliens[34] = &alien34;
    aliens[35] = &alien35;
    aliens[36] = &alien36;
    aliens[37] = &alien37;
    aliens[38] = &alien38;
    aliens[39] = &alien39;

    playerBullets[0] = &playerBullet0;
    playerBullets[1] = &playerBullet1;
    playerBullets[2] = &playerBullet2;

    enemyBullets[0] = &enemyBullet0;
    enemyBullets[1] = &enemyBullet1;
    enemyBullets[2] = &enemyBullet2;

    bunkers[0] = &bunker0;
    bunkers[1] = &bunker1;
    bunkers[2] = &bunker2;

    for (uint8_t i = 0; i < BUNKER_COUNT; i++)
    {
        bunkerAlive[i] = true;
        bunkerHitsTaken[i] = 0;
    }

    for (uint8_t i = 0; i < ALIEN_COUNT; i++)
    {
        alienBaseX[i] = aliens[i]->getX();
        alienBaseY[i] = aliens[i]->getY();
        alienAlive[i] = true;
        alienState[i] = ALIEN_FORMATION;
        alienDiveTimer[i] = 0;
        alienDiveStartX[i] = 0;

        const uint8_t row = i / ALIEN_COLS;
        if (row == 0)
        {
            alienTier[i] = ALIEN_TIER_LARGE;
        }
        else if (row == (ALIEN_ROWS - 1))
        {
            alienTier[i] = ALIEN_TIER_SMALL;
        }
        else
        {
            alienTier[i] = ALIEN_TIER_MEDIUM;
        }
    }

    for (uint8_t i = 0; i < MAX_PLAYER_BULLETS; i++)
    {
        playerBulletActive[i] = false;
        playerBulletX[i] = 0;
        playerBulletY[i] = 0;
        playerBulletTier[i] = BULLET_TIER_NORMAL;
        playerBulletPierceLeft[i] = 0;
    }

    for (uint8_t i = 0; i < MAX_ENEMY_BULLETS; i++)
    {
        enemyBulletActive[i] = false;
        enemyBulletX[i] = 0;
        enemyBulletY[i] = 0;
        enemyBulletVX[i] = 0;
    }

    scoreBuffer[0] = 0;
    highScoreBuffer[0] = 0;
    levelBuffer[0] = 0;
    livesBuffer[0] = 0;
}

void Screen2View::setupScreen()
{
    Screen2ViewBase::setupScreen();

    textArea1.setWildcard(scoreBuffer);
    textArea2.setWildcard(highScoreBuffer);
    textArea3.setWildcard(levelBuffer);

    startNewGame();
}

void Screen2View::tearDownScreen()
{
    Screen2ViewBase::tearDownScreen();
}

void Screen2View::ExitFromScreen2()
{
    presenter->stopGame();
}

void Screen2View::TogglePause()
{
    if (phase == PHASE_PLAYING)
    {
        phase = PHASE_PAUSED;

        restartButton.setPosition(30, 198, 180, 36);

        overlayPanel.setVisible(true);
        overlayPanel.invalidate();
        pauseText.setVisible(true);
        pauseText.invalidate();
        continueButton.setVisible(true);
        continueButton.invalidate();
        restartButton.setVisible(true);
        restartButton.invalidate();

        updateOverlayScoreTexts();
        overlayScoreText.setVisible(true);
        overlayScoreText.invalidate();
        overlayHighScoreText.setVisible(true);
        overlayHighScoreText.invalidate();
    }
    else if (phase == PHASE_PAUSED)
    {
        ContinueGame();
    }
}

void Screen2View::ContinueGame()
{
    if (phase != PHASE_PAUSED)
    {
        return;
    }

    phase = PHASE_PLAYING;

    overlayPanel.setVisible(false);
    overlayPanel.invalidate();
    pauseText.setVisible(false);
    pauseText.invalidate();
    continueButton.setVisible(false);
    continueButton.invalidate();
    overlayScoreText.setVisible(false);
    overlayScoreText.invalidate();
    overlayHighScoreText.setVisible(false);
    overlayHighScoreText.invalidate();
    restartButton.setVisible(false);
    restartButton.invalidate();
}

void Screen2View::RestartGame()
{
    if (phase != PHASE_GAMEOVER && phase != PHASE_PAUSED)
    {
        return;
    }

    presenter->startGame();
    startNewGame();
}

void Screen2View::FireBullet()
{
    if (phase != PHASE_PLAYING || fireCooldown > 0)
    {
        return;
    }

    uint8_t slot = MAX_PLAYER_BULLETS;
    for (uint8_t i = 0; i < MAX_PLAYER_BULLETS; i++)
    {
        if (!playerBulletActive[i])
        {
            slot = i;
            break;
        }
    }
    if (slot == MAX_PLAYER_BULLETS)
    {
        return;
    }

    /* Bullet "power" tier is decided once at the moment of firing: Double Shot
       is the strongest (bigger cyan bolt, pierces one extra alien, double
       damage to the boss), Rapid Fire keeps normal power but is tinted amber
       so its faster pace is visually obvious, and with no power-up active the
       shot stays the plain base bolt. */
    const BulletTier tier = doubleShotActive ? BULLET_TIER_POWER : (rapidFireActive ? BULLET_TIER_RAPID : BULLET_TIER_NORMAL);
    const int16_t bulletWidth = (tier == BULLET_TIER_POWER) ? 6 : (tier == BULLET_TIER_RAPID) ? 4 : BULLET_WIDTH;

    playerBulletActive[slot] = true;
    setPlayerBulletVisual(slot, tier);
    playerBulletX[slot] = localPlayerX + (PLAYER_WIDTH / 2) - (bulletWidth / 2);
    playerBulletY[slot] = PLAYER_Y - BULLET_HEIGHT;
    playerBullets[slot]->setXY(playerBulletX[slot], playerBulletY[slot]);
    playerBullets[slot]->setVisible(true);
    playerBullets[slot]->invalidate();

    if (doubleShotActive)
    {
        uint8_t slot2 = MAX_PLAYER_BULLETS;
        for (uint8_t i = 0; i < MAX_PLAYER_BULLETS; i++)
        {
            if (!playerBulletActive[i])
            {
                slot2 = i;
                break;
            }
        }
        if (slot2 != MAX_PLAYER_BULLETS)
        {
            playerBulletActive[slot2] = true;
            setPlayerBulletVisual(slot2, tier);
            playerBulletX[slot2] = playerBulletX[slot] + 16;
            playerBulletY[slot2] = playerBulletY[slot];
            playerBullets[slot2]->setXY(playerBulletX[slot2], playerBulletY[slot2]);
            playerBullets[slot2]->setVisible(true);
            playerBullets[slot2]->invalidate();
        }
    }

    fireCooldown = rapidFireActive ? (fireCooldownMax / 3) : fireCooldownMax;
    playSound(SND_SHOOT);
    triggerGreenLed();
}

void Screen2View::setPlayerBulletVisual(uint8_t slot, BulletTier tier)
{
    playerBullets[slot]->invalidate();
    playerBulletTier[slot] = tier;

    switch (tier)
    {
    case BULLET_TIER_POWER:
        playerBullets[slot]->setBitmap(touchgfx::Bitmap(BITMAP_BULLETPLAYERPOWER_ID));
        playerBullets[slot]->setWidthHeight(6, 14);
        playerBulletPierceLeft[slot] = 1;
        break;
    case BULLET_TIER_RAPID:
        playerBullets[slot]->setBitmap(touchgfx::Bitmap(BITMAP_BULLETPLAYERRAPID_ID));
        playerBullets[slot]->setWidthHeight(4, 9);
        playerBulletPierceLeft[slot] = 0;
        break;
    default:
        playerBullets[slot]->setBitmap(touchgfx::Bitmap(BITMAP_BULLETPLAYER_ID));
        playerBullets[slot]->setWidthHeight(BULLET_WIDTH, BULLET_HEIGHT);
        playerBulletPierceLeft[slot] = 0;
        break;
    }
}

bool Screen2View::isBossLevel() const
{
    return (presenter->getLevel() % 3) == 0;
}

void Screen2View::resetPlayerAndBullets()
{
    for (uint8_t i = 0; i < MAX_PLAYER_BULLETS; i++)
    {
        playerBulletActive[i] = false;
        playerBullets[i]->setVisible(false);
        playerBullets[i]->invalidate();
    }
    for (uint8_t i = 0; i < MAX_ENEMY_BULLETS; i++)
    {
        enemyBulletActive[i] = false;
        enemyBullets[i]->setVisible(false);
        enemyBullets[i]->invalidate();
    }
}

void Screen2View::startNewGame()
{
    phase = PHASE_PLAYING;
    levelClearPending = false;
    tickCount = 0;

    localPlayerX = 94;
    player.setVisible(true);
    player.moveTo(localPlayerX, PLAYER_Y);

    invulnerable = false;
    invulnerableTimer = 0;
    shieldPowerActive = false;
    shieldIcon.setVisible(false);
    shieldIcon.invalidate();
    doubleShotActive = false;
    doubleShotTimer = 0;
    rapidFireActive = false;
    rapidFireTimer = 0;
    fireCooldown = 0;

    itemActive = false;
    powerItem.setVisible(false);
    powerItem.invalidate();

    resetBunkers();

    explosionActive = false;
    explosion.setVisible(false);
    explosion.invalidate();

    deathFlashActive = false;
    deathFlashTimer = 0;
    deathFlash.setAlpha(0);
    deathFlash.setVisible(false);
    deathFlash.invalidate();

    gameOverText.setVisible(false);
    gameOverText.invalidate();
    gameOverLogo.setVisible(false);
    gameOverLogo.invalidate();
    restartButton.setVisible(false);
    restartButton.invalidate();
    pauseText.setVisible(false);
    pauseText.invalidate();
    overlayPanel.setVisible(false);
    overlayPanel.invalidate();
    overlayScoreText.setVisible(false);
    overlayScoreText.invalidate();
    overlayHighScoreText.setVisible(false);
    overlayHighScoreText.invalidate();
    continueButton.setVisible(false);
    continueButton.invalidate();

    resetPlayerAndBullets();

    alienMoveIntervalBase = 20;
    alienMoveInterval = alienMoveIntervalBase;
    alienFireCooldownMax = ALIEN_FIRE_COOLDOWN_BASE;
    diveCooldownMax = DIVE_COOLDOWN_BASE;
    diveCooldown = diveCooldownMax;
    alienFireCooldown = alienFireCooldownMax;

    if (isBossLevel())
    {
        spawnBoss();
    }
    else
    {
        spawnWave();
    }

    refreshHudTexts();
}

void Screen2View::resetBunkers()
{
    for (uint8_t i = 0; i < BUNKER_COUNT; i++)
    {
        bunkerAlive[i] = true;
        bunkerHitsTaken[i] = 0;
        bunkers[i]->setXY(BUNKER_X[i], BUNKER_Y);
        bunkers[i]->setBitmap(touchgfx::Bitmap(BITMAP_SHELTERFULL_ID));
        bunkers[i]->setVisible(true);
        bunkers[i]->invalidate();
    }
}

void Screen2View::updateBunkerSprite(uint8_t index)
{
    static const uint16_t damagedIds[BUNKER_MAX_HITS - 1] = {
        BITMAP_SHELTERDAMAGED1_ID, BITMAP_SHELTERDAMAGED2_ID, BITMAP_SHELTERDAMAGED3_ID,
        BITMAP_SHELTERDAMAGED4_ID, BITMAP_SHELTERDAMAGED5_ID, BITMAP_SHELTERDAMAGED6_ID,
        BITMAP_SHELTERDAMAGED7_ID, BITMAP_SHELTERDAMAGED8_ID, BITMAP_SHELTERDAMAGED9_ID
    };

    const uint8_t hits = bunkerHitsTaken[index];
    if (hits == 0)
    {
        bunkers[index]->setBitmap(touchgfx::Bitmap(BITMAP_SHELTERFULL_ID));
    }
    else
    {
        bunkers[index]->setBitmap(touchgfx::Bitmap(damagedIds[hits - 1]));
    }
    bunkers[index]->invalidate();
}

void Screen2View::damageBunker(uint8_t index)
{
    if (!bunkerAlive[index])
    {
        return;
    }

    bunkerHitsTaken[index]++;

    if (bunkerHitsTaken[index] >= BUNKER_MAX_HITS)
    {
        bunkerAlive[index] = false;
        bunkers[index]->setVisible(false);
        bunkers[index]->invalidate();
        return;
    }

    updateBunkerSprite(index);
}

void Screen2View::crushBunker(uint8_t index)
{
    if (!bunkerAlive[index])
    {
        return;
    }

    /* An invader that physically reaches the shelter's row simply flattens it
       outright (matches how it looks - the alien is standing right on top of
       it), rather than leaving it sitting there untouched underneath/behind
       the alien sprite. */
    bunkerAlive[index] = false;
    bunkerHitsTaken[index] = BUNKER_MAX_HITS;
    bunkers[index]->setVisible(false);
    bunkers[index]->invalidate();
}

void Screen2View::refreshBunkersUnderAlien(int16_t alienX, int16_t alienY)
{
    /* Once an invader's row overlaps a shelter's row, the shelter gets
       crushed outright instead of just being force-redrawn underneath the
       alien sprite. */
    for (uint8_t k = 0; k < BUNKER_COUNT; k++)
    {
        if (!bunkerAlive[k])
        {
            continue;
        }

        const int16_t bx = bunkers[k]->getX();
        const int16_t by = bunkers[k]->getY();

        if (alienX < bx + BUNKER_WIDTH && alienX + ALIEN_SIZE > bx &&
            alienY < by + BUNKER_HEIGHT && alienY + ALIEN_SIZE > by)
        {
            crushBunker(k);
        }
    }
}

void Screen2View::checkBulletsVsBunkers()
{
    for (uint8_t k = 0; k < BUNKER_COUNT; k++)
    {
        if (!bunkerAlive[k])
        {
            continue;
        }

        const int16_t bx = bunkers[k]->getX();
        const int16_t by = bunkers[k]->getY();

        for (uint8_t i = 0; i < MAX_PLAYER_BULLETS; i++)
        {
            if (!playerBulletActive[i])
            {
                continue;
            }
            if (playerBulletX[i] < bx + BUNKER_WIDTH && playerBulletX[i] + BULLET_WIDTH > bx &&
                playerBulletY[i] < by + BUNKER_HEIGHT && playerBulletY[i] + BULLET_HEIGHT > by)
            {
                playerBulletActive[i] = false;
                playerBullets[i]->setVisible(false);
                playerBullets[i]->invalidate();
                damageBunker(k);
            }
        }

        for (uint8_t i = 0; i < MAX_ENEMY_BULLETS; i++)
        {
            if (!enemyBulletActive[i])
            {
                continue;
            }
            if (enemyBulletX[i] < bx + BUNKER_WIDTH && enemyBulletX[i] + BULLET_WIDTH > bx &&
                enemyBulletY[i] < by + BUNKER_HEIGHT && enemyBulletY[i] + BULLET_HEIGHT > by)
            {
                enemyBulletActive[i] = false;
                enemyBullets[i]->setVisible(false);
                enemyBullets[i]->invalidate();
                damageBunker(k);
            }
        }
    }
}

void Screen2View::spawnWave()
{
    bossActive = false;
    boss.setVisible(false);
    boss.invalidate();
    bossHpBg.setVisible(false);
    bossHpBg.invalidate();
    bossHpFg.setVisible(false);
    bossHpFg.invalidate();
    bossLabelText.setVisible(false);
    bossLabelText.invalidate();

    alienOffsetX = 0;
    alienOffsetY = 0;
    alienDirection = 1;
    alienFrameToggle = false;

    for (uint8_t i = 0; i < ALIEN_COUNT; i++)
    {
        alienAlive[i] = true;
        alienState[i] = ALIEN_FORMATION;
        alienDiveTimer[i] = 0;
        aliens[i]->setVisible(true);
        /* Use moveTo() rather than setXY()+invalidate(): aliens can be
           anywhere (mid-dive, near the player row) when a wave starts, so
           the old position must also be invalidated or a ghost sprite is
           left behind at the lower rows after Game Over -> restart. */
        aliens[i]->moveTo(alienBaseX[i], alienBaseY[i]);
    }

    setAlienBitmapFrame(false);
}

void Screen2View::spawnBoss()
{
    for (uint8_t i = 0; i < ALIEN_COUNT; i++)
    {
        alienAlive[i] = false;
        aliens[i]->setVisible(false);
        aliens[i]->invalidate();
    }

    bossActive = true;
    bossHpMax = 30 + (int16_t)(presenter->getLevel()) * 4;
    bossHp = bossHpMax;
    bossPhase = 1;
    bossDirection = 1;
    bossFireCooldownMax = 70;
    bossFireCooldown = bossFireCooldownMax;

    boss.setXY(BOSS_START_X, BOSS_Y);
    boss.setVisible(true);
    boss.invalidate();

    bossHpBg.setVisible(true);
    bossHpBg.invalidate();
    bossHpFg.setWidth(BOSS_HP_BAR_W);
    bossHpFg.setVisible(true);
    bossHpFg.invalidate();
    bossLabelText.setVisible(true);
    bossLabelText.invalidate();
}

void Screen2View::startNextLevel()
{
    presenter->nextLevel();
    playSound(SND_LEVELUP);
    triggerGreenLed();

    resetPlayerAndBullets();
    resetBunkers();

    /* Each level's *starting* speed is its own gently-decreasing baseline,
       not whatever fully-ramped speed the previous level ended at (aliens
       also speed up within a level as the formation shrinks/bounces off the
       walls - see moveAliens()). Without this, level N+1 would begin at
       level N's fastest in-level speed and only get faster from there,
       making every new level feel like it starts in a rush. */
    if (alienMoveIntervalBase > alienMoveIntervalMin + 4)
    {
        alienMoveIntervalBase--;
    }
    alienMoveInterval = alienMoveIntervalBase;

    if (alienFireCooldownMax > 30)
    {
        alienFireCooldownMax -= 8;
    }
    if (diveCooldownMax > 60)
    {
        diveCooldownMax -= 15;
    }
    diveCooldown = diveCooldownMax;
    alienFireCooldown = alienFireCooldownMax;

    if (isBossLevel())
    {
        spawnBoss();
    }
    else
    {
        spawnWave();
    }

    refreshHudTexts();
}

void Screen2View::enterGameOver()
{
    presenter->setGameOver();
    phase = PHASE_GAMEOVER;
    playSound(SND_GAMEOVER);
    triggerRedLed();

    /* The death explosion (and its flash, if any) is intentionally left
       running: updateExplosion()/updateDeathFlash() are still ticked while
       PHASE_GAMEOVER is active, so the final-life blast finishes playing out
       instead of vanishing the instant Game Over appears. Other timer-driven
       animations (falling power item, etc.) do freeze once PHASE_GAMEOVER is
       set, so force-clear those now, otherwise they stay stuck on screen
       forever behind/around the Game Over text. */
    if (itemActive)
    {
        itemActive = false;
        powerItem.setVisible(false);
        powerItem.invalidate();
    }

    overlayPanel.setVisible(true);
    overlayPanel.invalidate();

    gameOverLogo.setVisible(true);
    gameOverLogo.invalidate();

    continueButton.setVisible(false);
    continueButton.invalidate();

    restartButton.setPosition(30, 158, 180, 36);
    restartButton.setVisible(true);
    restartButton.invalidate();

    updateOverlayScoreTexts();
    overlayScoreText.setVisible(true);
    overlayScoreText.invalidate();
    overlayHighScoreText.setVisible(true);
    overlayHighScoreText.invalidate();

    refreshHudTexts();
}

void Screen2View::setAlienBitmapFrame(bool frame2)
{
    const touchgfx::Bitmap largeBmp(frame2 ? BITMAP_ALIENLARGE2_ID : BITMAP_ALIENLARGE1_ID);
    const touchgfx::Bitmap mediumBmp(frame2 ? BITMAP_ALIENMEDIUM2_ID : BITMAP_ALIENMEDIUM1_ID);
    const touchgfx::Bitmap smallBmp(frame2 ? BITMAP_ALIENSMALL2_ID : BITMAP_ALIENSMALL1_ID);

    for (uint8_t i = 0; i < ALIEN_COUNT; i++)
    {
        if (!alienAlive[i])
        {
            continue;
        }

        switch (alienTier[i])
        {
        case ALIEN_TIER_LARGE:
            aliens[i]->setBitmap(largeBmp);
            break;
        case ALIEN_TIER_MEDIUM:
            aliens[i]->setBitmap(mediumBmp);
            break;
        default:
            aliens[i]->setBitmap(smallBmp);
            break;
        }
    }
}

bool Screen2View::aliveAliensLeft() const
{
    for (uint8_t i = 0; i < ALIEN_COUNT; i++)
    {
        if (alienAlive[i])
        {
            return true;
        }
    }
    return false;
}

uint32_t Screen2View::nextRandom()
{
    rngState = rngState * 1103515245U + 12345U;
    return (rngState >> 16);
}

void Screen2View::moveAliens()
{
    if ((tickCount % alienMoveInterval) != 0)
    {
        return;
    }

    int16_t minX = 32767;
    int16_t maxX = -32767;
    bool any = false;

    for (uint8_t i = 0; i < ALIEN_COUNT; i++)
    {
        if (!alienAlive[i] || alienState[i] != ALIEN_FORMATION)
        {
            continue;
        }
        any = true;
        const int16_t x = alienBaseX[i] + alienOffsetX;
        if (x < minX)
        {
            minX = x;
        }
        if ((x + ALIEN_SIZE) > maxX)
        {
            maxX = x + ALIEN_SIZE;
        }
    }

    if (!any)
    {
        return;
    }

    bool hitEdge = false;
    if (alienDirection > 0 && (maxX + ALIEN_MOVE_PIXELS) > ALIEN_RIGHT_MARGIN)
    {
        hitEdge = true;
    }
    else if (alienDirection < 0 && (minX - ALIEN_MOVE_PIXELS) < ALIEN_LEFT_MARGIN)
    {
        hitEdge = true;
    }

    if (hitEdge)
    {
        alienDirection = (int8_t)(-alienDirection);
        alienOffsetY += ALIEN_STEP_DOWN;
        if (alienMoveInterval > alienMoveIntervalMin)
        {
            alienMoveInterval--;
        }
    }
    else
    {
        alienOffsetX += alienDirection * ALIEN_MOVE_PIXELS;
    }

    alienFrameToggle = !alienFrameToggle;
    setAlienBitmapFrame(alienFrameToggle);

    for (uint8_t i = 0; i < ALIEN_COUNT; i++)
    {
        if (!alienAlive[i] || alienState[i] != ALIEN_FORMATION)
        {
            continue;
        }
        aliens[i]->moveTo(alienBaseX[i] + alienOffsetX, alienBaseY[i] + alienOffsetY);
        refreshBunkersUnderAlien(aliens[i]->getX(), aliens[i]->getY());
    }
}

void Screen2View::updateDivingAliens()
{
    static const int16_t DIVE_AMPLITUDE = 16;
    static const uint16_t DIVE_PERIOD = 40;
    static const int16_t DIVE_SPEED = 3;
    static const int16_t RETURN_SPEED = 6;

    for (uint8_t i = 0; i < ALIEN_COUNT; i++)
    {
        if (!alienAlive[i])
        {
            continue;
        }

        if (alienState[i] == ALIEN_DIVING)
        {
            alienDiveTimer[i]++;

            const uint16_t phase = alienDiveTimer[i] % DIVE_PERIOD;
            int16_t sway;
            if (phase < (DIVE_PERIOD / 2))
            {
                sway = (int16_t)(-DIVE_AMPLITUDE + (phase * (2 * DIVE_AMPLITUDE) / (DIVE_PERIOD / 2)));
            }
            else
            {
                sway = (int16_t)(DIVE_AMPLITUDE - ((phase - (DIVE_PERIOD / 2)) * (2 * DIVE_AMPLITUDE) / (DIVE_PERIOD / 2)));
            }

            int16_t newX = alienDiveStartX[i] + sway;
            int16_t newY = aliens[i]->getY() + DIVE_SPEED;

            if (newX < 2)
            {
                newX = 2;
            }
            if (newX > (240 - ALIEN_SIZE - 2))
            {
                newX = 240 - ALIEN_SIZE - 2;
            }

            aliens[i]->moveTo(newX, newY);
            refreshBunkersUnderAlien(newX, newY);

            if (newY > (PLAYER_Y - 10))
            {
                alienState[i] = ALIEN_RETURNING;
            }
        }
        else if (alienState[i] == ALIEN_RETURNING)
        {
            const int16_t targetY = alienBaseY[i] + alienOffsetY;
            int16_t newY = aliens[i]->getY() - RETURN_SPEED;

            if (newY <= targetY)
            {
                aliens[i]->moveTo(alienBaseX[i] + alienOffsetX, targetY);
                alienState[i] = ALIEN_FORMATION;
            }
            else
            {
                aliens[i]->moveTo(aliens[i]->getX(), newY);
            }
            refreshBunkersUnderAlien(aliens[i]->getX(), aliens[i]->getY());
        }
    }
}

void Screen2View::maybeStartDive()
{
    if (bossActive)
    {
        return;
    }
    if (diveCooldown > 0)
    {
        diveCooldown--;
        return;
    }

    uint8_t candidates[ALIEN_COUNT];
    uint8_t count = 0;
    for (uint8_t i = 0; i < ALIEN_COUNT; i++)
    {
        if (alienAlive[i] && alienState[i] == ALIEN_FORMATION)
        {
            candidates[count++] = i;
        }
    }
    if (count == 0)
    {
        return;
    }

    const uint8_t pick = candidates[nextRandom() % count];
    alienState[pick] = ALIEN_DIVING;
    alienDiveTimer[pick] = 0;
    alienDiveStartX[pick] = aliens[pick]->getX();

    diveCooldown = diveCooldownMax;
}

void Screen2View::maybeAlienFire()
{
    if (bossActive)
    {
        return;
    }
    if (alienFireCooldown > 0)
    {
        alienFireCooldown--;
        return;
    }

    uint8_t slot = MAX_ENEMY_BULLETS;
    for (uint8_t i = 0; i < MAX_ENEMY_BULLETS; i++)
    {
        if (!enemyBulletActive[i])
        {
            slot = i;
            break;
        }
    }
    if (slot == MAX_ENEMY_BULLETS)
    {
        return;
    }

    uint8_t candidates[ALIEN_COUNT];
    uint8_t count = 0;
    for (uint8_t i = 0; i < ALIEN_COUNT; i++)
    {
        if (alienAlive[i])
        {
            candidates[count++] = i;
        }
    }
    if (count == 0)
    {
        return;
    }

    const uint8_t pick = candidates[nextRandom() % count];
    enemyBulletActive[slot] = true;
    enemyBullets[slot]->invalidate();
    enemyBullets[slot]->setBitmap(touchgfx::Bitmap(BITMAP_BULLETENEMY_ID));
    enemyBullets[slot]->setWidthHeight(BULLET_WIDTH, BULLET_HEIGHT);
    enemyBulletX[slot] = aliens[pick]->getX() + (ALIEN_SIZE / 2) - (BULLET_WIDTH / 2);
    enemyBulletY[slot] = aliens[pick]->getY() + ALIEN_SIZE;
    enemyBulletVX[slot] = 0;
    enemyBullets[slot]->setXY(enemyBulletX[slot], enemyBulletY[slot]);
    enemyBullets[slot]->setVisible(true);
    enemyBullets[slot]->invalidate();

    alienFireCooldown = alienFireCooldownMax;
}

void Screen2View::moveBoss()
{
    if (!bossActive)
    {
        return;
    }
    if ((tickCount % 3) != 0)
    {
        return;
    }

    int16_t x = boss.getX();
    const int16_t speed = BOSS_SPEED + (int16_t)(bossPhase - 1) * 2;
    x += bossDirection * speed;

    if (x < BOSS_MIN_X)
    {
        x = BOSS_MIN_X;
        bossDirection = 1;
    }
    if (x > BOSS_MAX_X)
    {
        x = BOSS_MAX_X;
        bossDirection = -1;
    }

    boss.moveTo(x, boss.getY());
}

void Screen2View::maybeBossFire()
{
    if (!bossActive)
    {
        return;
    }
    if (bossFireCooldown > 0)
    {
        bossFireCooldown--;
        return;
    }

    const uint8_t shotsThisVolley = bossPhase;
    uint8_t fired = 0;

    for (uint8_t i = 0; i < MAX_ENEMY_BULLETS && fired < shotsThisVolley; i++)
    {
        if (enemyBulletActive[i])
        {
            continue;
        }

        enemyBulletActive[i] = true;
        enemyBullets[i]->invalidate();
        /* The boss fires visibly heavier ordinance than a regular invader so
           its shots read as more dangerous and telegraph the volley. */
        enemyBullets[i]->setBitmap(touchgfx::Bitmap(BITMAP_BULLETBOSS_ID));
        enemyBullets[i]->setWidthHeight(BOSS_BULLET_WIDTH, BOSS_BULLET_HEIGHT);
        const int16_t spreadIndex = (int16_t)fired - (int16_t)(shotsThisVolley / 2);
        enemyBulletX[i] = boss.getX() + (BOSS_WIDTH / 2) - (BOSS_BULLET_WIDTH / 2) + spreadIndex * 14;
        enemyBulletY[i] = boss.getY() + BOSS_HEIGHT;
        enemyBulletVX[i] = (int8_t)(spreadIndex * 2);
        enemyBullets[i]->setXY(enemyBulletX[i], enemyBulletY[i]);
        enemyBullets[i]->setVisible(true);
        enemyBullets[i]->invalidate();
        fired++;
    }

    bossFireCooldown = bossFireCooldownMax;
}

void Screen2View::movePlayerBullets()
{
    for (uint8_t i = 0; i < MAX_PLAYER_BULLETS; i++)
    {
        if (!playerBulletActive[i])
        {
            continue;
        }

        playerBulletY[i] -= PLAYER_BULLET_SPEED;

        if (playerBulletY[i] < -BULLET_HEIGHT)
        {
            playerBulletActive[i] = false;
            playerBullets[i]->setVisible(false);
            playerBullets[i]->invalidate();
            continue;
        }

        playerBullets[i]->moveTo(playerBulletX[i], playerBulletY[i]);
    }

    checkPlayerBulletVsAliens();
    checkPlayerBulletVsBoss();
}

void Screen2View::checkPlayerBulletVsAliens()
{
    if (bossActive)
    {
        return;
    }

    for (uint8_t b = 0; b < MAX_PLAYER_BULLETS; b++)
    {
        if (!playerBulletActive[b])
        {
            continue;
        }

        for (uint8_t i = 0; i < ALIEN_COUNT; i++)
        {
            if (!alienAlive[i])
            {
                continue;
            }

            const int16_t ax = aliens[i]->getX();
            const int16_t ay = aliens[i]->getY();

            if (playerBulletX[b] < ax + ALIEN_SIZE &&
                playerBulletX[b] + BULLET_WIDTH > ax &&
                playerBulletY[b] < ay + ALIEN_SIZE &&
                playerBulletY[b] + BULLET_HEIGHT > ay)
            {
                killAlien(i, playerBulletTier[b]);

                if (playerBulletPierceLeft[b] > 0)
                {
                    /* Powered-up shot: punches through this alien and keeps
                       flying, so it can go on to hit another one in the same
                       column on a later tick instead of fizzling out here. */
                    playerBulletPierceLeft[b]--;
                    continue;
                }

                playerBulletActive[b] = false;
                playerBullets[b]->setVisible(false);
                playerBullets[b]->invalidate();
                break;
            }
        }
    }
}

void Screen2View::checkPlayerBulletVsBoss()
{
    if (!bossActive)
    {
        return;
    }

    const int16_t bx = boss.getX();
    const int16_t by = boss.getY();

    for (uint8_t b = 0; b < MAX_PLAYER_BULLETS; b++)
    {
        if (!playerBulletActive[b])
        {
            continue;
        }

        if (playerBulletX[b] < bx + BOSS_WIDTH &&
            playerBulletX[b] + BULLET_WIDTH > bx &&
            playerBulletY[b] < by + BOSS_HEIGHT &&
            playerBulletY[b] + BULLET_HEIGHT > by)
        {
            const int16_t hitX = playerBulletX[b];
            const int16_t hitY = playerBulletY[b];
            const BulletTier tier = playerBulletTier[b];

            playerBulletActive[b] = false;
            playerBullets[b]->setVisible(false);
            playerBullets[b]->invalidate();

            damageBoss(tier == BULLET_TIER_POWER ? 2 : 1, tier, hitX, hitY);
        }
    }
}

void Screen2View::killAlien(uint8_t index, BulletTier killerTier)
{
    const int16_t x = aliens[index]->getX();
    const int16_t y = aliens[index]->getY();

    alienAlive[index] = false;
    aliens[index]->setVisible(false);
    aliens[index]->invalidate();

    /* Each bullet tier gets a visibly distinct kill burst: power hits hardest
       so it gets a bigger/brighter burst, rapid keeps the normal size but
       recolored to its own warm palette, and a plain shot stays as before. */
    switch (killerTier)
    {
    case BULLET_TIER_POWER:
        triggerExplosionAt(x - 8, y - 2, EXPLOSION_ALIEN_POWER);
        break;
    case BULLET_TIER_RAPID:
        triggerExplosionAt(x - 2, y + 2, EXPLOSION_ALIEN_RAPID);
        break;
    default:
        triggerExplosionAt(x - 2, y + 2, EXPLOSION_ALIEN);
        break;
    }

    uint16_t points = SCORE_ALIEN_SMALL;
    switch (alienTier[index])
    {
    case ALIEN_TIER_LARGE:
        points = SCORE_ALIEN_LARGE;
        break;
    case ALIEN_TIER_MEDIUM:
        points = SCORE_ALIEN_MEDIUM;
        break;
    default:
        points = SCORE_ALIEN_SMALL;
        break;
    }
    presenter->addScore(points);
    playSound(SND_EXPLOSION);

    if ((nextRandom() % 100) < 18)
    {
        spawnItemAt(x + (ALIEN_SIZE / 2) - (ITEM_SIZE / 2), y);
    }
}

void Screen2View::damageBoss(int16_t amount, BulletTier killerTier, int16_t hitX, int16_t hitY)
{
    if (!bossActive)
    {
        return;
    }

    const int16_t bx = boss.getX();
    const int16_t by = boss.getY();

    bossHp -= amount;
    presenter->addScore(SCORE_PER_BOSS_HIT);
    playSound(SND_HIT);
    triggerGreenLed();

    if (bossHp < 0)
    {
        bossHp = 0;
    }

    /* Non-lethal hits get a quick tiered "hit spark" right where the shot
       landed, so sustained fire on the boss visibly communicates shot
       strength tick to tick. The lethal hit skips the spark in favor of the
       one-time death explosion triggered further below. */
    if (bossHp > 0)
    {
        switch (killerTier)
        {
        case BULLET_TIER_POWER:
            triggerExplosionAt(hitX - 2, hitY - 1, EXPLOSION_HIT_POWER);
            break;
        case BULLET_TIER_RAPID:
            triggerExplosionAt(hitX, hitY, EXPLOSION_HIT_RAPID);
            break;
        default:
            triggerExplosionAt(hitX, hitY, EXPLOSION_HIT_NORMAL);
            break;
        }
    }

    const uint8_t newPhase = (bossHp > (bossHpMax * 2 / 3)) ? 1 : ((bossHp > (bossHpMax / 3)) ? 2 : 3);
    if (newPhase != bossPhase)
    {
        bossPhase = newPhase;
        if (bossFireCooldownMax > 20)
        {
            bossFireCooldownMax -= 20;
        }
    }

    int16_t barWidth = (int16_t)((int32_t)BOSS_HP_BAR_W * bossHp / bossHpMax);
    if (barWidth < 0)
    {
        barWidth = 0;
    }
    bossHpFg.setWidth(barWidth);
    bossHpFg.invalidate();

    if (bossHp <= 0)
    {
        bossActive = false;
        boss.setVisible(false);
        boss.invalidate();
        bossHpBg.setVisible(false);
        bossHpBg.invalidate();
        bossHpFg.setVisible(false);
        bossHpFg.invalidate();
        bossLabelText.setVisible(false);
        bossLabelText.invalidate();

        triggerExplosionAt(bx + (BOSS_WIDTH / 2) - 20, by + (BOSS_HEIGHT / 2) - 8, EXPLOSION_BOSS);
        presenter->addScore(SCORE_BOSS_CLEAR_BONUS);
        levelClearPending = true;
    }
}

void Screen2View::moveEnemyBullets()
{
    for (uint8_t i = 0; i < MAX_ENEMY_BULLETS; i++)
    {
        if (!enemyBulletActive[i])
        {
            continue;
        }

        enemyBulletY[i] += ENEMY_BULLET_SPEED;
        enemyBulletX[i] += enemyBulletVX[i];

        if (enemyBulletY[i] > 320)
        {
            enemyBulletActive[i] = false;
            enemyBullets[i]->setVisible(false);
            enemyBullets[i]->invalidate();
            continue;
        }

        enemyBullets[i]->moveTo(enemyBulletX[i], enemyBulletY[i]);
    }

    checkEnemyBulletVsPlayer();
}

void Screen2View::checkEnemyBulletVsPlayer()
{
    if (invulnerable)
    {
        return;
    }

    const int16_t px = localPlayerX;
    const int16_t py = PLAYER_Y;

    for (uint8_t i = 0; i < MAX_ENEMY_BULLETS; i++)
    {
        if (!enemyBulletActive[i])
        {
            continue;
        }

        if (enemyBulletX[i] < px + PLAYER_WIDTH &&
            enemyBulletX[i] + BULLET_WIDTH > px &&
            enemyBulletY[i] < py + PLAYER_HEIGHT &&
            enemyBulletY[i] + BULLET_HEIGHT > py)
        {
            enemyBulletActive[i] = false;
            enemyBullets[i]->setVisible(false);
            enemyBullets[i]->invalidate();

            loseOneLife();
            break;
        }
    }
}

void Screen2View::checkAlienVsPlayer()
{
    if (invulnerable)
    {
        return;
    }

    const int16_t px = localPlayerX;
    const int16_t py = PLAYER_Y;
    bool kamikazeHit = false;
    bool formationReachedBottom = false;

    for (uint8_t i = 0; i < ALIEN_COUNT; i++)
    {
        if (!alienAlive[i])
        {
            continue;
        }

        const int16_t ax = aliens[i]->getX();
        const int16_t ay = aliens[i]->getY();

        const bool overlap = (ax < px + PLAYER_WIDTH) && (ax + ALIEN_SIZE > px) &&
                              (ay < py + PLAYER_HEIGHT) && (ay + ALIEN_SIZE > py);

        if (overlap)
        {
            killAlien(i, BULLET_TIER_NORMAL);
            kamikazeHit = true;
        }
        else if (alienState[i] == ALIEN_FORMATION && (ay + ALIEN_SIZE) >= (PLAYER_Y - ALIEN_GAMEOVER_MARGIN))
        {
            formationReachedBottom = true;
        }
    }

    if (formationReachedBottom)
    {
        /* Classic rule: if the invaders reach the player's row, it is game over
           regardless of remaining lives. */
        enterGameOver();
        return;
    }

    if (kamikazeHit)
    {
        loseOneLife();
    }
}

void Screen2View::loseOneLife()
{
    if (invulnerable)
    {
        return;
    }

    const bool isFinalLife = (presenter->getLives() <= 1);

    if (isFinalLife)
    {
        /* Last life: a bigger, fierier composited blast plus a brief screen
           flash, instead of the small routine hit-spark used for the other
           lives. */
        triggerExplosionAt(localPlayerX + (PLAYER_WIDTH / 2) - 17, PLAYER_Y + (PLAYER_HEIGHT / 2) - 10, EXPLOSION_PLAYER_FINAL);
        triggerDeathFlash();
    }
    else
    {
        triggerExplosionAt(localPlayerX + (PLAYER_WIDTH / 2) - 11, PLAYER_Y + (PLAYER_HEIGHT / 2) - 6, EXPLOSION_PLAYER);
    }

    const bool over = presenter->loseLife();
    playSound(SND_PLAYER_HIT);
    triggerRedLed();

    if (over)
    {
        player.setVisible(false);
        player.invalidate();
        enterGameOver();
    }
    else
    {
        invulnerable = true;
        invulnerableTimer = INVULNERABLE_TICKS;
    }

    refreshHudTexts();
}

void Screen2View::spawnItemAt(int16_t x, int16_t y)
{
    if (itemActive)
    {
        return;
    }

    itemActive = true;
    itemX = x;
    itemY = y;
    itemType = (PowerUpType)(nextRandom() % 4);

    touchgfx::colortype color;
    switch (itemType)
    {
    case POWERUP_RAPID_FIRE:
        color = touchgfx::Color::getColorFromRGB(255, 220, 40);
        break;
    case POWERUP_DOUBLE_SHOT:
        color = touchgfx::Color::getColorFromRGB(60, 220, 255);
        break;
    case POWERUP_SHIELD:
        color = touchgfx::Color::getColorFromRGB(60, 120, 255);
        break;
    default:
        color = touchgfx::Color::getColorFromRGB(255, 60, 200);
        break;
    }

    powerItem.setColor(color);
    powerItem.setXY(itemX, itemY);
    powerItem.setVisible(true);
    powerItem.invalidate();
}

void Screen2View::moveItem()
{
    if (!itemActive)
    {
        return;
    }

    itemY += ITEM_SPEED;

    if (itemY > 320)
    {
        itemActive = false;
        powerItem.setVisible(false);
        powerItem.invalidate();
        return;
    }

    powerItem.moveTo(powerItem.getX(), itemY);

    const int16_t px = localPlayerX;
    const int16_t py = PLAYER_Y;

    if (itemX < px + PLAYER_WIDTH && itemX + ITEM_SIZE > px &&
        itemY < py + PLAYER_HEIGHT && itemY + ITEM_SIZE > py)
    {
        collectItem();
    }
}

void Screen2View::collectItem()
{
    itemActive = false;
    powerItem.setVisible(false);
    powerItem.invalidate();

    applyPowerUp(itemType);
    playSound(SND_POWERUP);
    triggerGreenLed();
    refreshHudTexts();
}

void Screen2View::applyPowerUp(PowerUpType type)
{
    switch (type)
    {
    case POWERUP_RAPID_FIRE:
        rapidFireActive = true;
        rapidFireTimer = RAPID_FIRE_DURATION;
        break;
    case POWERUP_DOUBLE_SHOT:
        doubleShotActive = true;
        doubleShotTimer = DOUBLE_SHOT_DURATION;
        break;
    case POWERUP_SHIELD:
        invulnerable = true;
        invulnerableTimer = SHIELD_DURATION;
        shieldPowerActive = true;
        break;
    case POWERUP_EXTRA_LIFE:
        presenter->addLife();
        break;
    default:
        break;
    }
}

void Screen2View::triggerExplosionAt(int16_t x, int16_t y, ExplosionKind kind)
{
    explosion.invalidate();

    uint32_t duration = EXPLOSION_TICKS;

    switch (kind)
    {
    case EXPLOSION_BOSS:
        explosion.setBitmap(touchgfx::Bitmap(BITMAP_EXPLOSIONBOSS_ID));
        explosion.setWidthHeight(40, 16);
        break;
    case EXPLOSION_PLAYER:
        explosion.setBitmap(touchgfx::Bitmap(BITMAP_EXPLOSIONPLAYER_ID));
        explosion.setWidthHeight(22, 12);
        break;
    case EXPLOSION_PLAYER_FINAL:
        explosion.setBitmap(touchgfx::Bitmap(BITMAP_EXPLOSIONPLAYERBIG_ID));
        explosion.setWidthHeight(34, 20);
        duration = EXPLOSION_TICKS_FINAL;
        break;
    case EXPLOSION_ALIEN_POWER:
        explosion.setBitmap(touchgfx::Bitmap(BITMAP_EXPLOSIONALIENPOWER_ID));
        explosion.setWidthHeight(32, 20);
        duration = EXPLOSION_TICKS + 6;
        break;
    case EXPLOSION_ALIEN_RAPID:
        explosion.setBitmap(touchgfx::Bitmap(BITMAP_EXPLOSIONALIENRAPID_ID));
        explosion.setWidthHeight(24, 15);
        break;
    case EXPLOSION_HIT_NORMAL:
        explosion.setBitmap(touchgfx::Bitmap(BITMAP_HITSPARKNORMAL_ID));
        explosion.setWidthHeight(8, 8);
        duration = HIT_SPARK_TICKS;
        break;
    case EXPLOSION_HIT_RAPID:
        explosion.setBitmap(touchgfx::Bitmap(BITMAP_HITSPARKRAPID_ID));
        explosion.setWidthHeight(8, 8);
        duration = HIT_SPARK_TICKS;
        break;
    case EXPLOSION_HIT_POWER:
        explosion.setBitmap(touchgfx::Bitmap(BITMAP_HITSPARKPOWER_ID));
        explosion.setWidthHeight(12, 10);
        duration = HIT_SPARK_TICKS;
        break;
    default:
        explosion.setBitmap(touchgfx::Bitmap(BITMAP_EXPLOSIONALIEN_ID));
        explosion.setWidthHeight(24, 15);
        break;
    }

    explosion.setXY(x, y);
    explosion.setVisible(true);
    explosion.invalidate();
    explosionActive = true;
    explosionTimer = duration;
}

void Screen2View::triggerDeathFlash()
{
    deathFlash.setAlpha(190);
    deathFlash.setVisible(true);
    deathFlash.invalidate();
    deathFlashActive = true;
    deathFlashTimer = DEATH_FLASH_TICKS;
}

void Screen2View::updateDeathFlash()
{
    if (!deathFlashActive)
    {
        return;
    }

    if (deathFlashTimer > 0)
    {
        deathFlashTimer--;
        /* Step the alpha down so the flash visibly fades rather than
           popping off abruptly. */
        const uint8_t alpha = (uint8_t)((190 * deathFlashTimer) / DEATH_FLASH_TICKS);
        deathFlash.setAlpha(alpha);
        deathFlash.invalidate();
    }
    else
    {
        deathFlashActive = false;
        deathFlash.setAlpha(0);
        deathFlash.setVisible(false);
        deathFlash.invalidate();
    }
}

void Screen2View::updateExplosion()
{
    if (!explosionActive)
    {
        return;
    }

    if (explosionTimer > 0)
    {
        explosionTimer--;
    }
    else
    {
        explosionActive = false;
        explosion.setVisible(false);
        explosion.invalidate();
    }
}

void Screen2View::updateInvulnerability()
{
    if (!invulnerable)
    {
        return;
    }

    if (invulnerableTimer > 0)
    {
        invulnerableTimer--;
        player.setVisible(((invulnerableTimer / 5) % 2) == 0);
        player.invalidate();
    }
    else
    {
        invulnerable = false;
        shieldPowerActive = false;
        player.setVisible(true);
        player.invalidate();
    }

    updateShieldIcon();
}

void Screen2View::updateShieldIcon()
{
    if (!shieldPowerActive)
    {
        if (shieldIcon.isVisible())
        {
            shieldIcon.setVisible(false);
            shieldIcon.invalidate();
        }
        return;
    }

    shieldIcon.setVisible(true);
    shieldIcon.moveTo(localPlayerX + (PLAYER_WIDTH / 2) - 8, PLAYER_Y - 12);
}

void Screen2View::updatePowerUpTimers()
{
    if (rapidFireActive)
    {
        if (rapidFireTimer > 0)
        {
            rapidFireTimer--;
        }
        else
        {
            rapidFireActive = false;
        }
    }

    if (doubleShotActive)
    {
        if (doubleShotTimer > 0)
        {
            doubleShotTimer--;
        }
        else
        {
            doubleShotActive = false;
        }
    }
}

void Screen2View::updateLedFeedback()
{
    if (greenLedTicks > 0)
    {
        greenLedTicks--;
        if (greenLedTicks == 0)
        {
            HAL_GPIO_WritePin(GPIOG, GPIO_PIN_13, GPIO_PIN_RESET);
        }
    }
    if (redLedTicks > 0)
    {
        redLedTicks--;
        if (redLedTicks == 0)
        {
            HAL_GPIO_WritePin(GPIOG, GPIO_PIN_14, GPIO_PIN_RESET);
        }
    }
}

void Screen2View::triggerGreenLed()
{
    HAL_GPIO_WritePin(GPIOG, GPIO_PIN_13, GPIO_PIN_SET);
    greenLedTicks = LED_TICKS_SHORT;
}

void Screen2View::triggerRedLed()
{
    HAL_GPIO_WritePin(GPIOG, GPIO_PIN_14, GPIO_PIN_SET);
    redLedTicks = LED_TICKS_LONG;
}

void Screen2View::playSound(SoundId id)
{
    /* Active buzzer can only beep, not vary pitch, so most short in-game
       events (shoot/hit-alien/level-up/power-up) end up sounding almost the
       same and just add noise. Keep the buzzer only for the two moments
       that actually matter: losing a life and game over. */
    switch (id)
    {
    case SND_PLAYER_HIT:
        startToneSequence(TONE_PLAYER_HIT, 3);
        break;
    case SND_GAMEOVER:
        startToneSequence(TONE_GAMEOVER, 5);
        break;
    case SND_SHOOT:
    case SND_EXPLOSION:
    case SND_HIT:
    case SND_LEVELUP:
    case SND_POWERUP:
    default:
        break;
    }
}

void Screen2View::startToneSequence(const ToneStep* steps, uint8_t count)
{
    if (count == 0)
    {
        return;
    }

    buzzerSeq = steps;
    buzzerSeqLen = count;
    buzzerSeqIndex = 0;
    buzzerStepTicksLeft = steps[0].ticks;
    buzzerSetOn(steps[0].on != 0);
}

void Screen2View::updateBuzzer()
{
    if (buzzerSeqLen == 0 || buzzerStepTicksLeft == 0)
    {
        return;
    }

    buzzerStepTicksLeft--;
    if (buzzerStepTicksLeft > 0)
    {
        return;
    }

    buzzerSeqIndex++;
    if (buzzerSeqIndex < buzzerSeqLen)
    {
        buzzerStepTicksLeft = buzzerSeq[buzzerSeqIndex].ticks;
        buzzerSetOn(buzzerSeq[buzzerSeqIndex].on != 0);
    }
    else
    {
        buzzerSetOn(false);
        buzzerSeqLen = 0;
    }
}

void Screen2View::updateScoreTexts()
{
    refreshHudTexts();
}

void Screen2View::refreshHudTexts()
{
    textArea1.invalidateContent();
    textArea2.invalidateContent();
    textArea3.invalidateContent();

    Unicode::itoa((int32_t)presenter->getScore(), scoreBuffer, SCORE_TEXT_BUFFER_SIZE, 10);
    Unicode::itoa((int32_t)presenter->getHighScore(), highScoreBuffer, SCORE_TEXT_BUFFER_SIZE, 10);
    Unicode::itoa((int32_t)presenter->getLevel(), levelBuffer, SCORE_TEXT_BUFFER_SIZE, 10);

    textArea1.setWildcard(scoreBuffer);
    textArea2.setWildcard(highScoreBuffer);
    textArea3.setWildcard(levelBuffer);

    textArea1.resizeToCurrentTextWithAlignment();
    textArea2.resizeToCurrentTextWithAlignment();
    textArea3.resizeToCurrentTextWithAlignment();

    textArea1.invalidateContent();
    textArea2.invalidateContent();
    textArea3.invalidateContent();

    updateLivesText();
}

void Screen2View::updateOverlayScoreTexts()
{
    overlayScoreText.invalidateContent();
    overlayHighScoreText.invalidateContent();

    Unicode::itoa((int32_t)presenter->getScore(), scoreBuffer, SCORE_TEXT_BUFFER_SIZE, 10);
    Unicode::itoa((int32_t)presenter->getHighScore(), highScoreBuffer, SCORE_TEXT_BUFFER_SIZE, 10);

    overlayScoreText.setWildcard(scoreBuffer);
    overlayHighScoreText.setWildcard(highScoreBuffer);

    overlayScoreText.resizeToCurrentTextWithAlignment();
    overlayHighScoreText.resizeToCurrentTextWithAlignment();

    overlayScoreText.invalidateContent();
    overlayHighScoreText.invalidateContent();
}

void Screen2View::updateLivesText()
{
    livesText.invalidateContent();

    Unicode::itoa((int32_t)presenter->getLives(), livesBuffer, SCORE_TEXT_BUFFER_SIZE, 10);
    livesText.setWildcard(livesBuffer);
    livesText.resizeToCurrentTextWithAlignment();

    livesText.invalidateContent();
}

void Screen2View::handleTickEvent()
{
    Screen2ViewBase::handleTickEvent();

    updateLedFeedback();

    /* Keep ticking the death explosion/flash even once the game has frozen
       (paused or game over), so the final-life blast plays out fully instead
       of being cut off the instant the phase changes. */
    updateExplosion();
    updateDeathFlash();
    updateBuzzer();

    if (phase == PHASE_PAUSED || phase == PHASE_GAMEOVER)
    {
        return;
    }

    tickCount++;

    const int8_t dir = presenter->getCarDirection();
    localPlayerX += dir * PLAYER_SPEED;
    if (localPlayerX < PLAYER_MIN_X)
    {
        localPlayerX = PLAYER_MIN_X;
    }
    if (localPlayerX > PLAYER_MAX_X)
    {
        localPlayerX = PLAYER_MAX_X;
    }
    player.moveTo(localPlayerX, player.getY());

    if (presenter->consumeFireRequest())
    {
        FireBullet();
    }

    if (fireCooldown > 0)
    {
        fireCooldown--;
    }

    updatePowerUpTimers();
    updateInvulnerability();

    if (bossActive)
    {
        moveBoss();
        maybeBossFire();
    }
    else
    {
        moveAliens();
        updateDivingAliens();
        maybeStartDive();
        maybeAlienFire();
    }

    movePlayerBullets();
    moveEnemyBullets();
    checkBulletsVsBunkers();
    checkAlienVsPlayer();
    moveItem();

    if (phase == PHASE_GAMEOVER)
    {
        return;
    }

    if (levelClearPending)
    {
        levelClearPending = false;
        startNextLevel();
    }
    else if (!bossActive && !aliveAliensLeft())
    {
        startNextLevel();
    }

    refreshHudTexts();
}
