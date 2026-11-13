#include "GameDisplayData.hpp"

#include "tanks/GameEngine.hpp"
#include "tanks/Wall.hpp"

namespace tanks
{

GameDisplayData makeGameDisplayData(const GameEngine& engine)
{
    const GameMap& map = engine.getMap();
    GameDisplayData display;

    switch (engine.getState())
    {
    case GameState::Ready:
        display.state = "READY - press Enter to start";
        break;
    case GameState::Running:
        display.state = "BATTLE";
        break;
    case GameState::Won:
        display.state = "YOU WON - press R to restart";
        break;
    case GameState::Lost:
        display.state = "YOU LOST - press R to restart";
        break;
    }

    for (int y = 0; y < map.getHeight(); ++y)
    {
        for (int x = 0; x < map.getWidth(); ++x)
        {
            CellDisplayData cell;

            for (const GameObject* object : map.getObjectsAt({x, y}))
            {
                switch (object->getKind())
                {
                case ObjectKind::FloorTile:
                    cell.symbol = '.';
                    break;
                case ObjectKind::WaterTile:
                    cell.symbol = '~';
                    break;
                case ObjectKind::Wall:
                    if (static_cast<const Wall*>(object)->getDurability() == 1)
                    {
                        cell.symbol = '+';
                    }
                    else
                    {
                        cell.symbol = '#';
                    }
                    break;
                case ObjectKind::PlayerTank:
                    cell.symbol = 'P';
                    cell.direction =
                        static_cast<const PlayerTank*>(object)->getFacing();
                    break;
                case ObjectKind::BotTank:
                    cell.direction =
                        static_cast<const BotTank*>(object)->getFacing();
                    if (static_cast<const BotTank*>(object)->getHealth() == 1)
                    {
                        cell.symbol = 'b';
                    }
                    else
                    {
                        cell.symbol = 'B';
                    }
                    break;
                case ObjectKind::Projectile:
                    cell.symbol = '*';
                    break;
                }
            }

            display.cells.push_back(cell);
        }
    }

    const PlayerTank* player = map.getPlayer();
    char facing = '-';
    const char* shotStatus = "unavailable";
    int health = 0;

    if (player != nullptr)
    {
        health = player->getHealth();

        if (player->canFire())
        {
            shotStatus = "ready";
        }
        else
        {
            shotStatus = "reloading";
        }

        switch (player->getFacing())
        {
        case Direction::Up:
            facing = '^';
            break;
        case Direction::Right:
            facing = '>';
            break;
        case Direction::Down:
            facing = 'v';
            break;
        case Direction::Left:
            facing = '<';
            break;
        }
    }

    display.status = "Health: " + std::to_string(health) +
        " | Facing: " + facing +
        " | Shot: " + shotStatus +
        " | Bots: " + std::to_string(map.getBots().size());
    return display;
}

} // namespace tanks
