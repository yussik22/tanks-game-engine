#pragma once

namespace tanks
{

inline constexpr float worldTickSeconds = 0.25f;

struct GridPosition
{
    int x = 0;
    int y = 0;
};

enum class Direction
{
    Up,
    Right,
    Down,
    Left
};

inline GridPosition adjacentPosition(const GridPosition& position, Direction direction)
{
    GridPosition next = position;

    switch (direction)
    {
    case Direction::Up:
        next.y -= 1;
        break;
    case Direction::Right:
        next.x += 1;
        break;
    case Direction::Down:
        next.y += 1;
        break;
    case Direction::Left:
        next.x -= 1;
        break;
    }

    return next;
}

enum class GameState
{
    Ready,
    Running,
    Won,
    Lost
};

enum class CommandKind
{
    Start,
    Move,
    Turn,
    Fire,
    AimAndFire,
    Restart
};

struct Command
{
    CommandKind kind;
    Direction direction = Direction::Up;
};

} // namespace tanks
