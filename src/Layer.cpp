#include "tanks/Layer.hpp"

#include <algorithm>
#include <utility>

namespace tanks
{

GameObject* Layer::add(std::unique_ptr<GameObject> object)
{
    if (!object || !accepts(*object))
    {
        return nullptr;
    }

    GameObject* addedObject = object.get();
    objects.push_back(std::move(object));
    return addedObject;
}

std::vector<GameObject*> Layer::getActiveObjects()
{
    std::vector<GameObject*> active;

    for (const auto& object : objects)
    {
        if (object->isActive())
        {
            active.push_back(object.get());
        }
    }

    return active;
}

std::vector<const GameObject*> Layer::getActiveObjects() const
{
    std::vector<const GameObject*> active;

    for (const auto& object : objects)
    {
        if (object->isActive())
        {
            active.push_back(object.get());
        }
    }

    return active;
}

std::vector<GameObject*> Layer::getObjectsAt(const GridPosition& position)
{
    std::vector<GameObject*> found;

    for (const auto& object : objects)
    {
        GridPosition objectPosition = object->getPosition();

        if (object->isActive() && objectPosition.x == position.x &&
            objectPosition.y == position.y)
        {
            found.push_back(object.get());
        }
    }

    return found;
}

std::vector<const GameObject*> Layer::getObjectsAt(const GridPosition& position) const
{
    std::vector<const GameObject*> found;

    for (const auto& object : objects)
    {
        GridPosition objectPosition = object->getPosition();

        if (object->isActive() && objectPosition.x == position.x &&
            objectPosition.y == position.y)
        {
            found.push_back(object.get());
        }
    }

    return found;
}

void Layer::removeInactive()
{
    objects.erase(std::remove_if(objects.begin(), objects.end(),
        [](const std::unique_ptr<GameObject>& object)
        {
            return !object->isActive();
        }), objects.end());
}

bool FloorLayer::accepts(const GameObject& object) const
{
    return object.getKind() == ObjectKind::FloorTile;
}

bool WallLayer::accepts(const GameObject& object) const
{
    ObjectKind kind = object.getKind();

    return kind == ObjectKind::Wall ||
           kind == ObjectKind::PlayerTank ||
           kind == ObjectKind::BotTank ||
           kind == ObjectKind::Projectile;
}

bool WaterLayer::accepts(const GameObject& object) const
{
    return object.getKind() == ObjectKind::WaterTile;
}

} // namespace tanks
