#include "tanks/Tiles.hpp"

namespace tanks
{

FloorTile::FloorTile(const GridPosition& position)
{
    this->position = position;
}

ObjectKind FloorTile::getKind() const
{
    return ObjectKind::FloorTile;
}

WaterTile::WaterTile(const GridPosition& position)
{
    this->position = position;
}

ObjectKind WaterTile::getKind() const
{
    return ObjectKind::WaterTile;
}

} // namespace tanks
