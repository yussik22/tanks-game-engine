#pragma once

#include <cstdint>
#include <memory>
#include <optional>
#include "tanks/CollisionSystem.hpp"
#include "tanks/GameMap.hpp"
#include "tanks/Types.hpp"

namespace tanks
{

class GameEngine
{
public:
    GameEngine(std::uint32_t initialSeed);

    void handle(const Command& command);
    void releaseMove();
    void update(float dt);

    GameState getState() const;
    const GameMap& getMap() const;

private:
    void start();
    void simulateTick();
    void executeCommand(Tank& tank, const Command& command);
    GameState evaluateOutcome() const;

    GameState state = GameState::Ready;
    std::unique_ptr<GameMap> map;
    CollisionSystem collisions;
    std::uint32_t nextMapSeed = 0;
    std::optional<Command> pendingMove;
    bool playerMoveReady = true;
    float accumulator = 0.0f;
};

} // namespace tanks
