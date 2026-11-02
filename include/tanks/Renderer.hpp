#pragma once

namespace tanks
{

class GameEngine;

class Renderer
{
public:
    virtual ~Renderer()
    {
    }
    virtual void render(const GameEngine& engine) const = 0;
};

} // namespace tanks
