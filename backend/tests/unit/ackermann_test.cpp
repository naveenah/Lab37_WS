#include <gtest/gtest.h>
#include "physics/ackermann_model.hpp"
#include <cmath>

using namespace teleop;

TEST(AckermannTest, StraightLineMotion) {
    AckermannModel model(2.0f, 0.6f, 5.0f);
    RobotKinematicsState state{0, 0, 0, 2.0f, 0.0f};
    auto result = model.computeStep(state, 1.0f, 0.0f, 1.0f);
    EXPECT_GT(result.x, state.x);          // Moved forward
    EXPECT_NEAR(result.y, 0.0f, 1e-5f);   // No lateral drift
    EXPECT_NEAR(result.heading, 0.0f, 1e-5f); // No turning
}

TEST(AckermannTest, TurningChangesHeading) {
    AckermannModel model(2.0f, 0.6f, 5.0f);
    RobotKinematicsState state{0, 0, 0, 2.0f, 0.0f};
    // Apply many steps with steering to overcome smooth steering rate
    for (int i = 0; i < 60; ++i) {
        state = model.computeStep(state, 1.0f, 1.0f, 0.016f);
    }
    EXPECT_NE(state.heading, 0.0f);
}

TEST(AckermannTest, ZeroSpeedNoMovement) {
    AckermannModel model(2.0f, 0.6f, 5.0f);
    RobotKinematicsState state{5, 3, 1.0f, 0.0f, 0.0f};
    auto result = model.computeStep(state, 0.0f, 0.0f, 1.0f);
    EXPECT_NEAR(result.x, 5.0f, 0.1f);
    EXPECT_NEAR(result.y, 3.0f, 0.1f);
}

TEST(AckermannTest, HeadingNormalization) {
    AckermannModel model(2.0f, 0.6f, 5.0f);
    RobotKinematicsState state{0, 0, 3.0f, 5.0f, 0.0f};
    for (int i = 0; i < 1000; ++i) {
        state = model.computeStep(state, 1.0f, 1.0f, 0.016f);
    }
    EXPECT_GE(state.heading, -static_cast<float>(M_PI));
    EXPECT_LE(state.heading, static_cast<float>(M_PI));
}

TEST(AckermannTest, SpeedNeverExceedsMax) {
    AckermannModel model(2.0f, 0.6f, 5.0f);
    RobotKinematicsState state{0, 0, 0, 0, 0};
    for (int i = 0; i < 1000; ++i) {
        state = model.computeStep(state, 1.0f, 0.0f, 0.016f);
    }
    EXPECT_LE(std::abs(state.speed), 5.0f + 0.01f);
}

TEST(AckermannTest, ReverseMotion) {
    AckermannModel model(2.0f, 0.6f, 5.0f);
    RobotKinematicsState state{0, 0, 0, 0, 0};
    for (int i = 0; i < 200; ++i) {
        state = model.computeStep(state, -1.0f, 0.0f, 0.016f);
    }
    EXPECT_LT(state.speed, 0.0f);
    EXPECT_LT(state.x, 0.0f); // Moved backward
}

TEST(AckermannTest, TurningRadiusStraight) {
    AckermannModel model(2.0f, 0.6f, 5.0f);
    EXPECT_TRUE(std::isinf(model.turningRadius(0.0f)));
}

TEST(AckermannTest, TurningRadiusFinite) {
    AckermannModel model(2.0f, 0.6f, 5.0f);
    float radius = model.turningRadius(0.3f);
    EXPECT_TRUE(std::isfinite(radius));
    EXPECT_GT(radius, 0.0f);
}

TEST(AckermannTest, SmoothAcceleration) {
    AckermannModel model(2.0f, 0.6f, 5.0f, 3.0f, 5.0f);
    RobotKinematicsState state{0, 0, 0, 0, 0};
    // One step should not reach max speed immediately
    auto result = model.computeStep(state, 1.0f, 0.0f, 0.016f);
    EXPECT_GT(result.speed, 0.0f);
    EXPECT_LT(result.speed, 5.0f);
}

TEST(AckermannTest, SmoothSteering) {
    AckermannModel model(2.0f, 0.6f, 5.0f, 3.0f, 5.0f, 2.0f);
    RobotKinematicsState state{0, 0, 0, 2.0f, 0.0f};
    // One step should not reach max steering immediately
    auto result = model.computeStep(state, 1.0f, 1.0f, 0.016f);
    EXPECT_GT(result.steeringAngle, 0.0f);
    EXPECT_LT(result.steeringAngle, 0.6f);
}
