#include "tanks/Wall.hpp"

namespace tanks
{

Wall::Wall(const GridPosition& position, int durability)
{
    this->position = position;
    this->durability = durability;

    if (this->durability <= 0)
    {
        this->durability = 0;
        deactivate();
    }
}

ObjectKind Wall::getKind() const
{
    return ObjectKind::Wall;
}

int Wall::getDurability() const
{
    return durability;
}

void Wall::takeDamage(int amount)
{
    if (amount <= 0 || isDestroyed())
    {
        return;
    }

    if (amount >= durability)
    {
        durability = 0;
        deactivate();
        return;
    }

    durability -= amount;
}

bool Wall::isDestroyed() const
{
    return durability == 0;
}

} // namespace tanks
