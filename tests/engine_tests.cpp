#include <cstdint>
#include <iostream>
#include <limits>
#include <memory>
#include <set>
#include <stdexcept>
#include <string>
#include <vector>
#include "tanks/CollisionSystem.hpp"
#include "tanks/GameEngine.hpp"
#include "tanks/KeyboardInput.hpp"
#include "tanks/Layer.hpp"
#include "tanks/Tiles.hpp"
#include "tanks/Wall.hpp"

using namespace tanks;

void check(bool condition, const std::string& message)
{
    if (!condition)
    {
        throw std::runtime_error(message);
    }
}

void testLayersAndDamage()
{
    FloorLayer floor;
    GameObject* tile = floor.add(std::make_unique<FloorTile>(GridPosition{1, 1}));
    check(tile != nullptr, "Floor layer rejected a floor tile");
    check(floor.add(std::make_unique<Wall>(GridPosition{1, 1})) == nullptr,
          "Floor layer accepted a wall");
    check(floor.getObjectsAt({1, 1}).size() == 1, "Floor tile query failed");
    tile->deactivate();
    check(floor.getObjectsAt({1, 1}).empty(), "Inactive tile was visible");
    floor.removeInactive();
    check(floor.getObjectsAt({1, 1}).empty(), "Inactive tile was not removed");

    Wall wall({2, 2});
    wall.takeDamage(-1);
    check(wall.getDurability() == 2, "Negative damage changed wall durability");
    wall.takeDamage(1);
    check(wall.getDurability() == 1 && wall.isActive(), "First wall hit failed");
    wall.takeDamage(1);
    check(wall.isDestroyed() && !wall.isActive(), "Destroyed wall stayed active");
}

void testMapGeneration()
{
    std::unique_ptr<GameMap> first = GameMap::generate(17);
    std::unique_ptr<GameMap> second = GameMap::generate(17);
    check(first->getWidth() == 13 && first->getHeight() == 13,
          "Map size is not 13 by 13");
    check(first->getPlayer() != nullptr && first->getBots().size() == 3,
          "Tanks were not placed");

    std::set<std::uint32_t> ids;

    for (int y = 0; y < first->getHeight(); ++y)
    {
        for (int x = 0; x < first->getWidth(); ++x)
        {
            GridPosition position{x, y};
            std::vector<const GameObject*> firstObjects = first->getObjectsAt(position);
            std::vector<const GameObject*> secondObjects = second->getObjectsAt(position);
            check(firstObjects.size() == secondObjects.size(),
                  "Same seed created different map contents");
            check(!firstObjects.empty() &&
                  firstObjects.front()->getKind() == ObjectKind::FloorTile,
                  "Map cell has no floor");

            bool water = false;
            bool solid = false;
            bool wall = false;

            for (std::size_t index = 0; index < firstObjects.size(); ++index)
            {
                const GameObject* object = firstObjects[index];
                check(object->getKind() == secondObjects[index]->getKind(),
                      "Same seed created different terrain");
                check(object->getId() != 0 && ids.insert(object->getId()).second,
                      "Map object ID is missing or duplicated");
                water |= object->getKind() == ObjectKind::WaterTile;
                wall |= object->getKind() == ObjectKind::Wall;
                solid |= object->getKind() == ObjectKind::Wall ||
                         object->getKind() == ObjectKind::PlayerTank ||
                         object->getKind() == ObjectKind::BotTank;
            }

            check(!(water && solid), "Water overlaps a solid object");

            bool reserved = (y == 11 && x >= 2 && x <= 10) ||
                            ((x == 2 || x == 10) && y >= 2 && y <= 11);
            check(!reserved || (!water && !wall),
                  "Reserved route contains terrain");
        }
    }

    for (std::uint32_t seed = 0; seed < 32; ++seed)
    {
        std::unique_ptr<GameMap> map = GameMap::generate(seed);
        check(map->getPlayer() != nullptr && map->getBots().size() == 3,
              "A generated map lost a tank");

        for (int x = 2; x <= 10; ++x)
        {
            check(!map->hasWaterAt({x, 11}), "Reserved route contains water");
        }
    }
}

void testMapPlacementAndSight()
{
    std::unique_ptr<GameMap> map = GameMap::generate(17);
    PlayerTank* player = map->getPlayer();
    check(player != nullptr, "Player missing from placement test");
    check(map->hasClearLineOfSight({6, 11}, {10, 11}),
          "Open route does not allow line of sight");
    check(!map->hasClearLineOfSight({6, 11}, {7, 10}),
          "Diagonal line of sight was accepted");
    check(!map->hasClearLineOfSight({6, 11}, {6, 11}),
          "Same-cell line of sight was accepted");
    check(!map->hasClearLineOfSight({6, 11}, {13, 11}),
          "Out-of-bounds line of sight was accepted");

    check(!map->addInitialObject(std::make_unique<FloorTile>(GridPosition{7, 11})),
          "Duplicate floor was accepted");
    check(!map->addInitialObject(std::make_unique<PlayerTank>(GridPosition{9, 11})),
          "Second player was accepted");
    check(!map->addInitialObject(std::make_unique<Wall>(GridPosition{-1, 11})),
          "Out-of-bounds wall was accepted");
    check(!map->addInitialObject(std::make_unique<Projectile>(
          GridPosition{7, 11}, Direction::Right, 1, player->getId())),
          "Projectile was accepted as initial terrain");

    check(map->addInitialObject(std::make_unique<Wall>(GridPosition{7, 11})),
          "Could not place wall on clear floor");
    check(!map->hasClearLineOfSight({6, 11}, {10, 11}),
          "Wall did not block line of sight");
    check(!map->addInitialObject(std::make_unique<BotTank>(GridPosition{7, 11})),
          "Tank was placed on wall");
    check(!map->addInitialObject(std::make_unique<WaterTile>(GridPosition{7, 11})),
          "Water was placed on wall");

    Wall* wall = nullptr;
    for (GameObject* object : map->getWallObjectsAt({7, 11}))
    {
        if (object->getKind() == ObjectKind::Wall)
        {
            wall = static_cast<Wall*>(object);
        }
    }
    check(wall != nullptr, "Placed wall was not found");
    wall->takeDamage(2);
    check(map->hasClearLineOfSight({6, 11}, {10, 11}),
          "Destroyed wall still blocked sight");
    map->removeInactive();

    check(map->addInitialObject(std::make_unique<WaterTile>(GridPosition{8, 11})),
          "Could not place water on clear floor");
    check(map->hasClearLineOfSight({6, 11}, {10, 11}),
          "Water blocked line of sight");
    check(!map->addInitialObject(std::make_unique<Wall>(GridPosition{8, 11})),
          "Wall was placed on water");
    check(!map->addInitialObject(std::make_unique<WaterTile>(GridPosition{8, 11})),
          "Duplicate water was accepted");
    check(map->addInitialObject(std::make_unique<BotTank>(GridPosition{7, 11})),
          "Could not place a tank between sight endpoints");
    check(!map->hasClearLineOfSight({6, 11}, {10, 11}),
          "Intermediate tank did not block line of sight");
    check(map->addProjectile(std::make_unique<Projectile>(
          GridPosition{-1, 11}, Direction::Right, 1, player->getId())) == nullptr,
          "Out-of-bounds projectile was accepted");
}

void testMovementRules()
{
    std::unique_ptr<GameMap> map = GameMap::generate(23);
    CollisionSystem collisions;
    PlayerTank* player = map->getPlayer();
    check(player != nullptr, "Player missing from movement test");
    check(!player->tryMove({6, 11}, *map, collisions),
          "Tank moved without changing cell");
    check(!player->tryMove({8, 11}, *map, collisions),
          "Tank teleported two cells");

    Projectile* shot = map->addProjectile(std::make_unique<Projectile>(
        GridPosition{7, 11}, Direction::Right, 1, player->getId()));
    check(shot != nullptr && player->tryMove({7, 11}, *map, collisions),
          "Projectile blocked tank movement");
    check(map->addInitialObject(std::make_unique<Wall>(GridPosition{8, 11})),
          "Could not place movement-test wall");
    check(!player->tryMove({8, 11}, *map, collisions),
          "Tank moved into a wall");

    for (GameObject* object : map->getWallObjectsAt({8, 11}))
    {
        if (object->getKind() == ObjectKind::Wall)
        {
            static_cast<Wall*>(object)->takeDamage(2);
        }
    }
    map->removeInactive();
    check(map->addInitialObject(std::make_unique<WaterTile>(GridPosition{8, 11})),
          "Could not place movement-test water");
    check(!player->tryMove({8, 11}, *map, collisions),
          "Tank moved into water");

    PlayerTank nearBot({2, 3});
    check(!collisions.canTankEnter(nearBot, {2, 2}, *map),
          "Tank moved into another tank");
    PlayerTank edge({0, 0});
    check(!collisions.canTankEnter(edge, {-1, 0}, *map),
          "Tank moved outside the map");
}

void testMovementAndShots()
{
    std::unique_ptr<GameMap> map = GameMap::generate(23);
    CollisionSystem collisions;
    PlayerTank* player = map->getPlayer();
    check(player != nullptr, "Player missing");
    check(!collisions.canTankEnter(*player, {8, 11}, *map),
          "Tank can move more than one cell");
    check(player->tryMove({7, 11}, *map, collisions),
          "Tank cannot move along the clear route");
    check(player->getPosition().x == 7 && player->getPosition().y == 11,
          "Tank position did not change");

    std::unique_ptr<Projectile> firstShot = player->fire();
    check(firstShot != nullptr && firstShot->getOwnerId() == player->getId(),
          "Player did not fire with its own ID");
    check(player->fire() == nullptr, "Cooldown allowed immediate second shot");
    for (int tick = 0; tick < 4; ++tick)
    {
        player->updateCooldown(0.1f);
        check(!player->canFire(), "Cooldown ended before five ticks");
    }
    player->updateCooldown(0.1f);
    check(player->canFire(), "Cooldown did not end after five ticks");
    check(player->fire() != nullptr, "Cooldown did not expire");

    Wall* wall = nullptr;

    for (int y = 0; y < map->getHeight() && wall == nullptr; ++y)
    {
        for (int x = 0; x < map->getWidth() && wall == nullptr; ++x)
        {
            for (GameObject* object : map->getWallObjectsAt({x, y}))
            {
                if (object->getKind() == ObjectKind::Wall)
                {
                    wall = static_cast<Wall*>(object);
                    break;
                }
            }
        }
    }

    check(wall != nullptr, "Generated map contains no wall");
    GridPosition wallPosition = wall->getPosition();

    for (int hit = 0; hit < 2; ++hit)
    {
        Projectile* shot = map->addProjectile(std::make_unique<Projectile>(
            wallPosition, Direction::Up, 1, player->getId()));
        check(shot != nullptr, "Could not add test projectile");
        collisions.resolveProjectileStep(*shot, *map);
        check(!shot->isActive(), "Projectile survived a wall hit");
    }

    check(wall->isDestroyed() && !wall->isActive(), "Two hits did not destroy wall");
    map->removeInactive();

    for (GameObject* object : map->getWallObjectsAt(wallPosition))
    {
        check(object->getKind() != ObjectKind::Wall,
              "Destroyed wall remained on map");
    }

    Projectile* ownShot = map->addProjectile(std::make_unique<Projectile>(
        player->getPosition(), Direction::Up, 1, player->getId()));
    check(ownShot != nullptr, "Could not add own projectile");
    int health = player->getHealth();
    collisions.resolveProjectileStep(*ownShot, *map);
    check(ownShot->isActive() && player->getHealth() == health,
          "Projectile damaged its owner");

    GridPosition waterPosition{-1, -1};

    for (int y = 0; y < map->getHeight() && waterPosition.x < 0; ++y)
    {
        for (int x = 0; x < map->getWidth(); ++x)
        {
            if (map->hasWaterAt({x, y}))
            {
                waterPosition = {x, y};
                break;
            }
        }
    }

    check(waterPosition.x >= 0, "Generated map contains no water");
    Projectile* waterShot = map->addProjectile(std::make_unique<Projectile>(
        waterPosition, Direction::Up, 1, player->getId()));
    check(waterShot != nullptr, "Could not place shot over water");
    collisions.resolveProjectileStep(*waterShot, *map);
    check(waterShot->isActive(), "Water stopped a projectile");
}

void testBotFire()
{
    std::unique_ptr<GameMap> map = GameMap::generate(5);
    CollisionSystem collisions;
    PlayerTank* player = map->getPlayer();
    std::vector<BotTank*> bots = map->getBots();
    check(player != nullptr && bots.size() == 3, "Test tanks missing");

    for (int x = 5; x >= 2; --x)
    {
        check(player->tryMove({x, 11}, *map, collisions),
              "Player could not follow reserved route");
    }
    for (int y = 10; y >= 3; --y)
    {
        check(player->tryMove({2, y}, *map, collisions),
              "Player could not approach bot");
    }

    Command botCommand = bots[0]->chooseCommand(*map, *player, collisions);
    check(botCommand.kind == CommandKind::AimAndFire &&
          botCommand.direction == Direction::Down,
          "Bot did not aim at visible player");
    bots[0]->turn(botCommand.direction);
    Projectile* botShot = map->addProjectile(bots[0]->fire());
    check(botShot != nullptr, "Bot did not create a shot");
    botShot->step();
    collisions.resolveProjectileStep(*botShot, *map);
    check(player->getHealth() == 2 && !botShot->isActive(),
          "Bot shot did not damage player");

    player->turn(Direction::Up);
    Projectile* playerShot = map->addProjectile(player->fire());
    check(playerShot != nullptr, "Player did not fire at adjacent bot");
    playerShot->step();
    collisions.resolveProjectileStep(*playerShot, *map);
    check(bots[0]->getHealth() == 1 && !playerShot->isActive(),
          "Point-blank player shot did not damage bot");

    Projectile* friendlyShot = map->addProjectile(std::make_unique<Projectile>(
        bots[1]->getPosition(), Direction::Up, 1, bots[0]->getId()));
    check(friendlyShot != nullptr, "Could not add friendly-fire shot");
    collisions.resolveProjectileStep(*friendlyShot, *map);
    check(bots[1]->getHealth() == 1 && !friendlyShot->isActive(),
          "Bot shot did not damage another bot");
}

void testProjectileEdgeCases()
{
    std::unique_ptr<GameMap> map = GameMap::generate(5);
    CollisionSystem collisions;
    PlayerTank* player = map->getPlayer();
    check(player != nullptr, "Player missing from projectile test");
    check(map->addInitialObject(std::make_unique<Wall>(GridPosition{7, 11}, 1)),
          "Could not place one-hit wall");

    Projectile* first = map->addProjectile(std::make_unique<Projectile>(
        GridPosition{7, 11}, Direction::Right, 1, player->getId()));
    Projectile* second = map->addProjectile(std::make_unique<Projectile>(
        GridPosition{7, 11}, Direction::Right, 1, player->getId()));
    check(first != nullptr && second != nullptr, "Could not add two shots");
    collisions.resolveProjectileStep(*first, *map);
    check(!first->isActive(), "First shot did not hit wall");
    collisions.resolveProjectileStep(*second, *map);
    check(second->isActive(), "Second shot hit an inactive wall");

    second->step();
    collisions.resolveProjectileStep(*second, *map);
    check(second->isActive() && second->getPosition().x == 8,
          "Second shot did not continue through destroyed wall");

    Projectile* overlapping = map->addProjectile(std::make_unique<Projectile>(
        GridPosition{8, 11}, Direction::Left, 1, player->getId()));
    check(overlapping != nullptr, "Could not overlap two projectiles");
    collisions.resolveProjectileStep(*overlapping, *map);
    check(overlapping->isActive(), "Projectiles collided with each other");

    Projectile* leaving = map->addProjectile(std::make_unique<Projectile>(
        GridPosition{12, 11}, Direction::Right, 1, player->getId()));
    check(leaving != nullptr, "Could not place boundary projectile");
    leaving->step();
    collisions.resolveProjectileStep(*leaving, *map);
    check(!leaving->isActive(), "Out-of-bounds projectile stayed active");
}

void testBotPatrolAndTrappedState()
{
    std::unique_ptr<GameMap> map = GameMap::generate(5);
    CollisionSystem collisions;
    PlayerTank* player = map->getPlayer();
    std::vector<BotTank*> bots = map->getBots();
    check(player != nullptr && !bots.empty(), "Bot patrol test has no tanks");
    BotTank* bot = bots[0];
    Command patrol = bot->chooseCommand(*map, *player, collisions);
    check(patrol.kind == CommandKind::Move || patrol.kind == CommandKind::Turn,
          "Bot fired without a clear line of sight");

    for (Direction direction : {Direction::Up, Direction::Right,
                                Direction::Down, Direction::Left})
    {
        GridPosition neighbor = adjacentPosition(bot->getPosition(), direction);

        if (!map->inBounds(neighbor) || map->hasWaterAt(neighbor) ||
            !map->getWallObjectsAt(neighbor).empty())
        {
            continue;
        }

        check(map->addInitialObject(std::make_unique<Wall>(neighbor)),
              "Could not block bot neighbor");
    }

    Command trapped = bot->chooseCommand(*map, *player, collisions);
    check(trapped.kind == CommandKind::Turn,
          "Trapped bot tried to move or fire");
}

void testBotSightBlockedByWall()
{
    std::unique_ptr<GameMap> map = GameMap::generate(5);
    CollisionSystem collisions;
    PlayerTank* player = map->getPlayer();
    std::vector<BotTank*> bots = map->getBots();
    check(player != nullptr && !bots.empty(), "Sight test has no tanks");

    for (int x = 5; x >= 2; --x)
    {
        check(player->tryMove({x, 11}, *map, collisions),
              "Player could not reach bot column");
    }
    for (int y = 10; y >= 5; --y)
    {
        check(player->tryMove({2, y}, *map, collisions),
              "Player could not reach sight test cell");
    }

    check(bots[0]->chooseCommand(*map, *player, collisions).kind ==
          CommandKind::AimAndFire,
          "Bot did not see player in clear column");
    check(map->addInitialObject(std::make_unique<Wall>(GridPosition{2, 4})),
          "Could not place sight-blocking wall");
    check(bots[0]->chooseCommand(*map, *player, collisions).kind !=
          CommandKind::AimAndFire,
          "Bot fired through a wall");

    for (GameObject* object : map->getWallObjectsAt({2, 4}))
    {
        if (object->getKind() == ObjectKind::Wall)
        {
            static_cast<Wall*>(object)->takeDamage(2);
        }
    }

    check(bots[0]->chooseCommand(*map, *player, collisions).kind ==
          CommandKind::AimAndFire,
          "Bot could not see past destroyed wall");
}

void testPointBlankWallShot()
{
    GameEngine engine(42);
    engine.handle({CommandKind::Start});
    const GameMap& map = engine.getMap();
    const PlayerTank* player = map.getPlayer();
    const Wall* wall = nullptr;

    for (const GameObject* object : map.getWallObjectsAt({6, 10}))
    {
        if (object->getKind() == ObjectKind::Wall)
        {
            wall = static_cast<const Wall*>(object);
        }
    }

    check(player != nullptr && wall != nullptr && wall->getDurability() == 2,
          "Expected adjacent starting wall is missing");
    check(player->canFire(), "Starting player cannot fire");
    engine.handle({CommandKind::Fire});
    engine.update(worldTickSeconds);
    check(wall->getDurability() == 1 && wall->isActive(),
          "Point-blank shot did not damage wall");
    check(!player->canFire(), "Player did not enter reload after firing");
}

void testProjectileSpeed()
{
    GameEngine engine(42);
    engine.handle({CommandKind::Start});
    engine.handle({CommandKind::Move, Direction::Right});
    engine.update(worldTickSeconds);

    const GameMap& map = engine.getMap();
    const PlayerTank* player = map.getPlayer();
    check(player != nullptr && player->getPosition().x == 7 &&
          player->getPosition().y == 11,
          "Tank did not move exactly one cell in a tick");

    engine.handle({CommandKind::Fire});
    engine.update(worldTickSeconds);
    bool foundAtSecondCell = false;

    for (const GameObject* object : map.getWallObjectsAt({9, 11}))
    {
        if (object->getKind() == ObjectKind::Projectile &&
            static_cast<const Projectile*>(object)->getOwnerId() == player->getId())
        {
            foundAtSecondCell = true;
        }
    }

    check(foundAtSecondCell, "Projectile did not move two cells in one tick");
}

void testReadyMapAndProjectileEntry()
{
    GameEngine engine(42);
    const GameMap* readyMap = &engine.getMap();
    engine.handle({CommandKind::Start});
    check(&engine.getMap() == readyMap,
          "Starting changed the map shown on the Ready screen");

    GameMap& map = const_cast<GameMap&>(engine.getMap());
    PlayerTank* player = map.getPlayer();
    check(player != nullptr && !map.getBots().empty(),
          "Projectile entry test has no tanks");
    Projectile* shot = map.addProjectile(std::make_unique<Projectile>(
        GridPosition{7, 11}, Direction::Right, 1, map.getBots()[0]->getId()));
    check(shot != nullptr, "Could not place incoming projectile");

    engine.handle({CommandKind::Move, Direction::Right});
    check(player->getPosition().x == 7 && player->getHealth() == 2,
          "Walking into a projectile did not damage the player");
}

void testEngineTimingAndQueue()
{
    GameEngine timing(42);
    timing.handle({CommandKind::Start});
    timing.handle({CommandKind::Move, Direction::Right});
    check(timing.getMap().getPlayer()->getPosition().x == 7,
          "First movement command waited for a world tick");
    timing.handle({CommandKind::Move, Direction::Right});
    timing.update(worldTickSeconds / 2.0f);
    check(timing.getMap().getPlayer()->getPosition().x == 7,
          "Half tick applied pending movement early");
    timing.update(worldTickSeconds / 2.0f);
    check(timing.getMap().getPlayer()->getPosition().x == 8,
          "Two half ticks did not apply pending movement");

    timing.handle({CommandKind::Move, Direction::Right});
    timing.update(-1.0f);
    timing.update(std::numeric_limits<float>::quiet_NaN());
    check(timing.getMap().getPlayer()->getPosition().x == 8,
          "Invalid elapsed time advanced the game");
    timing.update(worldTickSeconds);
    check(timing.getMap().getPlayer()->getPosition().x == 9,
          "Valid elapsed time did not advance the game");

    GameEngine queued(42);
    queued.handle({CommandKind::Start});
    queued.handle({CommandKind::Move, Direction::Right});
    queued.handle({CommandKind::Move, Direction::Left});
    queued.handle({CommandKind::Move, Direction::Right});
    queued.handle({CommandKind::Move, Direction::Left});
    queued.update(worldTickSeconds);
    check(queued.getMap().getPlayer()->getPosition().x == 6,
          "Latest movement command did not replace older input");

    GameEngine released(42);
    released.handle({CommandKind::Start});
    released.handle({CommandKind::Move, Direction::Right});
    released.handle({CommandKind::Move, Direction::Right});
    released.releaseMove();
    released.update(worldTickSeconds);
    check(released.getMap().getPlayer()->getPosition().x == 7,
          "Released movement key caused another step");

    GameEngine turning(42);
    turning.handle({CommandKind::Start});
    turning.handle({CommandKind::Turn, Direction::Right});
    check(turning.getMap().getPlayer()->getFacing() == Direction::Right,
          "Turret turn waited for a world tick");

    GameEngine batched(42);
    batched.handle({CommandKind::Start});
    batched.handle({CommandKind::Fire});
    check(!batched.getMap().getPlayer()->canFire(),
          "Fire command waited for a world tick");
    batched.update(3.0f * worldTickSeconds);
    check(batched.getMap().getPlayer()->canFire(),
          "One update did not run three world ticks");

    GameEngine longFrame(42);
    GameEngine reference(42);
    longFrame.handle({CommandKind::Start});
    reference.handle({CommandKind::Start});
    longFrame.update(3.0f);
    reference.update(3.0f * worldTickSeconds);
    const PlayerTank* longFramePlayer = longFrame.getMap().getPlayer();
    const PlayerTank* referencePlayer = reference.getMap().getPlayer();
    check(longFramePlayer != nullptr && referencePlayer != nullptr &&
          longFramePlayer->getHealth() == referencePlayer->getHealth(),
          "Long frame ran more than three world ticks");
    std::vector<const BotTank*> longFrameBots = longFrame.getMap().getBots();
    std::vector<const BotTank*> referenceBots = reference.getMap().getBots();
    check(longFrameBots.size() == referenceBots.size(),
          "Long frame changed the number of bots");

    for (std::size_t index = 0; index < longFrameBots.size(); ++index)
    {
        GridPosition actual = longFrameBots[index]->getPosition();
        GridPosition expected = referenceBots[index]->getPosition();
        check(actual.x == expected.x && actual.y == expected.y &&
              longFrameBots[index]->getFacing() == referenceBots[index]->getFacing(),
              "Long frame advanced bot actions past three world ticks");
    }

    GameEngine ignored(42);
    ignored.handle({CommandKind::Start});
    ignored.handle({CommandKind::AimAndFire, Direction::Right});
    ignored.update(worldTickSeconds);
    check(ignored.getMap().getPlayer()->getFacing() == Direction::Up,
          "Player was allowed to send bot-only AimAndFire command");
}

void testWinAndSimultaneousLoss()
{
    CollisionSystem collisions;
    GameEngine winEngine(42);
    winEngine.handle({CommandKind::Start});

    // The engine exposes a read-only map. These test scenarios apply damage
    // through the same public collision method used for normal shots.
    GameMap& winMap = const_cast<GameMap&>(winEngine.getMap());
    PlayerTank* winPlayer = winMap.getPlayer();
    check(winPlayer != nullptr, "Win test has no player");

    for (BotTank* bot : winMap.getBots())
    {
        Projectile* shot = winMap.addProjectile(std::make_unique<Projectile>(
            bot->getPosition(), Direction::Up, bot->getHealth(), winPlayer->getId()));
        check(shot != nullptr, "Could not prepare bot hit");
        collisions.resolveProjectileStep(*shot, winMap);
    }

    winEngine.handle({CommandKind::Turn, Direction::Right});
    winEngine.handle({CommandKind::Move, Direction::Left});
    winEngine.update(worldTickSeconds);
    check(winEngine.getState() == GameState::Won,
          "Defeating all bots did not win the battle");
    winEngine.handle({CommandKind::Move, Direction::Right});
    winEngine.update(worldTickSeconds);
    check(winEngine.getState() == GameState::Won,
          "Gameplay command changed Won state");
    winEngine.handle({CommandKind::Restart});
    check(winEngine.getState() == GameState::Running &&
          winEngine.getMap().getBots().size() == 3,
          "Restart after win did not create a new battle");
    winEngine.update(worldTickSeconds);
    check(winEngine.getMap().getPlayer()->getPosition().x == 6,
          "Restart kept an old queued command");

    GameEngine lossEngine(42);
    lossEngine.handle({CommandKind::Start});
    GameMap& lossMap = const_cast<GameMap&>(lossEngine.getMap());
    PlayerTank* lossPlayer = lossMap.getPlayer();
    std::vector<BotTank*> lossBots = lossMap.getBots();
    check(lossPlayer != nullptr && !lossBots.empty(),
          "Loss test has no tanks");
    for (std::size_t index = 0; index < 2; ++index)
    {
        BotTank* bot = lossBots[index];
        Projectile* shot = lossMap.addProjectile(std::make_unique<Projectile>(
            bot->getPosition(), Direction::Up, bot->getHealth(), lossPlayer->getId()));
        check(shot != nullptr, "Could not prepare earlier bot hit");
        collisions.resolveProjectileStep(*shot, lossMap);
    }

    BotTank* finalBot = lossBots[2];
    Projectile* playerShot = lossMap.addProjectile(std::make_unique<Projectile>(
        GridPosition{10, 9}, Direction::Up, finalBot->getHealth(),
        lossPlayer->getId()));
    Projectile* botShot = lossMap.addProjectile(std::make_unique<Projectile>(
        GridPosition{6, 12}, Direction::Up, lossPlayer->getHealth(),
        finalBot->getId()));
    check(playerShot != nullptr && botShot != nullptr,
          "Could not prepare simultaneous shots");
    lossEngine.update(worldTickSeconds);
    check(lossEngine.getState() == GameState::Lost,
          "Player death did not take priority over final bot death");
    check(lossMap.getPlayer() == nullptr && lossMap.getBots().empty(),
          "Both final shots did not hit in the same tick");
}

void testBattleStress()
{
    const Command actions[] = {
        {CommandKind::Move, Direction::Up},
        {CommandKind::Move, Direction::Right},
        {CommandKind::Move, Direction::Down},
        {CommandKind::Move, Direction::Left},
        {CommandKind::Fire},
        {CommandKind::Turn, Direction::Right}
    };

    for (std::uint32_t seed = 0; seed < 16; ++seed)
    {
        GameEngine engine(seed);
        engine.handle({CommandKind::Start});

        for (int tick = 0; tick < 80 && engine.getState() == GameState::Running; ++tick)
        {
            engine.handle(actions[(seed + static_cast<std::uint32_t>(tick)) % 6]);
            engine.update(worldTickSeconds);
            const GameMap& map = engine.getMap();
            std::set<std::uint32_t> ids;

            for (int y = 0; y < map.getHeight(); ++y)
            {
                for (int x = 0; x < map.getWidth(); ++x)
                {
                    int solidCount = 0;
                    bool water = false;

                    for (const GameObject* object : map.getObjectsAt({x, y}))
                    {
                        check(object->isActive() && object->getId() != 0 &&
                              ids.insert(object->getId()).second,
                              "Battle contains inactive or duplicate-ID object");
                        ObjectKind kind = object->getKind();
                        water |= kind == ObjectKind::WaterTile;
                        solidCount += kind == ObjectKind::Wall ||
                                      kind == ObjectKind::PlayerTank ||
                                      kind == ObjectKind::BotTank;
                    }

                    check(solidCount <= 1 && !(water && solidCount > 0),
                          "Battle map has overlapping solid objects");
                }
            }
        }
    }
}

void testInputAndEngineState()
{
    KeyboardInput input;
    check(input.parse("w")->kind == CommandKind::Move &&
          input.parse("w")->direction == Direction::Up,
          "W key does not move up");
    check(input.parse("f")->kind == CommandKind::Fire,
          "F key does not fire");
    check(!input.parse("unknown"), "Unknown input was accepted");

    GameEngine engine(42);
    check(engine.getState() == GameState::Ready, "Engine did not start Ready");
    engine.handle({CommandKind::Move, Direction::Right});
    engine.update(worldTickSeconds);
    check(engine.getState() == GameState::Ready,
          "Move command started battle from Ready");
    engine.handle({CommandKind::Start});
    check(engine.getState() == GameState::Running,
          "Start command did not start battle");
    engine.handle({CommandKind::Restart});
    check(engine.getState() == GameState::Running,
          "Restart changed a running battle");
    engine.update(worldTickSeconds);
    check(engine.getState() == GameState::Running,
          "Battle ended after one tick");

    for (int tick = 0; tick < 100 && engine.getState() == GameState::Running; ++tick)
    {
        engine.update(worldTickSeconds);
    }

    check(engine.getState() == GameState::Lost,
          "Bot attacks did not reach the loss state");
    engine.update(worldTickSeconds);
    check(engine.getState() == GameState::Lost,
          "Terminal state advanced another battle tick");
    engine.handle({CommandKind::Restart});
    check(engine.getState() == GameState::Running &&
          engine.getMap().getPlayer() != nullptr &&
          engine.getMap().getPlayer()->getHealth() == 3,
          "Restart did not create a fresh battle");
}

int main()
{
    try
    {
        testLayersAndDamage();
        testMapGeneration();
        testMapPlacementAndSight();
        testMovementRules();
        testMovementAndShots();
        testBotFire();
        testProjectileEdgeCases();
        testBotPatrolAndTrappedState();
        testBotSightBlockedByWall();
        testPointBlankWallShot();
        testProjectileSpeed();
        testReadyMapAndProjectileEntry();
        testEngineTimingAndQueue();
        testWinAndSimultaneousLoss();
        testInputAndEngineState();
        testBattleStress();
        std::cout << "All engine tests passed\n";
        return 0;
    }
    catch (const std::exception& error)
    {
        std::cerr << "Engine test failed: " << error.what() << '\n';
        return 1;
    }
}
