#ifndef SCREEN2PRESENTER_HPP
#define SCREEN2PRESENTER_HPP

#include <gui/model/ModelListener.hpp>
#include <mvp/Presenter.hpp>

using namespace touchgfx;

class Screen2View;

class Screen2Presenter : public touchgfx::Presenter, public ModelListener
{
public:
    Screen2Presenter(Screen2View& v);

    virtual void activate();
    virtual void deactivate();
    virtual ~Screen2Presenter() {}

    void UpdateImageX(int16_t x);
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

private:
    Screen2Presenter();

    Screen2View& view;
};

#endif // SCREEN2PRESENTER_HPP
