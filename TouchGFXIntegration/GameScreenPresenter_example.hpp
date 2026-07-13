#ifndef GAME_SCREEN_PRESENTER_EXAMPLE_HPP
#define GAME_SCREEN_PRESENTER_EXAMPLE_HPP

#include <gui/model/ModelListener.hpp>
#include <mvp/Presenter.hpp>
#include "../CoreGame/GameTypes.hpp"

using namespace touchgfx;

class GameScreenView;

class GameScreenPresenter : public touchgfx::Presenter, public ModelListener
{
public:
    GameScreenPresenter(GameScreenView& v);
    virtual ~GameScreenPresenter() {}
    virtual void activate() {}
    virtual void deactivate() {}
    const si::RenderState& getGameState() const;

private:
    GameScreenPresenter();
    GameScreenView& view;
};

#endif
