#ifndef GAME_CONFIG_HPP
#define GAME_CONFIG_HPP

#include <stdint.h>

namespace si
{
static const int SCREEN_W = 240;
static const int SCREEN_H = 320;

static const int PLAYER_W = 22;
static const int PLAYER_H = 14;
static const int PLAYER_Y = 288;
static const int PLAYER_SPEED = 4;
static const int PLAYER_MAX_HP = 3;

static const int MAX_PLAYER_BULLETS = 10;
static const int MAX_ENEMY_BULLETS = 8;
static const int MAX_ENEMIES = 24;
static const int MAX_EXPLOSIONS = 12;
static const int MAX_ITEMS = 5;

static const uint32_t SHOOT_COOLDOWN_TICKS = 10;
static const uint32_t POWER_SHOOT_COOLDOWN_TICKS = 18;
static const uint32_t INVINCIBLE_TICKS = 50;

static const int NORMAL_BULLET_DAMAGE = 1;
static const int POWER_BULLET_DAMAGE = 2;
static const int SPREAD_BULLET_DAMAGE = 1;

static const int BOSS_W = 58;
static const int BOSS_H = 28;
static const int BOSS_BASE_HP = 18;

static const int ITEM_SIZE = 10;
static const uint32_t POWERUP_TICKS = 420;
}

#endif
