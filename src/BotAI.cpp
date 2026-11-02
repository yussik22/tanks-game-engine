#include "tanks/BotAI.hpp"

#include "tanks/CollisionSystem.hpp"
#include "tanks/GameMap.hpp"
#include "tanks/Tank.hpp"

namespace tanks
{

Command BotAI::chooseCommand(const BotTank& bot, const GameMap& map,
                             const PlayerTank& player,
                             const CollisionSystem& collisions)
{
    GridPosition botPosition = bot.getPosition();
    GridPosition playerPosition = player.getPosition();

    if (map.hasClearLineOfSight(botPosition, playerPosition))
    {
        Direction towardPlayer;

        if (playerPosition.x == botPosition.x)
        {
            towardPlayer = playerPosition.y > botPosition.y ?
                Direction::Down : Direction::Up;
        }
        else
        {
            towardPlayer = playerPosition.x > botPosition.x ?
                Direction::Right : Direction::Left;
        }

        return {CommandKind::AimAndFire, towardPlayer};
    }

    for (int offset = 0; offset < 4; ++offset)
    {
        int index = (static_cast<int>(patrolDirection) + offset) % 4;
        Direction candidate = static_cast<Direction>(index);
        GridPosition next = adjacentPosition(botPosition, candidate);

        if (collisions.canTankEnter(bot, next, map))
        {
            patrolDirection = candidate;
            return {CommandKind::Move, candidate};
        }
    }

    return {CommandKind::Turn, patrolDirection};
}

} // namespace tanks
