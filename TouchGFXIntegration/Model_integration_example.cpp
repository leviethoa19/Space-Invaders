#include "Model_integration_example.hpp"
#include "../STM32HAL/SpaceInvadersHw.hpp"

ModelWithSpaceInvaders::ModelWithSpaceInvaders() : Model()
{
    si::HW_Init();
}

void ModelWithSpaceInvaders::tick()
{
    si::InputState input = si::HW_ReadInput();
    game.update(input);
    si::HW_HandleSoundAndLed(game.consumeSoundEvent());
}

const si::RenderState& ModelWithSpaceInvaders::getGameState() const
{
    return game.renderState();
}
