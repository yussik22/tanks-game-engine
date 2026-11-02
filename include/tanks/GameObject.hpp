#pragma once

#include <cstdint>
#include "tanks/Types.hpp"

namespace tanks
{

enum class ObjectKind
{
    FloorTile,
    WaterTile,
    Wall,
    PlayerTank,
    BotTank,
    Projectile
};

class GameObject
{
public:
    virtual ~GameObject()
    {
    }

    virtual ObjectKind getKind() const = 0;

    std::uint32_t getId() const
    {
        return id;
    }

    GridPosition getPosition() const
    {
        return position;
    }

    bool isActive() const
    {
        return active;
    }

    void deactivate()
    {
        active = false;
    }

protected:
    GameObject()
    {
    }

    GridPosition position;

private:
    friend class GameMap;

    void setId(std::uint32_t id)
    {
        this->id = id;
    }

    std::uint32_t id = 0;
    bool active = true;
};

} // namespace tanks
