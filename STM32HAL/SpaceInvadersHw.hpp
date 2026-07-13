#ifndef SPACE_INVADERS_HW_HPP
#define SPACE_INVADERS_HW_HPP

#include "../CoreGame/GameTypes.hpp"

namespace si
{
void HW_Init();
InputState HW_ReadInput();
void HW_HandleSoundAndLed(SoundEvent event);
}

#endif
