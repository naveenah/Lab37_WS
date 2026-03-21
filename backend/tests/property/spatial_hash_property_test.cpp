#include <gtest/gtest.h>
#include <rapidcheck.h>
#include <rapidcheck/gtest.h>
#include "physics/spatial_hash_grid.hpp"
#include <algorithm>

using namespace teleop;

namespace {

rc::Gen<float> genFloat(float lo, float hi) {
    return rc::gen::map(rc::gen::inRange(0, 10001), [=](int i) {
        return lo + (hi - lo) * static_cast<float>(i) / 10000.0f;
    });
}

} // namespace

RC_GTEST_PROP(SpatialHashPropertyTest, AllOverlappingPairsFound, ()) {
    int n = *rc::gen::inRange(2, 30);
    float cellSize = *genFloat(1.0f, 10.0f);

    SpatialHashGrid grid(cellSize);
    std::vector<std::pair<entt::entity, AABB>> entities;

    for (int i = 0; i < n; ++i) {
        float x = *genFloat(-20.0f, 20.0f);
        float y = *genFloat(-20.0f, 20.0f);
        float size = *genFloat(0.5f, 3.0f);
        AABB aabb{{x, y}, {x + size, y + size}};
        auto entity = static_cast<entt::entity>(i);
        grid.insert(entity, aabb);
        entities.push_back({entity, aabb});
    }

    auto pairs = grid.getCandidatePairs();

    // Verify: every actually overlapping pair appears in candidates
    for (size_t i = 0; i < entities.size(); ++i) {
        for (size_t j = i + 1; j < entities.size(); ++j) {
            if (entities[i].second.overlaps(entities[j].second)) {
                auto a = std::min(entities[i].first, entities[j].first);
                auto b = std::max(entities[i].first, entities[j].first);
                bool found = std::any_of(pairs.begin(), pairs.end(),
                    [&](const auto& p) { return p.first == a && p.second == b; });
                RC_ASSERT(found);
            }
        }
    }
}

RC_GTEST_PROP(SpatialHashPropertyTest, NoDuplicatePairsReturned, ()) {
    int n = *rc::gen::inRange(2, 20);
    SpatialHashGrid grid(2.0f);

    for (int i = 0; i < n; ++i) {
        float x = *genFloat(-5.0f, 5.0f);
        float y = *genFloat(-5.0f, 5.0f);
        grid.insert(static_cast<entt::entity>(i), AABB{{x, y}, {x + 1, y + 1}});
    }

    auto pairs = grid.getCandidatePairs();

    // Check no duplicates
    std::set<std::pair<entt::entity, entt::entity>> uniquePairs(pairs.begin(), pairs.end());
    RC_ASSERT(uniquePairs.size() == pairs.size());
}
