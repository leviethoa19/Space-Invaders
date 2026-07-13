#include <gui/screen1_screen/Screen1View.hpp>

Screen1View::Screen1View()
{

}

void Screen1View::setupScreen()
{
    Screen1ViewBase::setupScreen();

    touchgfx::Unicode::itoa((int32_t)presenter->getHighScore(), hiScoreBuffer, HISCORE_TEXT_BUFFER_SIZE, 10);
    hiScoreText.setWildcard(hiScoreBuffer);
    hiScoreText.resizeToCurrentTextWithAlignment();
    hiScoreText.invalidateContent();
}

void Screen1View::tearDownScreen()
{
    Screen1ViewBase::tearDownScreen();
}

void Screen1View::handleTickEvent()
{
	tickCounter += 1;

}
