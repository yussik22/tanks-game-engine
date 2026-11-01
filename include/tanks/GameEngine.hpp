#pragma once

#include <cstdint>
#include <deque>
#include <memory>
#include "tanks/CollisionSystem.hpp"
#include "tanks/GameMap.hpp"
#include "tanks/Types.hpp"

namespace tanks {

class GameEngine
{
public:
    GameEngine(std::uint32_t initialSeed);

    void handle(const Command& command);
    void update(float dt);

    GameState getState() const;
    const GameMap& getMap() const;

private:
    void start();
    void simulateTick();
    GameState evaluateOutcome() const;

    GameState state = GameState::Ready;
    std::unique_ptr<GameMap> map;
    CollisionSystem collisions;
    std::uint32_t nextMapSeed = 0;
    std::deque<Command> pendingCommands;
    int maxPendingCommands = 4;
    float accumulator = 0.0f;
    float fixedTickSeconds = 0.1f;
};

} // namespace tanks
