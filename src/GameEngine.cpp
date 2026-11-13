#include "tanks/GameEngine.hpp"

#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <utility>

namespace tanks
{

GameEngine::GameEngine(std::uint32_t initialSeed)
{
    this->map = GameMap::generate(initialSeed);
    this->nextMapSeed = initialSeed + 1;
}

void GameEngine::handle(const Command& command)
{
    switch (state)
    {
    case GameState::Ready:
        if (command.kind == CommandKind::Start)
        {
            start();
        }
        break;
    case GameState::Running:
    {
        PlayerTank* player = map->getPlayer();

        if (player == nullptr)
        {
            break;
        }

        switch (command.kind)
        {
        case CommandKind::Move:
            if (playerMoveReady)
            {
                executeCommand(*player, command);
                playerMoveReady = false;

                for (Projectile* shot : map->getActiveProjectiles())
                {
                    GridPosition shotPosition = shot->getPosition();
                    GridPosition playerPosition = player->getPosition();

                    if (shotPosition.x == playerPosition.x &&
                        shotPosition.y == playerPosition.y)
                    {
                        collisions.resolveProjectileStep(*shot, *map);
                    }
                }

                state = evaluateOutcome();
                if (state != GameState::Running)
                {
                    pendingMove.reset();
                }
                map->removeInactive();
            }
            else
            {
                pendingMove = command;
            }
            break;
        case CommandKind::Turn:
        case CommandKind::Fire:
            executeCommand(*player, command);
            break;
        case CommandKind::Start:
        case CommandKind::AimAndFire:
        case CommandKind::Restart:
            break;
        }
        break;
    }
    case GameState::Won:
    case GameState::Lost:
        if (command.kind == CommandKind::Restart)
        {
            start();
        }
        break;
    }
}

void GameEngine::releaseMove()
{
    pendingMove.reset();
}

void GameEngine::update(float dt)
{
    if (state != GameState::Running || dt <= 0.0f || !std::isfinite(dt))
    {
        return;
    }

    const float maxFrameTime = 3.0f * worldTickSeconds;
    accumulator = std::min(accumulator + dt, maxFrameTime);

    while (state == GameState::Running &&
           accumulator + 0.000001f >= worldTickSeconds)
    {
        accumulator -= worldTickSeconds;

        if (accumulator < 0.0f)
        {
            accumulator = 0.0f;
        }

        simulateTick();
    }
}

GameState GameEngine::getState() const
{
    return state;
}

const GameMap& GameEngine::getMap() const
{
    return *map;
}

void GameEngine::start()
{
    if (state != GameState::Ready)
    {
        map = GameMap::generate(nextMapSeed);
        ++nextMapSeed;
    }
    pendingMove.reset();
    playerMoveReady = true;
    accumulator = 0.0f;
    state = GameState::Running;
}

void GameEngine::simulateTick()
{
    PlayerTank* player = map->getPlayer();

    if (player == nullptr)
    {
        state = GameState::Lost;
        pendingMove.reset();
        map->removeInactive();
        return;
    }

    player->updateCooldown(worldTickSeconds);
    std::vector<BotTank*> bots = map->getBots();

    for (BotTank* bot : bots)
    {
        bot->updateCooldown(worldTickSeconds);
    }

    playerMoveReady = true;

    if (pendingMove)
    {
        Command command = *pendingMove;
        pendingMove.reset();
        executeCommand(*player, command);
        playerMoveReady = false;
    }

    for (BotTank* bot : bots)
    {
        if (bot->isActive())
        {
            Command command = bot->chooseCommand(*map, *player, collisions);
            executeCommand(*bot, command);
        }
    }

    std::vector<Projectile*> shots = map->getActiveProjectiles();
    const int projectileStepsPerTick = 2;

    for (Projectile* shot : shots)
    {
        collisions.resolveProjectileStep(*shot, *map);
    }

    for (Projectile* shot : shots)
    {
        for (int step = 0; step < projectileStepsPerTick && shot->isActive(); ++step)
        {
            shot->step();
            collisions.resolveProjectileStep(*shot, *map);
        }
    }

    state = evaluateOutcome();

    if (state != GameState::Running)
    {
        pendingMove.reset();
    }

    map->removeInactive();
}

void GameEngine::executeCommand(Tank& tank, const Command& command)
{
    switch (command.kind)
    {
    case CommandKind::Move:
        tank.turn(command.direction);
        tank.tryMove(adjacentPosition(tank.getPosition(), command.direction),
                     *map, collisions);
        break;
    case CommandKind::Turn:
        tank.turn(command.direction);
        break;
    case CommandKind::Fire:
    case CommandKind::AimAndFire:
    {
        if (command.kind == CommandKind::AimAndFire)
        {
            tank.turn(command.direction);
        }

        std::unique_ptr<Projectile> shot = tank.fire();

        if (shot && map->addProjectile(std::move(shot)) == nullptr)
        {
            throw std::logic_error("Could not add projectile to map");
        }
        break;
    }
    case CommandKind::Start:
    case CommandKind::Restart:
        break;
    }
}

GameState GameEngine::evaluateOutcome() const
{
    if (map->getPlayer() == nullptr)
    {
        return GameState::Lost;
    }

    if (map->getBots().empty())
    {
        return GameState::Won;
    }

    return GameState::Running;
}

} // namespace tanks
