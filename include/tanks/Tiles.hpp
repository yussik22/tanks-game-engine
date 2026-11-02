#pragma once

#include "tanks/GameObject.hpp"

namespace tanks
{

class FloorTile : public GameObject
{
public:
    FloorTile(const GridPosition& position);
    ObjectKind getKind() const override;
};

class WaterTile : public GameObject
{
public:
    WaterTile(const GridPosition& position);
    ObjectKind getKind() const override;
};

} // namespace tanks
