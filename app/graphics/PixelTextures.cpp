#include "PixelTextures.hpp"

#include <stdexcept>

namespace
{

void paintRect(sf::Image& image, int left, int top, int width, int height,
               const sf::Color& color)
{
    for (int y = top; y < top + height; ++y)
    {
        for (int x = left; x < left + width; ++x)
        {
            image.setPixel(static_cast<unsigned int>(x),
                           static_cast<unsigned int>(y), color);
        }
    }
}

void loadTexture(sf::Texture& texture, const sf::Image& image)
{
    if (!texture.loadFromImage(image))
    {
        throw std::runtime_error("Could not create sprite texture");
    }

    texture.setSmooth(false);
}

sf::Image makeFloor()
{
    sf::Image image;
    image.create(PixelTextures::size, PixelTextures::size, sf::Color(55, 83, 62));
    sf::Color grass(103, 137, 91);
    paintRect(image, 3, 4, 2, 1, grass);
    paintRect(image, 10, 3, 1, 2, grass);
    paintRect(image, 7, 11, 2, 1, grass);
    paintRect(image, 12, 12, 2, 1, grass);
    return image;
}

sf::Image makeWater()
{
    sf::Image image;
    image.create(PixelTextures::size, PixelTextures::size, sf::Color(30, 88, 140));
    sf::Color wave(91, 171, 205);
    paintRect(image, 2, 4, 6, 1, wave);
    paintRect(image, 9, 9, 5, 1, wave);
    paintRect(image, 4, 13, 4, 1, wave);
    return image;
}

sf::Image makeWall(bool damaged)
{
    sf::Image image;
    image.create(PixelTextures::size, PixelTextures::size, sf::Color(74, 62, 61));
    sf::Color brick(166, 111, 82);
    sf::Color light(207, 152, 102);
    paintRect(image, 1, 1, 7, 5, brick);
    paintRect(image, 9, 1, 6, 5, brick);
    paintRect(image, 1, 7, 4, 5, brick);
    paintRect(image, 6, 7, 9, 5, brick);
    paintRect(image, 1, 13, 7, 2, brick);
    paintRect(image, 9, 13, 6, 2, brick);
    paintRect(image, 2, 2, 5, 1, light);
    paintRect(image, 10, 2, 4, 1, light);
    paintRect(image, 7, 8, 7, 1, light);

    if (damaged)
    {
        sf::Color crack(37, 40, 45);
        paintRect(image, 7, 3, 1, 4, crack);
        paintRect(image, 8, 7, 1, 3, crack);
        paintRect(image, 6, 10, 2, 1, crack);
        paintRect(image, 5, 11, 1, 3, crack);
    }

    return image;
}

sf::Image makeTank(const sf::Color& body, const sf::Color& light, bool damaged)
{
    sf::Image image;
    image.create(PixelTextures::size, PixelTextures::size, sf::Color::Transparent);
    sf::Color track(35, 39, 43);
    paintRect(image, 1, 2, 3, 12, track);
    paintRect(image, 12, 2, 3, 12, track);
    paintRect(image, 4, 3, 8, 11, body);
    paintRect(image, 5, 4, 6, 2, light);
    paintRect(image, 6, 6, 4, 6, track);
    paintRect(image, 5, 7, 6, 4, body);
    paintRect(image, 7, 1, 2, 7, light);

    if (damaged)
    {
        paintRect(image, 5, 9, 2, 1, track);
        paintRect(image, 7, 10, 1, 3, track);
    }

    return image;
}

sf::Image makeProjectile()
{
    sf::Image image;
    image.create(PixelTextures::size, PixelTextures::size, sf::Color::Transparent);
    paintRect(image, 5, 5, 6, 6, sf::Color(244, 159, 66));
    paintRect(image, 6, 6, 4, 4, sf::Color(255, 236, 141));
    return image;
}

} // namespace

PixelTextures::PixelTextures()
{
    loadTexture(floor, makeFloor());
    loadTexture(water, makeWater());
    loadTexture(wall, makeWall(false));
    loadTexture(damagedWall, makeWall(true));
    loadTexture(player,
        makeTank(sf::Color(49, 148, 99), sf::Color(135, 225, 157), false));
    loadTexture(bot,
        makeTank(sf::Color(176, 63, 72), sf::Color(241, 139, 116), false));
    loadTexture(damagedBot,
        makeTank(sf::Color(176, 63, 72), sf::Color(241, 139, 116), true));
    loadTexture(projectile, makeProjectile());
}
