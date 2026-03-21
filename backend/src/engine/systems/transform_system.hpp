#pragma once

#include <entt/entt.hpp>

namespace teleop {

class TransformSystem {
public:
    void tick(entt::registry& registry);
};

} // namespace teleop
