#pragma once

#include <memory>
#include "tanks/BotAI.hpp"
#include "tanks/Damageable.hpp"
#include "tanks/GameObject.hpp"

namespace tanks {

class CollisionSystem;
class GameMap;
class Projectile;

class Tank : public GameObject, public Damageable
{
public:
    int getHealth() const;
    Direction getFacing() const;
    void turn(Direction direction);
    bool tryMove(const GridPosition& next, const GameMap& map,
                 const CollisionSystem& collisions);
    std::unique_ptr<Projectile> fire();
    void updateCooldown(float dt);
    void takeDamage(int amount) override;
    bool isDestroyed() const override;

protected:
    Tank(const GridPosition& position, int health,
         Direction facing = Direction::Up);

private:
    void moveTo(const GridPosition& next);

    int health;
    Direction facing;
    float fireCooldown = 0.0f;
    float reloadSeconds = 0.5f;
};

class PlayerTank : public Tank
{
public:
    PlayerTank(const GridPosition& position);
    ObjectKind getKind() const override;
};

class BotTank : public Tank
{
public:
    BotTank(const GridPosition& position);
    ObjectKind getKind() const override;
    Command chooseCommand(const GameMap& map, const PlayerTank& player,
                          const CollisionSystem& collisions);

private:
    BotAI ai;
};

} // namespace tanks
