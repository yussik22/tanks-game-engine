#include "tanks/GameMap.hpp"

#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <utility>
#include "tanks/Tiles.hpp"
#include "tanks/Wall.hpp"

namespace tanks
{

namespace
{
const int mapWidth = 13;
const int mapHeight = 13;
const float noiseCellSize = 3.0f;
const float waterThreshold = 0.15f;
const float wallThreshold = 0.35f;

const int routeTopRow = 2;
const int routeBottomRow = 11;
const int routeLeftColumn = 2;
const int routeRightColumn = 10;

const GridPosition playerSpawn{6, 11};
const GridPosition firstBotSpawn{2, 2};
const GridPosition secondBotSpawn{10, 2};
const GridPosition thirdBotSpawn{10, 8};

// Fixed bit-mixing values keep terrain generation repeatable for a given seed.
const std::uint32_t xNoiseMultiplier = 0x9e3779b9u;
const std::uint32_t yNoiseMultiplier = 0x85ebca6bu;
const std::uint32_t firstMixMultiplier = 0x7feb352du;
const std::uint32_t secondMixMultiplier = 0x846ca68bu;
const std::uint32_t noiseFractionMask = 0x00ffffffu;
const float noiseFractionRange = 16777216.0f;

float latticeNoise(int x, int y, std::uint32_t seed)
{
    std::uint32_t value = seed ^
        (static_cast<std::uint32_t>(x) * xNoiseMultiplier) ^
        (static_cast<std::uint32_t>(y) * yNoiseMultiplier);
    value ^= value >> 16;
    value *= firstMixMultiplier;
    value ^= value >> 15;
    value *= secondMixMultiplier;
    value ^= value >> 16;
    return static_cast<float>(value & noiseFractionMask) / noiseFractionRange;
}

float smooth(float value)
{
    return value * value * (3.0f - 2.0f * value);
}

float interpolate(float from, float to, float amount)
{
    return from + (to - from) * amount;
}

bool isSolid(ObjectKind kind)
{
    return kind == ObjectKind::Wall ||
           kind == ObjectKind::PlayerTank ||
           kind == ObjectKind::BotTank;
}
}

GameMap::GameMap(int width, int height)
{
    this->width = width;
    this->height = height;
    this->floor = std::make_unique<FloorLayer>();
    this->walls = std::make_unique<WallLayer>();
    this->water = std::make_unique<WaterLayer>();

    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            if (!addInitialObject(std::make_unique<FloorTile>(GridPosition{x, y})))
            {
                throw std::runtime_error("Could not create floor tile");
            }
        }
    }
}

std::unique_ptr<GameMap> GameMap::generate(std::uint32_t seed)
{
    std::unique_ptr<GameMap> map(new GameMap(mapWidth, mapHeight));

    for (int y = 0; y < map->height; ++y)
    {
        for (int x = 0; x < map->width; ++x)
        {
            GridPosition position{x, y};

            if (isReservedRoute(position))
            {
                continue;
            }

            float noise = terrainNoise(position, seed);
            bool placed = true;

            if (noise < waterThreshold)
            {
                placed = map->addInitialObject(std::make_unique<WaterTile>(position));
            }
            else if (noise < wallThreshold)
            {
                placed = map->addInitialObject(std::make_unique<Wall>(position));
            }

            if (!placed)
            {
                throw std::runtime_error("Could not create terrain");
            }
        }
    }

    if (!map->addInitialObject(std::make_unique<PlayerTank>(playerSpawn)) ||
        !map->addInitialObject(std::make_unique<BotTank>(firstBotSpawn)) ||
        !map->addInitialObject(std::make_unique<BotTank>(secondBotSpawn)) ||
        !map->addInitialObject(std::make_unique<BotTank>(thirdBotSpawn)))
    {
        throw std::runtime_error("Could not create tanks");
    }

    return map;
}

float GameMap::terrainNoise(const GridPosition& position, std::uint32_t seed)
{
    float x = static_cast<float>(position.x) / noiseCellSize;
    float y = static_cast<float>(position.y) / noiseCellSize;
    int left = static_cast<int>(std::floor(x));
    int top = static_cast<int>(std::floor(y));
    float horizontal = smooth(x - static_cast<float>(left));
    float vertical = smooth(y - static_cast<float>(top));

    float upper = interpolate(latticeNoise(left, top, seed),
                              latticeNoise(left + 1, top, seed), horizontal);
    float lower = interpolate(latticeNoise(left, top + 1, seed),
                              latticeNoise(left + 1, top + 1, seed), horizontal);
    return interpolate(upper, lower, vertical);
}

bool GameMap::isReservedRoute(const GridPosition& position)
{
    return (position.y == routeBottomRow &&
            position.x >= routeLeftColumn && position.x <= routeRightColumn) ||
           ((position.x == routeLeftColumn || position.x == routeRightColumn) &&
            position.y >= routeTopRow && position.y <= routeBottomRow);
}

int GameMap::getWidth() const
{
    return width;
}

int GameMap::getHeight() const
{
    return height;
}

bool GameMap::inBounds(const GridPosition& position) const
{
    return position.x >= 0 && position.x < width &&
           position.y >= 0 && position.y < height;
}

bool GameMap::hasWaterAt(const GridPosition& position) const
{
    const WaterLayer& waterLayer = *water;
    return !waterLayer.getObjectsAt(position).empty();
}

bool GameMap::hasClearLineOfSight(const GridPosition& from,
                                  const GridPosition& to) const
{
    if (!inBounds(from) || !inBounds(to))
    {
        return false;
    }

    if ((from.x != to.x && from.y != to.y) ||
        (from.x == to.x && from.y == to.y))
    {
        return false;
    }

    int stepX = (to.x > from.x) - (to.x < from.x);
    int stepY = (to.y > from.y) - (to.y < from.y);
    GridPosition current{from.x + stepX, from.y + stepY};

    while (current.x != to.x || current.y != to.y)
    {
        for (const GameObject* object : getWallObjectsAt(current))
        {
            if (isSolid(object->getKind()))
            {
                return false;
            }
        }

        current.x += stepX;
        current.y += stepY;
    }

    return true;
}

std::vector<const GameObject*> GameMap::getObjectsAt(const GridPosition& position) const
{
    std::vector<const GameObject*> found;
    const FloorLayer& floorLayer = *floor;
    const WaterLayer& waterLayer = *water;

    for (const GameObject* object : floorLayer.getObjectsAt(position))
    {
        found.push_back(object);
    }
    for (const GameObject* object : waterLayer.getObjectsAt(position))
    {
        found.push_back(object);
    }

    std::vector<const GameObject*> wallObjects = getWallObjectsAt(position);

    for (const GameObject* object : wallObjects)
    {
        if (object->getKind() != ObjectKind::Projectile)
        {
            found.push_back(object);
        }
    }
    for (const GameObject* object : wallObjects)
    {
        if (object->getKind() == ObjectKind::Projectile)
        {
            found.push_back(object);
        }
    }

    return found;
}

std::vector<GameObject*> GameMap::getWallObjectsAt(const GridPosition& position)
{
    return walls->getObjectsAt(position);
}

std::vector<const GameObject*> GameMap::getWallObjectsAt(const GridPosition& position) const
{
    const WallLayer& wallLayer = *walls;
    return wallLayer.getObjectsAt(position);
}

std::vector<Projectile*> GameMap::getActiveProjectiles()
{
    std::vector<Projectile*> shots;

    for (GameObject* object : walls->getActiveObjects())
    {
        if (object->getKind() == ObjectKind::Projectile)
        {
            shots.push_back(static_cast<Projectile*>(object));
        }
    }

    std::sort(shots.begin(), shots.end(),
        [](const Projectile* first, const Projectile* second)
        {
            return first->getId() < second->getId();
        });
    return shots;
}

PlayerTank* GameMap::getPlayer()
{
    for (GameObject* object : walls->getActiveObjects())
    {
        if (object->getKind() == ObjectKind::PlayerTank)
        {
            return static_cast<PlayerTank*>(object);
        }
    }

    return nullptr;
}

const PlayerTank* GameMap::getPlayer() const
{
    const WallLayer& wallLayer = *walls;

    for (const GameObject* object : wallLayer.getActiveObjects())
    {
        if (object->getKind() == ObjectKind::PlayerTank)
        {
            return static_cast<const PlayerTank*>(object);
        }
    }

    return nullptr;
}

std::vector<BotTank*> GameMap::getBots()
{
    std::vector<BotTank*> bots;

    for (GameObject* object : walls->getActiveObjects())
    {
        if (object->getKind() == ObjectKind::BotTank)
        {
            bots.push_back(static_cast<BotTank*>(object));
        }
    }

    std::sort(bots.begin(), bots.end(),
        [](const BotTank* first, const BotTank* second)
        {
            return first->getId() < second->getId();
        });
    return bots;
}

std::vector<const BotTank*> GameMap::getBots() const
{
    std::vector<const BotTank*> bots;
    const WallLayer& wallLayer = *walls;

    for (const GameObject* object : wallLayer.getActiveObjects())
    {
        if (object->getKind() == ObjectKind::BotTank)
        {
            bots.push_back(static_cast<const BotTank*>(object));
        }
    }

    std::sort(bots.begin(), bots.end(),
        [](const BotTank* first, const BotTank* second)
        {
            return first->getId() < second->getId();
        });
    return bots;
}

bool GameMap::addInitialObject(std::unique_ptr<GameObject> object)
{
    if (!object || !object->isActive() || object->getId() != 0 ||
        !inBounds(object->getPosition()))
    {
        return false;
    }

    GridPosition position = object->getPosition();
    Layer* target = nullptr;
    ObjectKind kind = object->getKind();
    bool solidAtPosition = false;

    for (const GameObject* existing : getWallObjectsAt(position))
    {
        if (isSolid(existing->getKind()))
        {
            solidAtPosition = true;
            break;
        }
    }

    switch (kind)
    {
    case ObjectKind::FloorTile:
        target = floor.get();
        if (!floor->getObjectsAt(position).empty())
        {
            return false;
        }
        break;
    case ObjectKind::WaterTile:
        target = water.get();
        if (!water->getObjectsAt(position).empty() || solidAtPosition)
        {
            return false;
        }
        break;
    case ObjectKind::Wall:
    case ObjectKind::PlayerTank:
    case ObjectKind::BotTank:
        target = walls.get();
        if (hasWaterAt(position) || solidAtPosition ||
            (kind == ObjectKind::PlayerTank && getPlayer() != nullptr))
        {
            return false;
        }
        break;
    case ObjectKind::Projectile:
        return false;
    }

    object->setId(nextId);
    GameObject* added = target->add(std::move(object));

    if (added == nullptr)
    {
        return false;
    }

    ++nextId;
    return true;
}

Projectile* GameMap::addProjectile(std::unique_ptr<Projectile> shot)
{
    if (!shot || !shot->isActive() || shot->getId() != 0 ||
        !inBounds(shot->getPosition()))
    {
        return nullptr;
    }

    shot->setId(nextId);
    GameObject* added = walls->add(std::move(shot));

    if (added == nullptr)
    {
        return nullptr;
    }

    ++nextId;
    return static_cast<Projectile*>(added);
}

void GameMap::removeInactive()
{
    floor->removeInactive();
    water->removeInactive();
    walls->removeInactive();
}

} // namespace tanks
