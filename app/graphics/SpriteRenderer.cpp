#include "SpriteRenderer.hpp"

#include <cstdlib>
#include <stdexcept>
#include <vector>
#include "GameDisplayData.hpp"
#include "tanks/GameEngine.hpp"

using namespace tanks;

SpriteRenderer::SpriteRenderer(sf::RenderWindow& window)
{
    this->window = &window;

    std::vector<std::string> fontPaths;
    const char* windowsDirectory = std::getenv("WINDIR");

    if (windowsDirectory != nullptr)
    {
        fontPaths.push_back(std::string(windowsDirectory) + "/Fonts/consola.ttf");
        fontPaths.push_back(std::string(windowsDirectory) + "/Fonts/lucon.ttf");
        fontPaths.push_back(std::string(windowsDirectory) + "/Fonts/cour.ttf");
    }

    fontPaths.push_back("/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf");
    fontPaths.push_back("/usr/share/fonts/dejavu/DejaVuSansMono.ttf");

    for (const std::string& path : fontPaths)
    {
        if (font.loadFromFile(path))
        {
            return;
        }
    }

    throw std::runtime_error("No monospace font was found");
}

void SpriteRenderer::render(const GameEngine& engine) const
{
    const GameMap& map = engine.getMap();
    GameDisplayData display = makeGameDisplayData(engine);
    window->clear(sf::Color(20, 27, 36));
    drawText(display.state, 22, 12.0f, 4.0f,
             sf::Color(235, 244, 252));

    std::size_t index = 0;
    for (int y = 0; y < map.getHeight(); ++y)
    {
        for (int x = 0; x < map.getWidth(); ++x)
        {
            const CellDisplayData& cell = display.cells[index];
            ++index;
            drawSymbol(cell.symbol, cell.direction,
                       static_cast<float>(x * cellSize),
                       static_cast<float>(headerHeight + y * cellSize));
        }
    }

    drawText(display.status, 16, 12.0f, 38.0f,
             sf::Color(167, 222, 199));
    float footerTop = static_cast<float>(
        headerHeight + map.getHeight() * cellSize);
    drawText("WASD move | Arrows/IJKL aim | F/Space fire", 14,
             12.0f, footerTop + 2.0f, sf::Color(199, 215, 228));
    drawText("Enter start | R restart | Esc/Q quit", 14,
             12.0f, footerTop + 26.0f, sf::Color(199, 215, 228));
    window->display();
}

void SpriteRenderer::drawSprite(const sf::Texture& texture,
                                float left, float top) const
{
    sf::Sprite sprite(texture);
    float scale = static_cast<float>(cellSize) /
                  static_cast<float>(PixelTextures::size);
    sprite.setScale(scale, scale);
    sprite.setPosition(left, top);
    window->draw(sprite);
}

void SpriteRenderer::drawRotatedSprite(const sf::Texture& texture,
                                       Direction direction,
                                       float left, float top) const
{
    float rotation = 0.0f;

    switch (direction)
    {
    case Direction::Up:
        break;
    case Direction::Right:
        rotation = 90.0f;
        break;
    case Direction::Down:
        rotation = 180.0f;
        break;
    case Direction::Left:
        rotation = 270.0f;
        break;
    }

    sf::Sprite sprite(texture);
    float scale = static_cast<float>(cellSize) /
                  static_cast<float>(PixelTextures::size);
    sprite.setOrigin(PixelTextures::size / 2.0f,
                     PixelTextures::size / 2.0f);
    sprite.setScale(scale, scale);
    sprite.setPosition(left + cellSize / 2.0f,
                       top + cellSize / 2.0f);
    sprite.setRotation(rotation);
    window->draw(sprite);
}

void SpriteRenderer::drawSymbol(char symbol, Direction direction,
                                float left, float top) const
{
    drawSprite(textures.floor, left, top);

    switch (symbol)
    {
    case '.':
        break;
    case '~':
        drawSprite(textures.water, left, top);
        break;
    case '#':
        drawSprite(textures.wall, left, top);
        break;
    case '+':
        drawSprite(textures.damagedWall, left, top);
        break;
    case 'P':
        drawRotatedSprite(textures.player, direction, left, top);
        break;
    case 'B':
        drawRotatedSprite(textures.bot, direction, left, top);
        break;
    case 'b':
        drawRotatedSprite(textures.damagedBot, direction, left, top);
        break;
    case '*':
        drawSprite(textures.projectile, left, top);
        break;
    }
}

void SpriteRenderer::drawText(const std::string& value, unsigned int size,
                              float left, float top,
                              const sf::Color& color) const
{
    sf::Text text(value, font, size);
    text.setPosition(left, top);
    text.setFillColor(color);
    window->draw(text);
}
