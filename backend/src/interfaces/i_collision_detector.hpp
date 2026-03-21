#pragma once

#include "math/vec2.hpp"
#include "math/aabb.hpp"
#include <vector>
#include <span>
#include <cstdint>

namespace teleop {

struct CollisionPair {
    uint32_t entityA;
    uint32_t entityB;
    Vec2 normal;
    float penetrationDepth = 0.0f;
};

class ICollisionDetector {
public:
    virtual ~ICollisionDetector() = default;

    virtual std::vector<CollisionPair> detect(
        const std::vector<std::pair<uint32_t, std::span<const Vec2>>>& polygons,
        const std::vector<std::pair<uint32_t, AABB>>& bounds
    ) = 0;
};

} // namespace teleop
