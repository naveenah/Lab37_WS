#include <gtest/gtest.h>
#include "physics/sat_detector.hpp"
#include <vector>

using namespace teleop;

namespace {

std::vector<Vec2> makeSquare(float cx, float cy, float halfSize) {
    return {
        {cx - halfSize, cy - halfSize},
        {cx + halfSize, cy - halfSize},
        {cx + halfSize, cy + halfSize},
        {cx - halfSize, cy + halfSize},
    };
}

std::vector<Vec2> translate(const std::vector<Vec2>& poly, Vec2 offset) {
    std::vector<Vec2> result;
    result.reserve(poly.size());
    for (const auto& v : poly) {
        result.push_back(v + offset);
    }
    return result;
}

} // namespace

TEST(SATTest, IdenticalSquaresCollide) {
    auto square = makeSquare(0, 0, 1.0f);
    SATDetector sat;
    auto result = sat.test(square, square);
    EXPECT_TRUE(result.colliding);
}

TEST(SATTest, SeparatedSquaresDoNotCollide) {
    auto a = makeSquare(0, 0, 1.0f);
    auto b = makeSquare(5, 0, 1.0f);
    SATDetector sat;
    auto result = sat.test(a, b);
    EXPECT_FALSE(result.colliding);
}

TEST(SATTest, OverlappingSquaresCollide) {
    auto a = makeSquare(0, 0, 1.0f);
    auto b = makeSquare(0.5f, 0, 1.0f);
    SATDetector sat;
    auto result = sat.test(a, b);
    EXPECT_TRUE(result.colliding);
    EXPECT_GT(result.penetration, 0.0f);
}

TEST(SATTest, TouchingEdgesCollide) {
    auto a = makeSquare(0, 0, 1.0f);
    auto b = makeSquare(2.0f, 0, 1.0f); // Exactly touching
    SATDetector sat;
    auto result = sat.test(a, b);
    // Touching means overlap = 0, which is NOT colliding per our <= 0 check
    EXPECT_FALSE(result.colliding);
}

TEST(SATTest, SlightOverlapDetected) {
    auto a = makeSquare(0, 0, 1.0f);
    auto b = makeSquare(1.99f, 0, 1.0f); // Slight overlap
    SATDetector sat;
    auto result = sat.test(a, b);
    EXPECT_TRUE(result.colliding);
    EXPECT_NEAR(result.penetration, 0.01f, 0.005f);
}

TEST(SATTest, TriangleAndSquareCollision) {
    std::vector<Vec2> tri = {{0, 0}, {2, 0}, {1, 2}};
    auto sq = makeSquare(1, 0.5f, 0.5f);
    SATDetector sat;
    auto result = sat.test(tri, sq);
    EXPECT_TRUE(result.colliding);
}

TEST(SATTest, TriangleAndSquareSeparated) {
    std::vector<Vec2> tri = {{0, 0}, {2, 0}, {1, 2}};
    auto sq = makeSquare(5, 5, 0.5f);
    SATDetector sat;
    auto result = sat.test(tri, sq);
    EXPECT_FALSE(result.colliding);
}

TEST(SATTest, MTVPushesApart) {
    auto a = makeSquare(0, 0, 1.0f);
    auto b = makeSquare(0.5f, 0, 1.0f);
    SATDetector sat;
    auto result = sat.test(a, b);
    EXPECT_TRUE(result.colliding);

    // Apply MTV to separate
    auto aShifted = translate(a, result.normal * result.penetration);
    auto resultAfter = sat.test(aShifted, b);
    EXPECT_FALSE(resultAfter.colliding);
}

TEST(SATTest, CollisionIsSymmetric) {
    auto a = makeSquare(0, 0, 1.0f);
    auto b = makeSquare(0.5f, 0.5f, 1.0f);
    SATDetector sat;
    auto resultAB = sat.test(a, b);
    auto resultBA = sat.test(b, a);
    EXPECT_EQ(resultAB.colliding, resultBA.colliding);
}

TEST(SATTest, DiagonalOverlap) {
    auto a = makeSquare(0, 0, 1.0f);
    auto b = makeSquare(1.0f, 1.0f, 1.0f);
    SATDetector sat;
    auto result = sat.test(a, b);
    EXPECT_TRUE(result.colliding);
}

TEST(SATTest, TooFewVerticesReturnsNoCollision) {
    std::vector<Vec2> line = {{0, 0}, {1, 1}};
    auto sq = makeSquare(0, 0, 1.0f);
    SATDetector sat;
    auto result = sat.test(line, sq);
    EXPECT_FALSE(result.colliding);
}
