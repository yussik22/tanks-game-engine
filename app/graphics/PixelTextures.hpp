#pragma once

#include <SFML/Graphics.hpp>

struct PixelTextures
{
    static constexpr int size = 16;

    PixelTextures();

    sf::Texture floor;
    sf::Texture water;
    sf::Texture wall;
    sf::Texture damagedWall;
    sf::Texture player;
    sf::Texture bot;
    sf::Texture damagedBot;
    sf::Texture projectile;
};
