#ifndef MODEL_INTEGRATION_EXAMPLE_HPP
#define MODEL_INTEGRATION_EXAMPLE_HPP

#include <gui/model/Model.hpp>
#include "../CoreGame/GameCore.hpp"

// Copy the idea from this file into TouchGFX/gui/include/gui/model/Model.hpp.
// Do not include this file directly unless your include paths are already configured.

class ModelWithSpaceInvaders : public Model
{
public:
    ModelWithSpaceInvaders();
    virtual void tick();
    const si::RenderState& getGameState() const;

private:
    si::GameCore game;
};

#endif
