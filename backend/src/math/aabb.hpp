#pragma once

#include "math/vec2.hpp"
#include <algorithm>

namespace teleop {

struct AABB {
    Vec2 min;
    Vec2 max;

    constexpr AABB() = default;
    constexpr AABB(Vec2 min, Vec2 max) : min(min), max(max) {}

    constexpr bool overlaps(const AABB& other) const {
        return min.x <= other.max.x && max.x >= other.min.x &&
               min.y <= other.max.y && max.y >= other.min.y;
    }

    constexpr Vec2 center() const {
        return {(min.x + max.x) * 0.5f, (min.y + max.y) * 0.5f};
    }

    constexpr Vec2 halfExtents() const {
        return {(max.x - min.x) * 0.5f, (max.y - min.y) * 0.5f};
    }

    constexpr float width() const { return max.x - min.x; }
    constexpr float height() const { return max.y - min.y; }

    constexpr AABB expand(const Vec2& point) const {
        return {
            {std::min(min.x, point.x), std::min(min.y, point.y)},
            {std::max(max.x, point.x), std::max(max.y, point.y)}
        };
    }

    static AABB fromVertices(const Vec2* vertices, size_t count) {
        if (count == 0) return {};
        AABB result{vertices[0], vertices[0]};
        for (size_t i = 1; i < count; ++i) {
            result = result.expand(vertices[i]);
        }
        return result;
    }
};

} // namespace teleop
