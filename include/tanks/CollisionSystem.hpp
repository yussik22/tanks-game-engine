#pragma once

#include "tanks/Damageable.hpp"
#include "tanks/Types.hpp"

namespace tanks {

class GameMap;
class Projectile;
class Tank;

class CollisionSystem
{
public:
    bool canTankEnter(const Tank& tank, const GridPosition& next,
                      const GameMap& map) const;
    void resolveProjectileStep(Projectile& shot, GameMap& map) const;

private:
    Damageable* findFirstProjectileTarget(const Projectile& shot,
                                          GameMap& map) const;
};

} // namespace tanks
