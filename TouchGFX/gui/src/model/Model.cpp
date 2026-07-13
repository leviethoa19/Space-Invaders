#include <gui/model/Model.hpp>
#include <gui/model/ModelListener.hpp>
#include "cmsis_os2.h"
#include "stm32f4xx_hal.h"

extern osMessageQueueId_t Queue1Handle;
extern osMessageQueueId_t Queue2Handle;

#define HIGH_SCORE_FLASH_ADDR  0x081E0000UL
#define HIGH_SCORE_MAGIC       0x48534352UL
#define PLAYER_START_LIVES     3U

static uint32_t loadHighScore(void)
{
    const volatile uint32_t* record = (const volatile uint32_t*)HIGH_SCORE_FLASH_ADDR;

    if (record[0] == HIGH_SCORE_MAGIC)
    {
        return record[1];
    }

    return 0;
}

static void saveHighScore(uint32_t score)
{
    FLASH_EraseInitTypeDef eraseInit;
    uint32_t sectorError = 0;
    uint64_t data = ((uint64_t)HIGH_SCORE_MAGIC) | ((uint64_t)score << 32);

    eraseInit.TypeErase = FLASH_TYPEERASE_SECTORS;
    eraseInit.VoltageRange = FLASH_VOLTAGE_RANGE_3;
    eraseInit.Sector = FLASH_SECTOR_23;
    eraseInit.NbSectors = 1;

    HAL_FLASH_Unlock();
    HAL_FLASHEx_Erase(&eraseInit, &sectorError);
    HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, HIGH_SCORE_FLASH_ADDR, data);
    HAL_FLASH_Lock();
}

Model::Model()
    : ImageX(95),
      modelListener(0),
      carDirection(0),
      fireRequested(false),
      score(0),
      highScore(0),
      gameSpeed(2),
      gameTickCounter(0),
      lives(PLAYER_START_LIVES),
      level(1),
      gameOver(false),
      gameRunning(false)
{
    highScore = (uint16_t)loadHighScore();
}

void Model::tick()
{
    int8_t dir = 0;

    while (osMessageQueueGet(Queue1Handle, &dir, NULL, 0) == osOK)
    {
        if (gameRunning && !gameOver)
        {
            carDirection = dir;
        }
    }

    uint8_t firePulse = 0;
    while (osMessageQueueGet(Queue2Handle, &firePulse, NULL, 0) == osOK)
    {
        if (gameRunning && !gameOver)
        {
            fireRequested = true;
        }
    }

    if (!gameRunning || gameOver)
    {
        return;
    }

    gameTickCounter++;
}

bool Model::consumeFireRequest()
{
    const bool requested = fireRequested;
    fireRequested = false;
    return requested;
}

void Model::addScore(uint16_t points)
{
    if (!gameRunning || gameOver)
    {
        return;
    }

    score += points;

    if (score > highScore)
    {
        highScore = score;
    }
}

bool Model::loseLife()
{
    if (!gameRunning || gameOver)
    {
        return gameOver;
    }

    if (lives > 0)
    {
        lives--;
    }

    if (lives == 0)
    {
        setGameOver();
    }

    return gameOver;
}

void Model::addLife()
{
    if (!gameRunning || gameOver)
    {
        return;
    }

    if (lives < 9)
    {
        lives++;
    }
}

void Model::nextLevel()
{
    level++;
}

void Model::update(int16_t x)
{
    ImageX = x;
}

int16_t Model::GetImageX()
{
    return ImageX;
}

void Model::startGame()
{
    score = 0;
    gameOver = false;
    gameRunning = true;
    gameSpeed = 2;
    gameTickCounter = 0;
    carDirection = 0;
    lives = PLAYER_START_LIVES;
    level = 1;
}

void Model::stopGame()
{
    gameRunning = false;
    carDirection = 0;
}

void Model::setGameOver()
{
    if (gameOver)
    {
        return;
    }

    gameOver = true;

    if (score > highScore)
    {
        highScore = score;
        saveHighScore(highScore);
    }
}

int8_t Model::getCarDirection() const
{
    return carDirection;
}

uint16_t Model::getScore() const
{
    return score;
}

uint16_t Model::getHighScore() const
{
    return highScore;
}

uint8_t Model::getGameSpeed() const
{
    return gameSpeed;
}

uint8_t Model::getLives() const
{
    return lives;
}

uint16_t Model::getLevel() const
{
    return level;
}

bool Model::isGameOver() const
{
    return gameOver;
}

bool Model::isGameRunning() const
{
    return gameRunning;
}
