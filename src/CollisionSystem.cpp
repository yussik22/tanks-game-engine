#include "tanks/CollisionSystem.hpp"

#include <cstdlib>
#include "tanks/GameMap.hpp"
#include "tanks/Projectile.hpp"
#include "tanks/Tank.hpp"

namespace tanks
{

bool CollisionSystem::canTankEnter(const Tank& tank, const GridPosition& next,
                                   const GameMap& map) const
{
    if (!map.inBounds(next) || map.hasWaterAt(next))
    {
        return false;
    }

    GridPosition current = tank.getPosition();
    int distance = std::abs(next.x - current.x) + std::abs(next.y - current.y);

    if (distance != 1)
    {
        return false;
    }

    for (const GameObject* object : map.getWallObjectsAt(next))
    {
        switch (object->getKind())
        {
        case ObjectKind::Wall:
        case ObjectKind::PlayerTank:
        case ObjectKind::BotTank:
            return false;
        case ObjectKind::Projectile:
        case ObjectKind::FloorTile:
        case ObjectKind::WaterTile:
            break;
        }
    }

    return true;
}

void CollisionSystem::resolveProjectileStep(Projectile& shot, GameMap& map) const
{
    if (!shot.isActive())
    {
        return;
    }

    if (!map.inBounds(shot.getPosition()))
    {
        shot.deactivate();
        return;
    }

    Damageable* target = findFirstProjectileTarget(shot, map);

    if (target != nullptr)
    {
        target->takeDamage(shot.getDamage());
        shot.deactivate();
    }
}

Damageable* CollisionSystem::findFirstProjectileTarget(const Projectile& shot,
                                                        GameMap& map) const
{
    for (GameObject* object : map.getWallObjectsAt(shot.getPosition()))
    {
        if (object == &shot || object->getId() == shot.getOwnerId())
        {
            continue;
        }

        Damageable* target = dynamic_cast<Damageable*>(object);

        if (target != nullptr)
        {
            return target;
        }
    }

    return nullptr;
}

} // namespace tanks
