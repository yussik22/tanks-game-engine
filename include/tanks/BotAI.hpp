#pragma once

#include "tanks/Types.hpp"

namespace tanks {

class BotTank;
class CollisionSystem;
class GameMap;
class PlayerTank;

class BotAI
{
public:
    Command chooseCommand(const BotTank& bot, const GameMap& map,
                          const PlayerTank& player,
                          const CollisionSystem& collisions);

private:
    Direction patrolDirection = Direction::Up;
};

} // namespace tanks
