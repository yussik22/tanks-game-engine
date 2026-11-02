#include "tanks/Projectile.hpp"

namespace tanks
{

Projectile::Projectile(const GridPosition& position, Direction direction,
                       int damage, std::uint32_t ownerId)
{
    this->position = position;
    this->direction = direction;
    this->damage = damage;
    this->ownerId = ownerId;
}

ObjectKind Projectile::getKind() const
{
    return ObjectKind::Projectile;
}

void Projectile::step()
{
    position = adjacentPosition(position, direction);
}

int Projectile::getDamage() const
{
    return damage;
}

std::uint32_t Projectile::getOwnerId() const
{
    return ownerId;
}

} // namespace tanks
