#include <SFML/Graphics.hpp>
#include <exception>
#include <iostream>
#include "SpriteRenderer.hpp"
#include "WindowInput.hpp"
#include "tanks/GameEngine.hpp"

using namespace tanks;

int main()
{
    try
    {
        GameEngine engine(42);
        const GameMap& map = engine.getMap();
        unsigned int width = static_cast<unsigned int>(
            map.getWidth() * SpriteRenderer::cellSize);
        unsigned int height = static_cast<unsigned int>(
            SpriteRenderer::headerHeight +
            map.getHeight() * SpriteRenderer::cellSize +
            SpriteRenderer::footerHeight);
        sf::RenderWindow window(sf::VideoMode(width, height),
                                "Tanks - Sprite Renderer",
                                sf::Style::Titlebar | sf::Style::Close);
        window.setFramerateLimit(60);
        window.setKeyRepeatEnabled(false);

        WindowInput input;
        SpriteRenderer renderer(window);
        sf::Clock clock;

        while (window.isOpen())
        {
            bool started = false;
            sf::Event event;

            while (window.pollEvent(event))
            {
                if (input.handleEvent(event, engine, window))
                {
                    started = true;
                }
            }

            if (!window.isOpen())
            {
                break;
            }

            input.updateHeldMove(engine, window);
            float dt = clock.restart().asSeconds();

            if (!started)
            {
                engine.update(dt);
            }

            renderer.render(engine);
        }

        return 0;
    }
    catch (const std::exception& error)
    {
        std::cerr << "Could not start graphical Tanks: " << error.what() << '\n';
        return 1;
    }
}
