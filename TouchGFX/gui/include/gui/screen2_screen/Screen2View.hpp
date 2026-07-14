#ifndef SCREEN2VIEW_HPP
#define SCREEN2VIEW_HPP

#include <gui_generated/screen2_screen/Screen2ViewBase.hpp>
#include <gui/screen2_screen/Screen2Presenter.hpp>
#include <touchgfx/Unicode.hpp>

class Screen2View : public Screen2ViewBase
{
public:
    Screen2View();
    virtual ~Screen2View() {}
    virtual void setupScreen();
    virtual void tearDownScreen();
    virtual void handleTickEvent();
    virtual void ExitFromScreen2();
    virtual void FireBullet();
    virtual void TogglePause();
    virtual void RestartGame();
    virtual void ContinueGame();

    void updateScoreTexts();

    /* One step of a buzzer "beep pattern": the (active, fixed-tone) buzzer is
       either on or off for `ticks` game-ticks, played back-to-back to form a
       short rhythm per sound effect (e.g. short/long/double beep).
       Public so the beep-pattern tables can be defined as free arrays in the .cpp. */
    struct ToneStep
    {
        uint8_t on;
        uint8_t ticks;
    };

protected:
    static const uint8_t ALIEN_COLS = 10;
    static const uint8_t ALIEN_ROWS = 4;
    static const uint8_t ALIEN_COUNT = ALIEN_COLS * ALIEN_ROWS;
    static const uint8_t MAX_PLAYER_BULLETS = 3;
    static const uint8_t MAX_ENEMY_BULLETS = 3;
    static const uint8_t BUNKER_COUNT = 3;
    static const uint8_t BUNKER_MAX_HITS = 10;

    enum AlienState
    {
        ALIEN_FORMATION,
        ALIEN_DIVING,
        ALIEN_RETURNING
    };

    enum AlienTier
    {
        ALIEN_TIER_LARGE,
        ALIEN_TIER_MEDIUM,
        ALIEN_TIER_SMALL
    };

    enum ExplosionKind
    {
        EXPLOSION_ALIEN,
        EXPLOSION_ALIEN_RAPID,
        EXPLOSION_ALIEN_POWER,
        EXPLOSION_BOSS,
        EXPLOSION_PLAYER,
        EXPLOSION_PLAYER_FINAL,
        EXPLOSION_HIT_NORMAL,
        EXPLOSION_HIT_RAPID,
        EXPLOSION_HIT_POWER
    };

    enum GamePhase
    {
        PHASE_PLAYING,
        PHASE_PAUSED,
        PHASE_GAMEOVER
    };

    enum PowerUpType
    {
        POWERUP_RAPID_FIRE,
        POWERUP_DOUBLE_SHOT,
        POWERUP_SHIELD,
        POWERUP_EXTRA_LIFE
    };

    /* Visual/mechanical "power" tier of a player shot, decided at the moment
       it is fired from whichever weapon power-up is active. Higher tiers
       look different (color + size) and actually hit harder. */
    enum BulletTier
    {
        BULLET_TIER_NORMAL,
        BULLET_TIER_RAPID,
        BULLET_TIER_POWER
    };

    enum SoundId
    {
        SND_SHOOT,
        SND_EXPLOSION,
        SND_HIT,
        SND_PLAYER_HIT,
        SND_GAMEOVER,
        SND_LEVELUP,
        SND_POWERUP
    };

    // ---- Setup / wave helpers ----
    void startNewGame();
    void spawnWave();
    void spawnBoss();
    void startNextLevel();
    void enterGameOver();
    void resetPlayerAndBullets();
    bool isBossLevel() const;

    // ---- Per-frame update helpers ----
    void moveAliens();
    void updateDivingAliens();
    void maybeStartDive();
    void maybeAlienFire();
    void moveBoss();
    void maybeBossFire();
    void movePlayerBullets();
    void moveEnemyBullets();
    void moveItem();
    void updateInvulnerability();
    void updateExplosion();
    void updateDeathFlash();
    void updatePowerUpTimers();
    void updateLedFeedback();
    void updateBuzzer();

    // ---- Collision / gameplay events ----
    void checkPlayerBulletVsAliens();
    void checkPlayerBulletVsBoss();
    void checkEnemyBulletVsPlayer();
    void checkAlienVsPlayer();
    void checkBulletsVsBunkers();
    void refreshBunkersUnderAlien(int16_t alienX, int16_t alienY);
    void killAlien(uint8_t index, BulletTier killerTier);
    void damageBoss(int16_t amount, BulletTier killerTier, int16_t hitX, int16_t hitY);
    void loseOneLife();
    void spawnItemAt(int16_t x, int16_t y);
    void collectItem();
    void applyPowerUp(PowerUpType type);
    void triggerExplosionAt(int16_t x, int16_t y, ExplosionKind kind);
    void triggerDeathFlash();
    void setPlayerBulletVisual(uint8_t slot, BulletTier tier);
    bool aliveAliensLeft() const;

    // ---- Bunkers (destructible shields) ----
    void resetBunkers();
    void damageBunker(uint8_t index);
    void crushBunker(uint8_t index);
    void updateBunkerSprite(uint8_t index);

    // ---- Rendering helpers ----
    void setAlienBitmapFrame(bool frame2);
    void refreshHudTexts();
    void updateLivesText();
    void updateShieldIcon();
    void updateOverlayScoreTexts();

    // ---- Feedback: LED + buzzer ----
    void playSound(SoundId id);
    void triggerGreenLed();
    void triggerRedLed();
    void startToneSequence(const ToneStep* steps, uint8_t count);

    uint32_t nextRandom();

    touchgfx::Image* aliens[ALIEN_COUNT];
    bool alienAlive[ALIEN_COUNT];
    int16_t alienBaseX[ALIEN_COUNT];
    int16_t alienBaseY[ALIEN_COUNT];
    AlienState alienState[ALIEN_COUNT];
    AlienTier alienTier[ALIEN_COUNT];
    uint16_t alienDiveTimer[ALIEN_COUNT];
    int16_t alienDiveStartX[ALIEN_COUNT];

    touchgfx::Image* bunkers[BUNKER_COUNT];
    bool bunkerAlive[BUNKER_COUNT];
    uint8_t bunkerHitsTaken[BUNKER_COUNT];

    int16_t alienOffsetX;
    int16_t alienOffsetY;
    int8_t alienDirection;
    uint32_t alienMoveInterval;
    uint32_t alienMoveIntervalMin;
    uint32_t alienMoveIntervalBase;
    bool alienFrameToggle;
    uint32_t alienFireCooldown;
    uint32_t alienFireCooldownMax;
    uint32_t diveCooldown;
    uint32_t diveCooldownMax;

    bool bossActive;
    int16_t bossHp;
    int16_t bossHpMax;
    uint8_t bossPhase;
    int8_t bossDirection;
    uint32_t bossFireCooldown;
    uint32_t bossFireCooldownMax;

    touchgfx::Image* playerBullets[MAX_PLAYER_BULLETS];
    bool playerBulletActive[MAX_PLAYER_BULLETS];
    int16_t playerBulletX[MAX_PLAYER_BULLETS];
    int16_t playerBulletY[MAX_PLAYER_BULLETS];
    BulletTier playerBulletTier[MAX_PLAYER_BULLETS];
    uint8_t playerBulletPierceLeft[MAX_PLAYER_BULLETS];
    uint32_t fireCooldown;
    uint32_t fireCooldownMax;
    bool doubleShotActive;
    uint32_t doubleShotTimer;
    bool rapidFireActive;
    uint32_t rapidFireTimer;

    touchgfx::Image* enemyBullets[MAX_ENEMY_BULLETS];
    bool enemyBulletActive[MAX_ENEMY_BULLETS];
    int16_t enemyBulletX[MAX_ENEMY_BULLETS];
    int16_t enemyBulletY[MAX_ENEMY_BULLETS];
    int8_t enemyBulletVX[MAX_ENEMY_BULLETS];

    int16_t localPlayerX;
    bool invulnerable;
    uint32_t invulnerableTimer;
    bool shieldPowerActive;

    bool itemActive;
    int16_t itemX;
    int16_t itemY;
    PowerUpType itemType;

    bool explosionActive;
    uint32_t explosionTimer;
    bool deathFlashActive;
    uint32_t deathFlashTimer;

    GamePhase phase;
    bool levelClearPending;
    uint32_t tickCount;
    uint32_t rngState;

    uint16_t greenLedTicks;
    uint16_t redLedTicks;

    const ToneStep* buzzerSeq;
    uint8_t buzzerSeqLen;
    uint8_t buzzerSeqIndex;
    uint8_t buzzerStepTicksLeft;

    static const uint16_t SCORE_TEXT_BUFFER_SIZE = 16;
    touchgfx::Unicode::UnicodeChar scoreBuffer[SCORE_TEXT_BUFFER_SIZE];
    touchgfx::Unicode::UnicodeChar highScoreBuffer[SCORE_TEXT_BUFFER_SIZE];
    touchgfx::Unicode::UnicodeChar levelBuffer[SCORE_TEXT_BUFFER_SIZE];
    touchgfx::Unicode::UnicodeChar livesBuffer[SCORE_TEXT_BUFFER_SIZE];
};

#endif // SCREEN2VIEW_HPP
