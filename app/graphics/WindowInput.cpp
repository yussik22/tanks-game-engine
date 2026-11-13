#include "WindowInput.hpp"

#include <optional>
#include <string>

using namespace tanks;

namespace
{

std::optional<std::string> keyName(sf::Keyboard::Key key)
{
    switch (key)
    {
    case sf::Keyboard::Return:
        return "";
    case sf::Keyboard::W:
        return "w";
    case sf::Keyboard::A:
        return "a";
    case sf::Keyboard::S:
        return "s";
    case sf::Keyboard::D:
        return "d";
    case sf::Keyboard::I:
    case sf::Keyboard::Up:
        return "i";
    case sf::Keyboard::J:
    case sf::Keyboard::Left:
        return "j";
    case sf::Keyboard::K:
    case sf::Keyboard::Down:
        return "k";
    case sf::Keyboard::L:
    case sf::Keyboard::Right:
        return "l";
    case sf::Keyboard::F:
    case sf::Keyboard::Space:
        return "f";
    case sf::Keyboard::R:
        return "r";
    default:
        return std::nullopt;
    }
}

bool isMovementKey(sf::Keyboard::Key key)
{
    return key == sf::Keyboard::W || key == sf::Keyboard::A ||
           key == sf::Keyboard::S || key == sf::Keyboard::D;
}

} // namespace

bool WindowInput::handleEvent(const sf::Event& event, GameEngine& engine,
                              sf::RenderWindow& window)
{
    if (event.type == sf::Event::Closed)
    {
        window.close();
        return false;
    }

    if (event.type == sf::Event::KeyReleased &&
        event.key.code == recentMoveKey)
    {
        recentMoveKey = sf::Keyboard::Unknown;
        return false;
    }

    if (event.type != sf::Event::KeyPressed)
    {
        return false;
    }

    if (event.key.code == sf::Keyboard::Escape ||
        event.key.code == sf::Keyboard::Q)
    {
        window.close();
        return false;
    }

    if (isMovementKey(event.key.code))
    {
        recentMoveKey = event.key.code;
    }

    std::optional<std::string> key = keyName(event.key.code);

    if (!key)
    {
        return false;
    }

    GameState before = engine.getState();
    std::optional<Command> command = input.parse(*key);

    if (command)
    {
        engine.handle(*command);
    }

    return before != GameState::Running &&
           engine.getState() == GameState::Running;
}

void WindowInput::updateHeldMove(GameEngine& engine,
                                 const sf::RenderWindow& window)
{
    sf::Keyboard::Key heldMove = sf::Keyboard::Unknown;

    if (window.hasFocus())
    {
        if (recentMoveKey != sf::Keyboard::Unknown &&
            sf::Keyboard::isKeyPressed(recentMoveKey))
        {
            heldMove = recentMoveKey;
        }
        else if (sf::Keyboard::isKeyPressed(sf::Keyboard::W))
        {
            heldMove = sf::Keyboard::W;
        }
        else if (sf::Keyboard::isKeyPressed(sf::Keyboard::A))
        {
            heldMove = sf::Keyboard::A;
        }
        else if (sf::Keyboard::isKeyPressed(sf::Keyboard::S))
        {
            heldMove = sf::Keyboard::S;
        }
        else if (sf::Keyboard::isKeyPressed(sf::Keyboard::D))
        {
            heldMove = sf::Keyboard::D;
        }
    }

    if (heldMove != sf::Keyboard::Unknown)
    {
        recentMoveKey = heldMove;
        std::optional<std::string> key = keyName(heldMove);

        if (key)
        {
            std::optional<Command> command = input.parse(*key);

            if (command)
            {
                engine.handle(*command);
            }
        }
    }
    else if (wasHoldingMove)
    {
        engine.releaseMove();
    }

    wasHoldingMove = heldMove != sf::Keyboard::Unknown;
}
