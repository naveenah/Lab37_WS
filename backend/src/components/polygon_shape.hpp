#pragma once

#include "math/vec2.hpp"
#include "math/aabb.hpp"
#include <vector>

namespace teleop {

struct PolygonShape {
    std::vector<Vec2> localVertices;   // Vertices in local space (CCW winding)
    std::vector<Vec2> worldVertices;   // Cached world-space vertices (recomputed each tick)
    AABB aabb;                         // Axis-aligned bounding box (recomputed each tick)
};

} // namespace teleop
