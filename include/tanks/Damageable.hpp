#pragma once

namespace tanks {

class Damageable
{
public:
    virtual ~Damageable() {}

    virtual void takeDamage(int amount) = 0;
    virtual bool isDestroyed() const = 0;
};

} // namespace tanks
