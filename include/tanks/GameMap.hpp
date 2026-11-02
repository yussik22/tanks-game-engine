#pragma once

#include <cstdint>
#include <memory>
#include <vector>
#include "tanks/Layer.hpp"
#include "tanks/Projectile.hpp"
#include "tanks/Tank.hpp"

namespace tanks
{

class GameMap
{
public:
    static std::unique_ptr<GameMap> generate(std::uint32_t seed);

    int getWidth() const;
    int getHeight() const;
    bool inBounds(const GridPosition& position) const;
    bool hasWaterAt(const GridPosition& position) const;
    bool hasClearLineOfSight(const GridPosition& from,
                             const GridPosition& to) const;

    std::vector<const GameObject*> getObjectsAt(const GridPosition& position) const;
    std::vector<GameObject*> getWallObjectsAt(const GridPosition& position);
    std::vector<const GameObject*> getWallObjectsAt(const GridPosition& position) const;
    std::vector<Projectile*> getActiveProjectiles();

    PlayerTank* getPlayer();
    const PlayerTank* getPlayer() const;
    std::vector<BotTank*> getBots();
    std::vector<const BotTank*> getBots() const;

    bool addInitialObject(std::unique_ptr<GameObject> object);
    Projectile* addProjectile(std::unique_ptr<Projectile> shot);
    void removeInactive();

private:
    GameMap(int width, int height);

    static float terrainNoise(const GridPosition& position, std::uint32_t seed);
    static bool isReservedRoute(const GridPosition& position);

    int width;
    int height;
    std::uint32_t nextId = 1;
    std::unique_ptr<FloorLayer> floor;
    std::unique_ptr<WallLayer> walls;
    std::unique_ptr<WaterLayer> water;
};

} // namespace tanks
