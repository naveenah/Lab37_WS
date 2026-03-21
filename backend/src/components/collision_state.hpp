#pragma once

#include "math/vec2.hpp"
#include <entt/entt.hpp>

namespace teleop {

struct CollisionState {
    bool isColliding = false;                      // Currently in collision?
    entt::entity collidedWith = entt::null;         // Last collision partner
    bool wasCollidingLastTick = false;              // Was colliding in previous tick?
    Vec2 mtv = {0.0f, 0.0f};                       // Minimum Translation Vector for pushout
};

} // namespace teleop
