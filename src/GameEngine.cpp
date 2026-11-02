#include "tanks/GameEngine.hpp"

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
        switch (command.kind)
        {
        case CommandKind::Move:
        case CommandKind::Turn:
        case CommandKind::Fire:
            if (pendingCommands.size() >= static_cast<std::size_t>(maxPendingCommands))
            {
                pendingCommands.pop_front();
            }
            pendingCommands.push_back(command);
            break;
        case CommandKind::Start:
        case CommandKind::AimAndFire:
        case CommandKind::Restart:
            break;
        }
        break;
    case GameState::Won:
    case GameState::Lost:
        if (command.kind == CommandKind::Restart)
        {
            start();
        }
        break;
    }
}

void GameEngine::update(float dt)
{
    if (state != GameState::Running || dt <= 0.0f || !std::isfinite(dt))
    {
        return;
    }

    accumulator += dt;

    while (state == GameState::Running &&
           accumulator + 0.000001f >= fixedTickSeconds)
    {
        accumulator -= fixedTickSeconds;

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
    map = GameMap::generate(nextMapSeed);
    ++nextMapSeed;
    pendingCommands.clear();
    accumulator = 0.0f;
    state = GameState::Running;
}

void GameEngine::simulateTick()
{
    PlayerTank* player = map->getPlayer();

    if (player == nullptr)
    {
        state = GameState::Lost;
        pendingCommands.clear();
        map->removeInactive();
        return;
    }

    player->updateCooldown(fixedTickSeconds);
    std::vector<BotTank*> bots = map->getBots();

    for (BotTank* bot : bots)
    {
        bot->updateCooldown(fixedTickSeconds);
    }

    if (!pendingCommands.empty())
    {
        Command command = pendingCommands.front();
        pendingCommands.pop_front();
        executeCommand(*player, command);
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
        for (int step = 0; step < projectileStepsPerTick && shot->isActive(); ++step)
        {
            shot->step();
            collisions.resolveProjectileStep(*shot, *map);
        }
    }

    state = evaluateOutcome();

    if (state != GameState::Running)
    {
        pendingCommands.clear();
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
