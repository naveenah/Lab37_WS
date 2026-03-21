#include <gtest/gtest.h>
#include <rapidcheck.h>
#include <rapidcheck/gtest.h>
#include "physics/ackermann_model.hpp"
#include <cmath>

using namespace teleop;

namespace {

rc::Gen<float> genFloat(float lo, float hi) {
    return rc::gen::map(rc::gen::inRange(0, 10001), [=](int i) {
        return lo + (hi - lo) * static_cast<float>(i) / 10000.0f;
    });
}

} // namespace

RC_GTEST_PROP(AckermannPropertyTest, SpeedNeverExceedsMax, ()) {
    float maxSpeed = 5.0f;
    AckermannModel model(2.0f, 0.6f, maxSpeed);

    RobotKinematicsState state{0, 0, 0, 0, 0};
    float throttle = *genFloat(-1.0f, 1.0f);
    float steering = *genFloat(-1.0f, 1.0f);

    for (int i = 0; i < 500; ++i) {
        state = model.computeStep(state, throttle, steering, 0.016f);
        RC_ASSERT(std::abs(state.speed) <= maxSpeed + 0.1f);
    }
}

RC_GTEST_PROP(AckermannPropertyTest, HeadingAlwaysNormalized, ()) {
    AckermannModel model(2.0f, 0.6f, 5.0f);

    float startHeading = *genFloat(-10.0f, 10.0f);
    RobotKinematicsState state{0, 0, startHeading, 3.0f, 0.0f};

    float throttle = *genFloat(-1.0f, 1.0f);
    float steering = *genFloat(-1.0f, 1.0f);

    for (int i = 0; i < 500; ++i) {
        state = model.computeStep(state, throttle, steering, 0.016f);
        RC_ASSERT(state.heading >= -static_cast<float>(M_PI) - 0.01f);
        RC_ASSERT(state.heading <= static_cast<float>(M_PI) + 0.01f);
    }
}

RC_GTEST_PROP(AckermannPropertyTest, ZeroThrottleDeceleratesToZero, ()) {
    AckermannModel model(2.0f, 0.6f, 5.0f);

    float initialSpeed = *genFloat(0.1f, 5.0f);
    RobotKinematicsState state{0, 0, 0, initialSpeed, 0.0f};

    for (int i = 0; i < 1000; ++i) {
        state = model.computeStep(state, 0.0f, 0.0f, 0.016f);
    }
    RC_ASSERT(std::abs(state.speed) < 0.01f);
}
