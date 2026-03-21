#include <gtest/gtest.h>
#include "math/aabb.hpp"

using namespace teleop;

TEST(AABBTest, DefaultConstruction) {
    AABB aabb;
    EXPECT_FLOAT_EQ(aabb.min.x, 0.0f);
    EXPECT_FLOAT_EQ(aabb.min.y, 0.0f);
    EXPECT_FLOAT_EQ(aabb.max.x, 0.0f);
    EXPECT_FLOAT_EQ(aabb.max.y, 0.0f);
}

TEST(AABBTest, OverlappingBoxes) {
    AABB a{{0, 0}, {2, 2}};
    AABB b{{1, 1}, {3, 3}};
    EXPECT_TRUE(a.overlaps(b));
    EXPECT_TRUE(b.overlaps(a));
}

TEST(AABBTest, NonOverlappingBoxes) {
    AABB a{{0, 0}, {1, 1}};
    AABB b{{2, 2}, {3, 3}};
    EXPECT_FALSE(a.overlaps(b));
    EXPECT_FALSE(b.overlaps(a));
}

TEST(AABBTest, TouchingEdges) {
    AABB a{{0, 0}, {1, 1}};
    AABB b{{1, 0}, {2, 1}};
    EXPECT_TRUE(a.overlaps(b));
}

TEST(AABBTest, ContainedBox) {
    AABB outer{{0, 0}, {10, 10}};
    AABB inner{{2, 2}, {5, 5}};
    EXPECT_TRUE(outer.overlaps(inner));
    EXPECT_TRUE(inner.overlaps(outer));
}

TEST(AABBTest, SeparatedHorizontally) {
    AABB a{{0, 0}, {1, 1}};
    AABB b{{5, 0}, {6, 1}};
    EXPECT_FALSE(a.overlaps(b));
}

TEST(AABBTest, SeparatedVertically) {
    AABB a{{0, 0}, {1, 1}};
    AABB b{{0, 5}, {1, 6}};
    EXPECT_FALSE(a.overlaps(b));
}

TEST(AABBTest, Center) {
    AABB aabb{{2, 4}, {6, 8}};
    Vec2 c = aabb.center();
    EXPECT_FLOAT_EQ(c.x, 4.0f);
    EXPECT_FLOAT_EQ(c.y, 6.0f);
}

TEST(AABBTest, HalfExtents) {
    AABB aabb{{0, 0}, {4, 6}};
    Vec2 he = aabb.halfExtents();
    EXPECT_FLOAT_EQ(he.x, 2.0f);
    EXPECT_FLOAT_EQ(he.y, 3.0f);
}

TEST(AABBTest, WidthAndHeight) {
    AABB aabb{{1, 2}, {5, 7}};
    EXPECT_FLOAT_EQ(aabb.width(), 4.0f);
    EXPECT_FLOAT_EQ(aabb.height(), 5.0f);
}

TEST(AABBTest, ExpandWithPoint) {
    AABB aabb{{0, 0}, {1, 1}};
    AABB expanded = aabb.expand({2, 3});
    EXPECT_FLOAT_EQ(expanded.max.x, 2.0f);
    EXPECT_FLOAT_EQ(expanded.max.y, 3.0f);
    EXPECT_FLOAT_EQ(expanded.min.x, 0.0f);
    EXPECT_FLOAT_EQ(expanded.min.y, 0.0f);
}

TEST(AABBTest, ExpandWithNegativePoint) {
    AABB aabb{{0, 0}, {1, 1}};
    AABB expanded = aabb.expand({-2, -3});
    EXPECT_FLOAT_EQ(expanded.min.x, -2.0f);
    EXPECT_FLOAT_EQ(expanded.min.y, -3.0f);
    EXPECT_FLOAT_EQ(expanded.max.x, 1.0f);
    EXPECT_FLOAT_EQ(expanded.max.y, 1.0f);
}

TEST(AABBTest, FromVertices) {
    Vec2 verts[] = {{-1, -2}, {3, 1}, {0, 4}};
    AABB aabb = AABB::fromVertices(verts, 3);
    EXPECT_FLOAT_EQ(aabb.min.x, -1.0f);
    EXPECT_FLOAT_EQ(aabb.min.y, -2.0f);
    EXPECT_FLOAT_EQ(aabb.max.x, 3.0f);
    EXPECT_FLOAT_EQ(aabb.max.y, 4.0f);
}

TEST(AABBTest, OverlapIsSymmetric) {
    AABB a{{0, 0}, {3, 3}};
    AABB b{{2, 2}, {5, 5}};
    EXPECT_EQ(a.overlaps(b), b.overlaps(a));
}
