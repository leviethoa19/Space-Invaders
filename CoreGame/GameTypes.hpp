#ifndef GAME_TYPES_HPP
#define GAME_TYPES_HPP

#include <stdint.h>
#include "GameConfig.hpp"

namespace si
{
enum GameMode
{
    MODE_MENU,
    MODE_PLAYING,
    MODE_PAUSED,
    MODE_GAME_OVER
};

enum EnemyMoveState
{
    ENEMY_MOVE_RIGHT,
    ENEMY_MOVE_LEFT,
    ENEMY_MOVE_DOWN
};

enum BulletType
{
    BULLET_NORMAL,
    BULLET_POWER,
    BULLET_SPREAD
};

enum ItemType
{
    ITEM_NONE,
    ITEM_HEAL,
    ITEM_POWER,
    ITEM_SCORE
};

enum SoundEvent
{
    SOUND_NONE,
    SOUND_SHOOT,
    SOUND_HIT,
    SOUND_EXPLODE_SMALL,
    SOUND_PLAYER_HIT,
    SOUND_LEVEL_UP,
    SOUND_GAME_OVER,
    SOUND_ITEM
};

struct InputState
{
    bool left;
    bool right;
    bool shoot;
    bool start;
    bool pause;
};

struct Rect
{
    int x;
    int y;
    int w;
    int h;
    bool active;
};

struct Bullet
{
    Rect box;
    int vx;
    int vy;
    int damage;
    BulletType type;
};

struct Enemy
{
    Rect box;
    int hp;
    int point;
    bool boss;
    uint8_t phase;
};

struct Explosion
{
    Rect box;
    uint8_t frame;
    uint8_t power;
};

struct Item
{
    Rect box;
    int vy;
    ItemType type;
};

struct Player
{
    Rect box;
    int hp;
    int lives;
    uint32_t invincibleTicks;
    uint32_t powerTicks;
};

struct RenderState
{
    GameMode mode;
    Player player;
    Bullet playerBullets[MAX_PLAYER_BULLETS];
    Bullet enemyBullets[MAX_ENEMY_BULLETS];
    Enemy enemies[MAX_ENEMIES];
    Explosion explosions[MAX_EXPLOSIONS];
    Item items[MAX_ITEMS];
    int score;
    int level;
    int enemyCount;
    int bossHp;
};
}

#endif
