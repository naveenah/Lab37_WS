#include <gtest/gtest.h>
#include "physics/spatial_hash_grid.hpp"

using namespace teleop;

TEST(SpatialHashTest, NoEntitiesNoPairs) {
    SpatialHashGrid grid(2.0f);
    auto pairs = grid.getCandidatePairs();
    EXPECT_TRUE(pairs.empty());
}

TEST(SpatialHashTest, SingleEntityNoPairs) {
    SpatialHashGrid grid(2.0f);
    grid.insert(entt::entity{0}, AABB{{0, 0}, {1, 1}});
    auto pairs = grid.getCandidatePairs();
    EXPECT_TRUE(pairs.empty());
}

TEST(SpatialHashTest, OverlappingAABBsProducePair) {
    SpatialHashGrid grid(2.0f);
    grid.insert(entt::entity{0}, AABB{{0, 0}, {1, 1}});
    grid.insert(entt::entity{1}, AABB{{0.5f, 0.5f}, {1.5f, 1.5f}});
    auto pairs = grid.getCandidatePairs();
    EXPECT_EQ(pairs.size(), 1u);
}

TEST(SpatialHashTest, DistantAABBsNoPair) {
    SpatialHashGrid grid(2.0f);
    grid.insert(entt::entity{0}, AABB{{0, 0}, {1, 1}});
    grid.insert(entt::entity{1}, AABB{{10, 10}, {11, 11}});
    auto pairs = grid.getCandidatePairs();
    EXPECT_TRUE(pairs.empty());
}

TEST(SpatialHashTest, NoDuplicatePairs) {
    SpatialHashGrid grid(1.0f);
    // Entity spanning multiple cells
    grid.insert(entt::entity{0}, AABB{{0, 0}, {2, 2}});
    grid.insert(entt::entity{1}, AABB{{1, 1}, {3, 3}});
    auto pairs = grid.getCandidatePairs();
    EXPECT_EQ(pairs.size(), 1u);
}

TEST(SpatialHashTest, MultipleEntitiesMultiplePairs) {
    SpatialHashGrid grid(5.0f);
    grid.insert(entt::entity{0}, AABB{{0, 0}, {1, 1}});
    grid.insert(entt::entity{1}, AABB{{0.5f, 0}, {1.5f, 1}});
    grid.insert(entt::entity{2}, AABB{{0.25f, 0.25f}, {0.75f, 0.75f}});
    auto pairs = grid.getCandidatePairs();
    // All 3 entities are in the same cell, so we get 3 pairs: (0,1), (0,2), (1,2)
    EXPECT_EQ(pairs.size(), 3u);
}

TEST(SpatialHashTest, ClearRemovesAllEntities) {
    SpatialHashGrid grid(2.0f);
    grid.insert(entt::entity{0}, AABB{{0, 0}, {1, 1}});
    grid.insert(entt::entity{1}, AABB{{0.5f, 0.5f}, {1.5f, 1.5f}});
    grid.clear();
    auto pairs = grid.getCandidatePairs();
    EXPECT_TRUE(pairs.empty());
}

TEST(SpatialHashTest, EntitiesInDifferentCellsNoPair) {
    SpatialHashGrid grid(5.0f);
    // These are in different cells of a 5.0 grid
    grid.insert(entt::entity{0}, AABB{{0, 0}, {1, 1}});
    grid.insert(entt::entity{1}, AABB{{6, 6}, {7, 7}});
    auto pairs = grid.getCandidatePairs();
    EXPECT_TRUE(pairs.empty());
}
