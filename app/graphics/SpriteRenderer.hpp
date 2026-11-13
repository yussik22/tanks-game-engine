#pragma once

#include <SFML/Graphics.hpp>
#include <string>
#include "PixelTextures.hpp"
#include "tanks/Renderer.hpp"
#include "tanks/Types.hpp"

class SpriteRenderer : public tanks::Renderer
{
public:
    static constexpr int cellSize = 44;
    static constexpr int headerHeight = 70;
    static constexpr int footerHeight = 56;

    SpriteRenderer(sf::RenderWindow& window);
    void render(const tanks::GameEngine& engine) const override;

private:
    void drawSprite(const sf::Texture& texture, float left, float top) const;
    void drawRotatedSprite(const sf::Texture& texture,
                           tanks::Direction direction, float left, float top) const;
    void drawSymbol(char symbol, tanks::Direction direction,
                    float left, float top) const;
    void drawText(const std::string& value, unsigned int size,
                  float left, float top, const sf::Color& color) const;

    sf::RenderWindow* window = nullptr;
    sf::Font font;
    PixelTextures textures;
};
