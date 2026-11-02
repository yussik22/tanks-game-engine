#include "tanks/KeyboardInput.hpp"

#include <cctype>

namespace tanks
{

std::optional<Command> KeyboardInput::parse(const std::string& key) const
{
    if (key.empty() || key == "start")
    {
        return Command{CommandKind::Start};
    }

    if (key.size() != 1)
    {
        return std::nullopt;
    }

    char letter = static_cast<char>(std::tolower(static_cast<unsigned char>(key[0])));

    switch (letter)
    {
    case 'w':
        return Command{CommandKind::Move, Direction::Up};
    case 'd':
        return Command{CommandKind::Move, Direction::Right};
    case 's':
        return Command{CommandKind::Move, Direction::Down};
    case 'a':
        return Command{CommandKind::Move, Direction::Left};
    case 'i':
        return Command{CommandKind::Turn, Direction::Up};
    case 'l':
        return Command{CommandKind::Turn, Direction::Right};
    case 'k':
        return Command{CommandKind::Turn, Direction::Down};
    case 'j':
        return Command{CommandKind::Turn, Direction::Left};
    case 'f':
        return Command{CommandKind::Fire};
    case 'r':
        return Command{CommandKind::Restart};
    default:
        return std::nullopt;
    }
}

} // namespace tanks
