#include "physics/sat_detector.hpp"
#include <limits>
#include <cmath>

namespace teleop {

SATResult SATDetector::test(
    std::span<const Vec2> vertsA,
    std::span<const Vec2> vertsB
) const {
    if (vertsA.size() < 3 || vertsB.size() < 3) {
        return {false, {0, 0}, 0.0f};
    }

    float minOverlap = std::numeric_limits<float>::max();
    Vec2 mtvAxis = {0, 0};

    // Test axes from polygon A's edges
    for (size_t i = 0; i < vertsA.size(); ++i) {
        Vec2 edge = vertsA[(i + 1) % vertsA.size()] - vertsA[i];
        Vec2 axis = edge.perpendicular().normalized();

        // Skip degenerate axes
        if (axis.lengthSquared() < 1e-10f) continue;

        auto [minA, maxA] = project(vertsA, axis);
        auto [minB, maxB] = project(vertsB, axis);

        float overlap = std::min(maxA, maxB) - std::max(minA, minB);
        if (overlap <= 0.0f) {
            return {false, {0, 0}, 0.0f}; // Separating axis found
        }
        if (overlap < minOverlap) {
            minOverlap = overlap;
            mtvAxis = axis;
        }
    }

    // Test axes from polygon B's edges
    for (size_t i = 0; i < vertsB.size(); ++i) {
        Vec2 edge = vertsB[(i + 1) % vertsB.size()] - vertsB[i];
        Vec2 axis = edge.perpendicular().normalized();

        if (axis.lengthSquared() < 1e-10f) continue;

        auto [minA, maxA] = project(vertsA, axis);
        auto [minB, maxB] = project(vertsB, axis);

        float overlap = std::min(maxA, maxB) - std::max(minA, minB);
        if (overlap <= 0.0f) {
            return {false, {0, 0}, 0.0f};
        }
        if (overlap < minOverlap) {
            minOverlap = overlap;
            mtvAxis = axis;
        }
    }

    // Ensure MTV pushes A away from B
    Vec2 centerDiff = centroid(vertsB) - centroid(vertsA);
    if (mtvAxis.dot(centerDiff) > 0.0f) {
        mtvAxis = -mtvAxis;
    }

    return {true, mtvAxis, minOverlap};
}

std::pair<float, float> SATDetector::project(
    std::span<const Vec2> verts, Vec2 axis
) const {
    float min = std::numeric_limits<float>::max();
    float max = std::numeric_limits<float>::lowest();
    for (const auto& v : verts) {
        float p = v.dot(axis);
        if (p < min) min = p;
        if (p > max) max = p;
    }
    return {min, max};
}

Vec2 SATDetector::centroid(std::span<const Vec2> verts) {
    Vec2 sum{0, 0};
    for (const auto& v : verts) {
        sum += v;
    }
    return sum / static_cast<float>(verts.size());
}

} // namespace teleop
