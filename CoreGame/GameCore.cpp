#include "GameCore.hpp"

namespace si
{
static bool overlap(const Rect& a, const Rect& b)
{
    return a.active && b.active &&
           a.x < b.x + b.w &&
           a.x + a.w > b.x &&
           a.y < b.y + b.h &&
           a.y + a.h > b.y;
}

static void deactivate(Rect& r)
{
    r.active = false;
    r.x = 0;
    r.y = 0;
}

GameCore::GameCore()
{
    rng = 0xACE1u;
    reset();
}

void GameCore::reset()
{
    state.mode = MODE_MENU;
    state.score = 0;
    state.level = 1;
    state.enemyCount = 0;
    state.bossHp = 0;

    state.player.box.x = (SCREEN_W - PLAYER_W) / 2;
    state.player.box.y = PLAYER_Y;
    state.player.box.w = PLAYER_W;
    state.player.box.h = PLAYER_H;
    state.player.box.active = true;
    state.player.hp = PLAYER_MAX_HP;
    state.player.lives = 3;
    state.player.invincibleTicks = 0;
    state.player.powerTicks = 0;

    for (int i = 0; i < MAX_PLAYER_BULLETS; ++i) state.playerBullets[i].box.active = false;
    for (int i = 0; i < MAX_ENEMY_BULLETS; ++i) state.enemyBullets[i].box.active = false;
    for (int i = 0; i < MAX_ENEMIES; ++i) state.enemies[i].box.active = false;
    for (int i = 0; i < MAX_EXPLOSIONS; ++i) state.explosions[i].box.active = false;
    for (int i = 0; i < MAX_ITEMS; ++i) state.items[i].box.active = false;

    enemyMoveState = ENEMY_MOVE_RIGHT;
    enemyAfterDownState = ENEMY_MOVE_LEFT;
    pendingSound = SOUND_NONE;
    tick = 0;
    lastShootTick = 0;
    lastEnemyMoveTick = 0;
    lastEnemyShootTick = 0;
    lastBossShootTick = 0;
    enemyDownPixelsLeft = 0;
    previousStart = false;
    previousPause = false;
    previousShoot = false;
}

void GameCore::update(const InputState& input)
{
    ++tick;
    const bool startPressed = input.start && !previousStart;
    const bool pausePressed = input.pause && !previousPause;

    if (state.mode == MODE_MENU && startPressed)
    {
        startGame();
    }
    else if (state.mode == MODE_GAME_OVER && startPressed)
    {
        startGame();
    }
    else if (state.mode == MODE_PLAYING && pausePressed)
    {
        state.mode = MODE_PAUSED;
    }
    else if (state.mode == MODE_PAUSED && pausePressed)
    {
        state.mode = MODE_PLAYING;
    }

    if (state.mode == MODE_PLAYING)
    {
        updatePlaying(input);
    }

    previousStart = input.start;
    previousPause = input.pause;
    previousShoot = input.shoot;
}

const RenderState& GameCore::renderState() const
{
    return state;
}

SoundEvent GameCore::consumeSoundEvent()
{
    SoundEvent sound = pendingSound;
    pendingSound = SOUND_NONE;
    return sound;
}

void GameCore::startGame()
{
    reset();
    state.mode = MODE_PLAYING;
    spawnEnemyFormation();
}

void GameCore::nextLevel()
{
    ++state.level;
    state.player.hp = PLAYER_MAX_HP;
    if (state.level % 3 == 0)
    {
        spawnBoss();
    }
    else
    {
        spawnEnemyFormation();
    }
    emitSound(SOUND_LEVEL_UP);
}

void GameCore::spawnEnemyFormation()
{
    for (int i = 0; i < MAX_ENEMIES; ++i) state.enemies[i].box.active = false;

    const int rows = state.level >= 4 ? 4 : 3;
    const int cols = 6;
    const int startX = 18;
    const int startY = 34;
    const int gapX = 34;
    const int gapY = 24;
    int index = 0;

    for (int r = 0; r < rows; ++r)
    {
        for (int c = 0; c < cols && index < MAX_ENEMIES; ++c)
        {
            Enemy& e = state.enemies[index++];
            e.box.x = startX + c * gapX;
            e.box.y = startY + r * gapY;
            e.box.w = 18;
            e.box.h = 14;
            e.box.active = true;
            e.hp = 1 + state.level / 4;
            e.point = 10 + r * 5;
            e.boss = false;
            e.phase = 0;
        }
    }

    state.enemyCount = index;
    state.bossHp = 0;
    enemyMoveState = ENEMY_MOVE_RIGHT;
}

void GameCore::spawnBoss()
{
    for (int i = 0; i < MAX_ENEMIES; ++i) state.enemies[i].box.active = false;

    Enemy& b = state.enemies[0];
    b.box.x = (SCREEN_W - BOSS_W) / 2;
    b.box.y = 34;
    b.box.w = BOSS_W;
    b.box.h = BOSS_H;
    b.box.active = true;
    b.hp = BOSS_BASE_HP + state.level * 4;
    b.point = 250 + state.level * 30;
    b.boss = true;
    b.phase = 1;

    state.enemyCount = 1;
    state.bossHp = b.hp;
    enemyMoveState = ENEMY_MOVE_RIGHT;
}

void GameCore::updatePlaying(const InputState& input)
{
    updatePlayer(input);
    tryShoot(input);
    updateBullets();
    updateEnemies();
    updateEnemyFire();
    updateBossFire();
    updateItems();
    updateExplosions();
    checkCollisions();
    checkLevelEnd();

    if (state.player.invincibleTicks > 0) --state.player.invincibleTicks;
    if (state.player.powerTicks > 0) --state.player.powerTicks;
}

void GameCore::updatePlayer(const InputState& input)
{
    if (input.left) state.player.box.x -= PLAYER_SPEED;
    if (input.right) state.player.box.x += PLAYER_SPEED;

    if (state.player.box.x < 0) state.player.box.x = 0;
    if (state.player.box.x + state.player.box.w > SCREEN_W)
    {
        state.player.box.x = SCREEN_W - state.player.box.w;
    }
}

void GameCore::tryShoot(const InputState& input)
{
    if (!input.shoot) return;

    const bool powered = state.player.powerTicks > 0;
    const uint32_t cooldown = powered ? POWER_SHOOT_COOLDOWN_TICKS : SHOOT_COOLDOWN_TICKS;
    if (tick - lastShootTick < cooldown) return;

    const int cx = state.player.box.x + state.player.box.w / 2;
    const int y = state.player.box.y - 7;

    if (powered)
    {
        spawnPlayerBullet(BULLET_SPREAD, cx - 8, y, -1, -7, SPREAD_BULLET_DAMAGE);
        spawnPlayerBullet(BULLET_POWER, cx, y, 0, -8, POWER_BULLET_DAMAGE);
        spawnPlayerBullet(BULLET_SPREAD, cx + 8, y, 1, -7, SPREAD_BULLET_DAMAGE);
    }
    else
    {
        spawnPlayerBullet(BULLET_NORMAL, cx, y, 0, -7, NORMAL_BULLET_DAMAGE);
    }

    lastShootTick = tick;
    emitSound(SOUND_SHOOT);
}

void GameCore::spawnPlayerBullet(BulletType type, int x, int y, int vx, int vy, int damage)
{
    for (int i = 0; i < MAX_PLAYER_BULLETS; ++i)
    {
        Bullet& b = state.playerBullets[i];
        if (!b.box.active)
        {
            b.box.x = x;
            b.box.y = y;
            b.box.w = type == BULLET_POWER ? 5 : 3;
            b.box.h = type == BULLET_POWER ? 11 : 8;
            b.box.active = true;
            b.vx = vx;
            b.vy = vy;
            b.damage = damage;
            b.type = type;
            return;
        }
    }
}

void GameCore::updateBullets()
{
    for (int i = 0; i < MAX_PLAYER_BULLETS; ++i)
    {
        Bullet& b = state.playerBullets[i];
        if (!b.box.active) continue;
        b.box.x += b.vx;
        b.box.y += b.vy;
        if (b.box.y + b.box.h < 0 || b.box.x < -10 || b.box.x > SCREEN_W + 10)
        {
            b.box.active = false;
        }
    }

    for (int i = 0; i < MAX_ENEMY_BULLETS; ++i)
    {
        Bullet& b = state.enemyBullets[i];
        if (!b.box.active) continue;
        b.box.x += b.vx;
        b.box.y += b.vy;
        if (b.box.y > SCREEN_H)
        {
            b.box.active = false;
        }
    }
}

void GameCore::updateEnemies()
{
    if (tick - lastEnemyMoveTick < static_cast<uint32_t>(enemySpeedTicks())) return;
    lastEnemyMoveTick = tick;

    int dx = 0;
    int dy = 0;
    if (enemyMoveState == ENEMY_MOVE_RIGHT) dx = activeBossExists() ? 3 : 4;
    if (enemyMoveState == ENEMY_MOVE_LEFT) dx = activeBossExists() ? -3 : -4;
    if (enemyMoveState == ENEMY_MOVE_DOWN)
    {
        dy = 3;
        enemyDownPixelsLeft -= 3;
    }

    bool hitLeft = false;
    bool hitRight = false;

    for (int i = 0; i < MAX_ENEMIES; ++i)
    {
        Enemy& e = state.enemies[i];
        if (!e.box.active) continue;
        e.box.x += dx;
        e.box.y += dy;
        if (e.box.x <= 2) hitLeft = true;
        if (e.box.x + e.box.w >= SCREEN_W - 2) hitRight = true;
    }

    if (enemyMoveState == ENEMY_MOVE_DOWN && enemyDownPixelsLeft <= 0)
    {
        enemyMoveState = enemyAfterDownState;
    }
    else if (enemyMoveState == ENEMY_MOVE_RIGHT && hitRight)
    {
        enemyMoveState = ENEMY_MOVE_DOWN;
        enemyAfterDownState = ENEMY_MOVE_LEFT;
        enemyDownPixelsLeft = 12;
    }
    else if (enemyMoveState == ENEMY_MOVE_LEFT && hitLeft)
    {
        enemyMoveState = ENEMY_MOVE_DOWN;
        enemyAfterDownState = ENEMY_MOVE_RIGHT;
        enemyDownPixelsLeft = 12;
    }
}

void GameCore::updateEnemyFire()
{
    const uint32_t fireDelay = 80 > state.level * 5 ? 80 - state.level * 5 : 30;
    if (tick - lastEnemyShootTick < fireDelay) return;
    lastEnemyShootTick = tick;

    int alive[MAX_ENEMIES];
    int count = 0;
    for (int i = 0; i < MAX_ENEMIES; ++i)
    {
        if (state.enemies[i].box.active && !state.enemies[i].boss)
        {
            alive[count++] = i;
        }
    }
    if (count == 0) return;

    Enemy& e = state.enemies[alive[random16() % count]];
    spawnEnemyBullet(e.box.x + e.box.w / 2, e.box.y + e.box.h, 0, 3 + state.level / 3);
}

void GameCore::updateBossFire()
{
    if (!activeBossExists()) return;
    const uint32_t fireDelay = 38 > state.level ? 38 - state.level : 22;
    if (tick - lastBossShootTick < fireDelay) return;
    lastBossShootTick = tick;

    Enemy& b = state.enemies[0];
    if (b.hp < (BOSS_BASE_HP + state.level * 4) / 2) b.phase = 2;
    if (b.hp < (BOSS_BASE_HP + state.level * 4) / 4) b.phase = 3;

    spawnEnemyBullet(b.box.x + b.box.w / 2, b.box.y + b.box.h, 0, 4);
    if (b.phase >= 2)
    {
        spawnEnemyBullet(b.box.x + 8, b.box.y + b.box.h, -1, 4);
        spawnEnemyBullet(b.box.x + b.box.w - 8, b.box.y + b.box.h, 1, 4);
    }
    if (b.phase >= 3)
    {
        spawnEnemyBullet(b.box.x + b.box.w / 2, b.box.y + b.box.h, -2, 3);
        spawnEnemyBullet(b.box.x + b.box.w / 2, b.box.y + b.box.h, 2, 3);
    }
}

void GameCore::spawnEnemyBullet(int x, int y, int vx, int vy)
{
    for (int i = 0; i < MAX_ENEMY_BULLETS; ++i)
    {
        Bullet& b = state.enemyBullets[i];
        if (!b.box.active)
        {
            b.box.x = x;
            b.box.y = y;
            b.box.w = 4;
            b.box.h = 8;
            b.box.active = true;
            b.vx = vx;
            b.vy = vy;
            b.damage = 1;
            b.type = BULLET_NORMAL;
            return;
        }
    }
}

void GameCore::updateItems()
{
    for (int i = 0; i < MAX_ITEMS; ++i)
    {
        Item& item = state.items[i];
        if (!item.box.active) continue;
        item.box.y += item.vy;
        if (item.box.y > SCREEN_H) item.box.active = false;
    }
}

void GameCore::updateExplosions()
{
    for (int i = 0; i < MAX_EXPLOSIONS; ++i)
    {
        Explosion& e = state.explosions[i];
        if (!e.box.active) continue;
        ++e.frame;
        e.box.x -= 1;
        e.box.y -= 1;
        e.box.w += 2;
        e.box.h += 2;
        const uint8_t life = static_cast<uint8_t>(5 + e.power * 3);
        if (e.frame > life) e.box.active = false;
    }
}

void GameCore::checkCollisions()
{
    for (int bi = 0; bi < MAX_PLAYER_BULLETS; ++bi)
    {
        Bullet& b = state.playerBullets[bi];
        if (!b.box.active) continue;

        for (int ei = 0; ei < MAX_ENEMIES; ++ei)
        {
            Enemy& e = state.enemies[ei];
            if (!overlap(b.box, e.box)) continue;

            e.hp -= b.damage;
            b.box.active = false;
            spawnExplosion(b.box.x, b.box.y, static_cast<uint8_t>(b.damage));
            emitSound(SOUND_HIT);

            if (e.hp <= 0)
            {
                state.score += e.point;
                spawnExplosion(e.box.x + e.box.w / 2, e.box.y + e.box.h / 2, e.boss ? 5 : 2);
                if ((random16() % 5) == 0)
                {
                    ItemType type = (random16() & 1) ? ITEM_POWER : ITEM_HEAL;
                    spawnItem(e.box.x + e.box.w / 2, e.box.y + e.box.h / 2, type);
                }
                e.box.active = false;
                --state.enemyCount;
                emitSound(e.boss ? SOUND_GAME_OVER : SOUND_EXPLODE_SMALL);
            }

            state.bossHp = activeBossExists() ? state.enemies[0].hp : 0;
            break;
        }
    }

    for (int i = 0; i < MAX_ENEMY_BULLETS; ++i)
    {
        Bullet& b = state.enemyBullets[i];
        if (overlap(b.box, state.player.box))
        {
            b.box.active = false;
            damagePlayer();
        }
    }

    for (int i = 0; i < MAX_ENEMIES; ++i)
    {
        Enemy& e = state.enemies[i];
        if (overlap(e.box, state.player.box))
        {
            damagePlayer();
        }
        if (e.box.active && e.box.y + e.box.h >= state.player.box.y)
        {
            damagePlayer();
            e.box.active = false;
            --state.enemyCount;
        }
    }

    for (int i = 0; i < MAX_ITEMS; ++i)
    {
        Item& item = state.items[i];
        if (!overlap(item.box, state.player.box)) continue;

        if (item.type == ITEM_HEAL && state.player.hp < PLAYER_MAX_HP) ++state.player.hp;
        if (item.type == ITEM_POWER) state.player.powerTicks = POWERUP_TICKS;
        if (item.type == ITEM_SCORE) state.score += 100;
        item.box.active = false;
        emitSound(SOUND_ITEM);
    }
}

void GameCore::checkLevelEnd()
{
    if (state.enemyCount <= 0)
    {
        nextLevel();
    }
}

void GameCore::spawnExplosion(int cx, int cy, uint8_t power)
{
    for (int i = 0; i < MAX_EXPLOSIONS; ++i)
    {
        Explosion& e = state.explosions[i];
        if (!e.box.active)
        {
            const int size = 8 + power * 5;
            e.box.x = cx - size / 2;
            e.box.y = cy - size / 2;
            e.box.w = size;
            e.box.h = size;
            e.box.active = true;
            e.frame = 0;
            e.power = power;
            return;
        }
    }
}

void GameCore::spawnItem(int x, int y, ItemType type)
{
    for (int i = 0; i < MAX_ITEMS; ++i)
    {
        Item& item = state.items[i];
        if (!item.box.active)
        {
            item.box.x = x - ITEM_SIZE / 2;
            item.box.y = y - ITEM_SIZE / 2;
            item.box.w = ITEM_SIZE;
            item.box.h = ITEM_SIZE;
            item.box.active = true;
            item.vy = 2;
            item.type = type;
            return;
        }
    }
}

void GameCore::damagePlayer()
{
    if (state.player.invincibleTicks > 0) return;

    --state.player.hp;
    state.player.invincibleTicks = INVINCIBLE_TICKS;
    spawnExplosion(state.player.box.x + state.player.box.w / 2, state.player.box.y, 3);
    emitSound(SOUND_PLAYER_HIT);

    if (state.player.hp <= 0)
    {
        --state.player.lives;
        state.player.hp = PLAYER_MAX_HP;
        state.player.powerTicks = 0;
    }

    if (state.player.lives <= 0)
    {
        state.mode = MODE_GAME_OVER;
        emitSound(SOUND_GAME_OVER);
    }
}

void GameCore::emitSound(SoundEvent sound)
{
    pendingSound = sound;
}

uint16_t GameCore::random16()
{
    rng = static_cast<uint16_t>((rng >> 1) ^ (-(rng & 1u) & 0xB400u));
    return rng;
}

int GameCore::enemySpeedTicks() const
{
    int speed = 18 - state.level * 2;
    if (speed < 5) speed = 5;
    if (activeBossExists()) speed = 6;
    return speed;
}

bool GameCore::activeBossExists() const
{
    return state.enemies[0].box.active && state.enemies[0].boss;
}
}
