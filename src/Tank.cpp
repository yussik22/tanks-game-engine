#include "tanks/Tank.hpp"

#include <cmath>
#include "tanks/CollisionSystem.hpp"
#include "tanks/GameMap.hpp"
#include "tanks/Projectile.hpp"

namespace tanks
{

Tank::Tank(const GridPosition& position, int health, Direction facing)
{
    this->position = position;
    this->health = health;
    this->facing = facing;

    if (this->health <= 0)
    {
        this->health = 0;
        deactivate();
    }
}

int Tank::getHealth() const
{
    return health;
}

Direction Tank::getFacing() const
{
    return facing;
}

void Tank::turn(Direction direction)
{
    if (isActive())
    {
        facing = direction;
    }
}

bool Tank::tryMove(const GridPosition& next, const GameMap& map,
                   const CollisionSystem& collisions)
{
    if (!isActive() || !collisions.canTankEnter(*this, next, map))
    {
        return false;
    }

    moveTo(next);
    return true;
}

bool Tank::canFire() const
{
    return isActive() && fireCooldown <= 0.0f && getId() != 0;
}

std::unique_ptr<Projectile> Tank::fire()
{
    if (!canFire())
    {
        return nullptr;
    }

    std::unique_ptr<Projectile> shot =
        std::make_unique<Projectile>(position, facing, 1, getId());
    fireCooldown = reloadSeconds;
    return shot;
}

void Tank::updateCooldown(float dt)
{
    if (dt <= 0.0f || !std::isfinite(dt) || fireCooldown <= 0.0f)
    {
        return;
    }

    fireCooldown -= dt;

    if (fireCooldown <= 0.000001f)
    {
        fireCooldown = 0.0f;
    }
}

void Tank::takeDamage(int amount)
{
    if (amount <= 0 || !isActive())
    {
        return;
    }

    if (amount >= health)
    {
        health = 0;
        deactivate();
        return;
    }

    health -= amount;
}

bool Tank::isDestroyed() const
{
    return health == 0;
}

void Tank::moveTo(const GridPosition& next)
{
    position = next;
}

PlayerTank::PlayerTank(const GridPosition& position) : Tank(position, 3)
{
}

ObjectKind PlayerTank::getKind() const
{
    return ObjectKind::PlayerTank;
}

BotTank::BotTank(const GridPosition& position) : Tank(position, 2)
{
}

ObjectKind BotTank::getKind() const
{
    return ObjectKind::BotTank;
}

Command BotTank::chooseCommand(const GameMap& map, const PlayerTank& player,
                               const CollisionSystem& collisions)
{
    return ai.chooseCommand(*this, map, player, collisions);
}

} // namespace tanks
