#pragma once

#include <cstdint>
#include "tanks/GameObject.hpp"

namespace tanks
{

class Projectile : public GameObject
{
public:
    Projectile(const GridPosition& position, Direction direction,
               int damage, std::uint32_t ownerId);

    ObjectKind getKind() const override;
    void step();
    int getDamage() const;
    std::uint32_t getOwnerId() const;

private:
    Direction direction;
    int damage;
    std::uint32_t ownerId;
};

} // namespace tanks
