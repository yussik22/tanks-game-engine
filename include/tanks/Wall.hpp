#pragma once

#include "tanks/Damageable.hpp"
#include "tanks/GameObject.hpp"

namespace tanks {

class Wall : public GameObject, public Damageable
{
public:
    Wall(const GridPosition& position, int durability = 2);

    ObjectKind getKind() const override;
    int getDurability() const;
    void takeDamage(int amount) override;
    bool isDestroyed() const override;

private:
    int durability;
};

} // namespace tanks
