#pragma once

namespace tanks {

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
