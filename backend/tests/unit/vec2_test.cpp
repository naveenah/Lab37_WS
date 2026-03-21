#include <gtest/gtest.h>
#include "math/vec2.hpp"
#include <cmath>

using namespace teleop;

TEST(Vec2Test, DefaultConstruction) {
    Vec2 v;
    EXPECT_FLOAT_EQ(v.x, 0.0f);
    EXPECT_FLOAT_EQ(v.y, 0.0f);
}

TEST(Vec2Test, ParameterizedConstruction) {
    Vec2 v{3.0f, 4.0f};
    EXPECT_FLOAT_EQ(v.x, 3.0f);
    EXPECT_FLOAT_EQ(v.y, 4.0f);
}

TEST(Vec2Test, Addition) {
    Vec2 a{1.0f, 2.0f};
    Vec2 b{3.0f, 4.0f};
    Vec2 c = a + b;
    EXPECT_FLOAT_EQ(c.x, 4.0f);
    EXPECT_FLOAT_EQ(c.y, 6.0f);
}

TEST(Vec2Test, Subtraction) {
    Vec2 a{5.0f, 7.0f};
    Vec2 b{2.0f, 3.0f};
    Vec2 c = a - b;
    EXPECT_FLOAT_EQ(c.x, 3.0f);
    EXPECT_FLOAT_EQ(c.y, 4.0f);
}

TEST(Vec2Test, ScalarMultiplication) {
    Vec2 v{3.0f, 4.0f};
    Vec2 c = v * 2.0f;
    EXPECT_FLOAT_EQ(c.x, 6.0f);
    EXPECT_FLOAT_EQ(c.y, 8.0f);
}

TEST(Vec2Test, ScalarMultiplicationCommutative) {
    Vec2 v{3.0f, 4.0f};
    Vec2 a = v * 2.0f;
    Vec2 b = 2.0f * v;
    EXPECT_FLOAT_EQ(a.x, b.x);
    EXPECT_FLOAT_EQ(a.y, b.y);
}

TEST(Vec2Test, Negation) {
    Vec2 v{3.0f, -4.0f};
    Vec2 neg = -v;
    EXPECT_FLOAT_EQ(neg.x, -3.0f);
    EXPECT_FLOAT_EQ(neg.y, 4.0f);
}

TEST(Vec2Test, DotProductCommutative) {
    Vec2 a{3.0f, 4.0f};
    Vec2 b{1.0f, 2.0f};
    EXPECT_FLOAT_EQ(a.dot(b), b.dot(a));
}

TEST(Vec2Test, DotProductValue) {
    Vec2 a{3.0f, 4.0f};
    Vec2 b{1.0f, 2.0f};
    EXPECT_FLOAT_EQ(a.dot(b), 11.0f);
}

TEST(Vec2Test, DotProductOrthogonal) {
    Vec2 a{1.0f, 0.0f};
    Vec2 b{0.0f, 1.0f};
    EXPECT_FLOAT_EQ(a.dot(b), 0.0f);
}

TEST(Vec2Test, CrossProduct) {
    Vec2 a{1.0f, 0.0f};
    Vec2 b{0.0f, 1.0f};
    EXPECT_FLOAT_EQ(a.cross(b), 1.0f);
    EXPECT_FLOAT_EQ(b.cross(a), -1.0f);
}

TEST(Vec2Test, Length) {
    Vec2 v{3.0f, 4.0f};
    EXPECT_FLOAT_EQ(v.length(), 5.0f);
}

TEST(Vec2Test, LengthZero) {
    Vec2 v{0.0f, 0.0f};
    EXPECT_FLOAT_EQ(v.length(), 0.0f);
}

TEST(Vec2Test, NormalizedUnitLength) {
    Vec2 v{3.0f, 4.0f};
    Vec2 n = v.normalized();
    EXPECT_NEAR(n.length(), 1.0f, 1e-6f);
}

TEST(Vec2Test, NormalizedDirection) {
    Vec2 v{3.0f, 4.0f};
    Vec2 n = v.normalized();
    EXPECT_NEAR(n.x, 0.6f, 1e-6f);
    EXPECT_NEAR(n.y, 0.8f, 1e-6f);
}

TEST(Vec2Test, NormalizedZeroVector) {
    Vec2 v{0.0f, 0.0f};
    Vec2 n = v.normalized();
    EXPECT_FLOAT_EQ(n.x, 0.0f);
    EXPECT_FLOAT_EQ(n.y, 0.0f);
}

TEST(Vec2Test, PerpendicularIsOrthogonal) {
    Vec2 v{3.0f, 4.0f};
    Vec2 perp = v.perpendicular();
    EXPECT_NEAR(v.dot(perp), 0.0f, 1e-6f);
}

TEST(Vec2Test, PerpendicularPreservesLength) {
    Vec2 v{3.0f, 4.0f};
    Vec2 perp = v.perpendicular();
    EXPECT_NEAR(v.length(), perp.length(), 1e-6f);
}

TEST(Vec2Test, Distance) {
    Vec2 a{0.0f, 0.0f};
    Vec2 b{3.0f, 4.0f};
    EXPECT_FLOAT_EQ(a.distance(b), 5.0f);
}

TEST(Vec2Test, DistanceSymmetric) {
    Vec2 a{1.0f, 2.0f};
    Vec2 b{4.0f, 6.0f};
    EXPECT_FLOAT_EQ(a.distance(b), b.distance(a));
}

TEST(Vec2Test, FromAngle) {
    Vec2 v = Vec2::fromAngle(0.0f);
    EXPECT_NEAR(v.x, 1.0f, 1e-6f);
    EXPECT_NEAR(v.y, 0.0f, 1e-6f);

    Vec2 up = Vec2::fromAngle(static_cast<float>(M_PI / 2.0));
    EXPECT_NEAR(up.x, 0.0f, 1e-6f);
    EXPECT_NEAR(up.y, 1.0f, 1e-6f);
}

TEST(Vec2Test, CompoundAssignment) {
    Vec2 v{1.0f, 2.0f};
    v += Vec2{3.0f, 4.0f};
    EXPECT_FLOAT_EQ(v.x, 4.0f);
    EXPECT_FLOAT_EQ(v.y, 6.0f);

    v -= Vec2{1.0f, 1.0f};
    EXPECT_FLOAT_EQ(v.x, 3.0f);
    EXPECT_FLOAT_EQ(v.y, 5.0f);

    v *= 2.0f;
    EXPECT_FLOAT_EQ(v.x, 6.0f);
    EXPECT_FLOAT_EQ(v.y, 10.0f);
}
