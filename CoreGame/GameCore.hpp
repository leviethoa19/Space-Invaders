#ifndef GAME_CORE_HPP
#define GAME_CORE_HPP

#include "GameTypes.hpp"

namespace si
{
class GameCore
{
public:
    GameCore();

    void reset();
    void update(const InputState& input);
    const RenderState& renderState() const;
    SoundEvent consumeSoundEvent();

private:
    RenderState state;
    EnemyMoveState enemyMoveState;
    EnemyMoveState enemyAfterDownState;
    SoundEvent pendingSound;
    uint32_t tick;
    uint32_t lastShootTick;
    uint32_t lastEnemyMoveTick;
    uint32_t lastEnemyShootTick;
    uint32_t lastBossShootTick;
    uint16_t rng;
    int enemyDownPixelsLeft;

    bool previousStart;
    bool previousPause;
    bool previousShoot;

    void startGame();
    void nextLevel();
    void spawnEnemyFormation();
    void spawnBoss();
    void updatePlaying(const InputState& input);
    void updatePlayer(const InputState& input);
    void updateBullets();
    void updateEnemies();
    void updateEnemyFire();
    void updateBossFire();
    void updateItems();
    void updateExplosions();
    void checkCollisions();
    void checkLevelEnd();

    void spawnPlayerBullet(BulletType type, int x, int y, int vx, int vy, int damage);
    void tryShoot(const InputState& input);
    void spawnEnemyBullet(int x, int y, int vx, int vy);
    void spawnExplosion(int cx, int cy, uint8_t power);
    void spawnItem(int x, int y, ItemType type);
    void damagePlayer();
    void emitSound(SoundEvent sound);
    uint16_t random16();
    int enemySpeedTicks() const;
    bool activeBossExists() const;
};
}

#endif
