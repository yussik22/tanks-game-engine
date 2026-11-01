#pragma once

#include <memory>
#include <vector>
#include "tanks/GameObject.hpp"

namespace tanks {

class Layer
{
public:
    virtual ~Layer() {}

    GameObject* add(std::unique_ptr<GameObject> object);
    std::vector<GameObject*> getObjectsAt(const GridPosition& position);
    std::vector<const GameObject*> getObjectsAt(const GridPosition& position) const;
    void removeInactive();
    virtual bool accepts(const GameObject& object) const = 0;

private:
    std::vector<std::unique_ptr<GameObject>> objects;
};

class FloorLayer : public Layer
{
public:
    bool accepts(const GameObject& object) const override;
};

class WallLayer : public Layer
{
public:
    bool accepts(const GameObject& object) const override;
};

class WaterLayer : public Layer
{
public:
    bool accepts(const GameObject& object) const override;
};

} // namespace tanks
