#pragma once

#include <entt/entt.hpp>

namespace teleop {

class MovementSystem {
public:
    void tick(entt::registry& registry, float dt);
};

} // namespace teleop
