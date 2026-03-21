#pragma once

#include "math/vec2.hpp"
#include <vector>
#include <span>

namespace teleop {

struct SATResult {
    bool colliding = false;
    Vec2 normal = {0, 0};         // Minimum Translation Vector direction
    float penetration = 0.0f;     // MTV magnitude
};

class SATDetector {
public:
    SATResult test(
        std::span<const Vec2> vertsA,
        std::span<const Vec2> vertsB
    ) const;

private:
    std::pair<float, float> project(std::span<const Vec2> verts, Vec2 axis) const;
    static Vec2 centroid(std::span<const Vec2> verts);
};

} // namespace teleop
