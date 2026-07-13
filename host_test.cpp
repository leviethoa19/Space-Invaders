#include <stdio.h>
#include "CoreGame/GameCore.hpp"

int main()
{
    si::GameCore game;
    si::InputState input = {};

    input.start = true;
    game.update(input);
    input.start = false;

    for (int t = 0; t < 800; ++t)
    {
        input.left = false;
        input.right = (t / 30) % 2 == 0;
        input.shoot = true;
        input.pause = false;
        game.update(input);

        const si::RenderState& s = game.renderState();
        if (t % 100 == 0)
        {
            printf("tick=%d mode=%d score=%d level=%d hp=%d lives=%d enemies=%d\n",
                   t, s.mode, s.score, s.level, s.player.hp, s.player.lives, s.enemyCount);
        }
    }

    return 0;
}
