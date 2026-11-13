#pragma once

#include <SFML/Graphics.hpp>
#include "tanks/GameEngine.hpp"
#include "tanks/KeyboardInput.hpp"

class WindowInput
{
public:
    bool handleEvent(const sf::Event& event, tanks::GameEngine& engine,
                     sf::RenderWindow& window);
    void updateHeldMove(tanks::GameEngine& engine,
                        const sf::RenderWindow& window);

private:
    tanks::KeyboardInput input;
    sf::Keyboard::Key recentMoveKey = sf::Keyboard::Unknown;
    bool wasHoldingMove = false;
};
