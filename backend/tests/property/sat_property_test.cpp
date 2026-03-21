#include <gtest/gtest.h>
#include <rapidcheck.h>
#include <rapidcheck/gtest.h>
#include "physics/sat_detector.hpp"
#include <cmath>

using namespace teleop;

namespace {

// Generate a float in [lo, hi] via integer mapping (avoids std::make_unsigned<float> issue)
rc::Gen<float> genFloat(float lo, float hi) {
    return rc::gen::map(rc::gen::inRange(0, 10001), [=](int i) {
        return lo + (hi - lo) * static_cast<float>(i) / 10000.0f;
    });
}

// Generate a convex polygon by sampling angles and sorting
rc::Gen<std::vector<Vec2>> genConvexPolygon(int minVerts = 3, int maxVerts = 8, float maxRadius = 5.0f) {
    return rc::gen::exec([=]() {
        int n = *rc::gen::inRange(minVerts, maxVerts + 1);
        float cx = *genFloat(-10.0f, 10.0f);
        float cy = *genFloat(-10.0f, 10.0f);

        // Generate sorted angles
        std::vector<float> angles;
        for (int i = 0; i < n; ++i) {
            angles.push_back(*genFloat(0.0f, 2.0f * static_cast<float>(M_PI)));
        }
        std::sort(angles.begin(), angles.end());

        // Remove near-duplicate angles
        auto last = std::unique(angles.begin(), angles.end(),
            [](float a, float b) { return std::abs(a - b) < 0.1f; });
        angles.erase(last, angles.end());
        if (angles.size() < 3) {
            angles = {0.0f, 2.094f, 4.189f}; // equilateral fallback
        }

        std::vector<Vec2> verts;
        for (float angle : angles) {
            float r = *genFloat(0.5f, maxRadius);
            verts.push_back({cx + r * std::cos(angle), cy + r * std::sin(angle)});
        }
        return verts;
    });
}

std::vector<Vec2> translatePoly(const std::vector<Vec2>& poly, Vec2 offset) {
    std::vector<Vec2> result;
    result.reserve(poly.size());
    for (const auto& v : poly) {
        result.push_back(v + offset);
    }
    return result;
}

} // namespace

RC_GTEST_PROP(SATPropertyTest, CollisionIsSymmetric, ()) {
    auto polyA = *genConvexPolygon(3, 8, 5.0f);
    auto polyB = *genConvexPolygon(3, 8, 5.0f);

    SATDetector sat;
    auto resultAB = sat.test(polyA, polyB);
    auto resultBA = sat.test(polyB, polyA);

    RC_ASSERT(resultAB.colliding == resultBA.colliding);
}

RC_GTEST_PROP(SATPropertyTest, SeparatedPolygonsDoNotCollide, ()) {
    auto poly = *genConvexPolygon(3, 6, 1.0f);
    float separation = *genFloat(15.0f, 100.0f);

    auto polyA = translatePoly(poly, {0, 0});
    auto polyB = translatePoly(poly, {separation, 0});

    SATDetector sat;
    auto result = sat.test(polyA, polyB);
    RC_ASSERT(!result.colliding);
}

RC_GTEST_PROP(SATPropertyTest, PolygonCollidesWithItself, ()) {
    auto poly = *genConvexPolygon(3, 8, 5.0f);

    SATDetector sat;
    auto result = sat.test(poly, poly);
    RC_ASSERT(result.colliding);
}

RC_GTEST_PROP(SATPropertyTest, MTVResolvesCollision, ()) {
    auto polyA = *genConvexPolygon(3, 6, 2.0f);
    float offx = *genFloat(-0.5f, 0.5f);
    float offy = *genFloat(-0.5f, 0.5f);
    auto polyB = translatePoly(polyA, {offx, offy});

    SATDetector sat;
    auto result = sat.test(polyA, polyB);

    if (result.colliding && result.penetration > 0.001f) {
        auto resolved = translatePoly(polyA, result.normal * (result.penetration + 0.01f));
        auto afterResult = sat.test(resolved, polyB);
        RC_ASSERT(!afterResult.colliding);
    }
}
