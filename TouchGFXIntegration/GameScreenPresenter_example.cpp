#include <gui/game_screen/GameScreenView.hpp>
#include <gui/game_screen/GameScreenPresenter.hpp>
#include <gui/model/Model.hpp>

GameScreenPresenter::GameScreenPresenter(GameScreenView& v) : view(v)
{
}

const si::RenderState& GameScreenPresenter::getGameState() const
{
    return model->getGameState();
}
