#include <iostream>
#include <string>
#ifdef _WIN32
#include <conio.h>
#endif
#include "tanks/GameEngine.hpp"
#include "tanks/KeyboardInput.hpp"
#include "tanks/Renderer.hpp"
#include "tanks/Wall.hpp"

using namespace tanks;

class ConsoleRenderer : public Renderer
{
public:
    void render(const GameEngine& engine) const override
    {
        const GameMap& map = engine.getMap();

        switch (engine.getState())
        {
        case GameState::Ready:
            std::cout << "\nREADY - press Enter to start\n";
            break;
        case GameState::Running:
            std::cout << "\nBATTLE\n";
            break;
        case GameState::Won:
            std::cout << "\nYOU WON - press R to restart\n";
            break;
        case GameState::Lost:
            std::cout << "\nYOU LOST - press R to restart\n";
            break;
        }

        for (int y = 0; y < map.getHeight(); ++y)
        {
            for (int x = 0; x < map.getWidth(); ++x)
            {
                char symbol = ' ';

                for (const GameObject* object : map.getObjectsAt({x, y}))
                {
                    switch (object->getKind())
                    {
                    case ObjectKind::FloorTile:
                        symbol = '.';
                        break;
                    case ObjectKind::WaterTile:
                        symbol = '~';
                        break;
                    case ObjectKind::Wall:
                        symbol = static_cast<const Wall*>(object)->getDurability() == 1
                            ? '+' : '#';
                        break;
                    case ObjectKind::PlayerTank:
                        symbol = 'P';
                        break;
                    case ObjectKind::BotTank:
                        symbol = static_cast<const BotTank*>(object)->getHealth() == 1
                            ? 'b' : 'B';
                        break;
                    case ObjectKind::Projectile:
                        symbol = '*';
                        break;
                    }
                }

                std::cout << symbol << ' ';
            }

            std::cout << '\n';
        }

        const PlayerTank* player = map.getPlayer();
        char facing = '-';
        const char* shotStatus = "unavailable";

        if (player != nullptr)
        {
            shotStatus = player->canFire() ? "ready" : "reloading";
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

        std::cout << "Health: " << (player == nullptr ? 0 : player->getHealth())
                  << " | Facing: " << facing
                  << " | Shot: " << shotStatus
                  << " | Bots: " << map.getBots().size() << '\n';
        std::cout << "WASD move | IJKL turn | F fire | Enter wait | R restart | Q quit\n";
    }
};

bool readInput(std::string& line)
{
#ifdef _WIN32
    int key = _getch();

    if (key == 0 || key == 224)
    {
        _getch();
        line = "?";
    }
    else if (key == 13)
    {
        line.clear();
    }
    else
    {
        line = static_cast<char>(key);
    }

    return true;
#else
    return static_cast<bool>(std::getline(std::cin, line));
#endif
}

int main()
{
    GameEngine engine(42);
    KeyboardInput input;
    ConsoleRenderer renderer;
    renderer.render(engine);

    std::string line;

    while (true)
    {
        if (!readInput(line) || line == "q" ||
            line == "Q" || line == "quit")
        {
            break;
        }

        GameState previousState = engine.getState();
        std::optional<Command> command = input.parse(line);

        if (command)
        {
            engine.handle(*command);
        }

        bool started = previousState != GameState::Running &&
                       engine.getState() == GameState::Running;
        engine.update(started ? 0.0f : worldTickSeconds);
        renderer.render(engine);
    }

    return 0;
}
