#include <gui/screen2_screen/Screen2View.hpp>
#include <gui/screen2_screen/Screen2Presenter.hpp>

Screen2Presenter::Screen2Presenter(Screen2View& v)
    : view(v)
{
}

void Screen2Presenter::activate()
{
    model->startGame();
    view.updateScoreTexts();
}

void Screen2Presenter::deactivate()
{
    model->stopGame();
}

void Screen2Presenter::UpdateImageX(int16_t x)
{
    model->update(x);
}

int16_t Screen2Presenter::GetImageX()
{
    return model->GetImageX();
}

void Screen2Presenter::startGame()
{
    model->startGame();
}

void Screen2Presenter::stopGame()
{
    model->stopGame();
}

void Screen2Presenter::setGameOver()
{
    model->setGameOver();
}

void Screen2Presenter::addScore(uint16_t points)
{
    model->addScore(points);
}

bool Screen2Presenter::loseLife()
{
    return model->loseLife();
}

void Screen2Presenter::addLife()
{
    model->addLife();
}

void Screen2Presenter::nextLevel()
{
    model->nextLevel();
}

int8_t Screen2Presenter::getCarDirection() const
{
    return model->getCarDirection();
}

bool Screen2Presenter::consumeFireRequest()
{
    return model->consumeFireRequest();
}

uint16_t Screen2Presenter::getScore() const
{
    return model->getScore();
}

uint16_t Screen2Presenter::getHighScore() const
{
    return model->getHighScore();
}

uint8_t Screen2Presenter::getGameSpeed() const
{
    return model->getGameSpeed();
}

uint8_t Screen2Presenter::getLives() const
{
    return model->getLives();
}

uint16_t Screen2Presenter::getLevel() const
{
    return model->getLevel();
}

bool Screen2Presenter::isGameOver() const
{
    return model->isGameOver();
}
