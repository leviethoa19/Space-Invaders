#ifndef SCREEN1VIEW_HPP
#define SCREEN1VIEW_HPP

#include <gui_generated/screen1_screen/Screen1ViewBase.hpp>
#include <gui/screen1_screen/Screen1Presenter.hpp>
#include <touchgfx/Unicode.hpp>

class Screen1View : public Screen1ViewBase
{
public:
    Screen1View();
    virtual ~Screen1View() {}
    virtual void setupScreen();
    virtual void tearDownScreen();
    virtual void handleTickEvent();
protected:
    long tickCounter = 0;

private:
    static const uint16_t HISCORE_TEXT_BUFFER_SIZE = 16;
    touchgfx::Unicode::UnicodeChar hiScoreBuffer[HISCORE_TEXT_BUFFER_SIZE];
};

#endif // SCREEN1VIEW_HPP
