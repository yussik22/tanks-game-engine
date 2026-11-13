#pragma once

#include <string>
#include <vector>
#include "tanks/Types.hpp"

namespace tanks
{

class GameEngine;

struct CellDisplayData
{
    char symbol = ' ';
    Direction direction = Direction::Up;
};

struct GameDisplayData
{
    std::string state;
    std::vector<CellDisplayData> cells;
    std::string status;
};

GameDisplayData makeGameDisplayData(const GameEngine& engine);

} // namespace tanks
