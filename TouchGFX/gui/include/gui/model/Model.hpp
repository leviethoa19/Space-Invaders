#ifndef MODEL_HPP
#define MODEL_HPP

#include <stdint.h>

class ModelListener;

class Model
{
public:
    Model();

    void bind(ModelListener* listener)
    {
        modelListener = listener;
    }

    void tick();
    void update(int16_t x);
    int16_t GetImageX();

    void startGame();
    void stopGame();
    void setGameOver();
    void addScore(uint16_t points);
    bool loseLife();
    void addLife();
    void nextLevel();

    int8_t getCarDirection() const;
    bool consumeFireRequest();
    uint16_t getScore() const;
    uint16_t getHighScore() const;
    uint8_t getGameSpeed() const;
    uint8_t getLives() const;
    uint16_t getLevel() const;
    bool isGameOver() const;
    bool isGameRunning() const;

    int16_t ImageX;

protected:
    ModelListener* modelListener;

private:
    int8_t carDirection;
    bool fireRequested;
    uint16_t score;
    uint16_t highScore;
    uint8_t gameSpeed;
    uint32_t gameTickCounter;
    uint8_t lives;
    uint16_t level;
    bool gameOver;
    bool gameRunning;
};

#endif // MODEL_HPP
