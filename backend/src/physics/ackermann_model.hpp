#pragma once

#include "interfaces/i_robot_kinematics.hpp"
#include <cmath>
#include <algorithm>
#include <limits>

namespace teleop {

class AckermannModel : public IRobotKinematics {
public:
    AckermannModel(float wheelbase, float maxSteeringAngle, float maxSpeed,
                   float acceleration = 3.0f, float deceleration = 5.0f,
                   float steeringRate = 2.0f)
        : wheelbase_(wheelbase)
        , maxSteeringAngle_(maxSteeringAngle)
        , maxSpeed_(maxSpeed)
        , acceleration_(acceleration)
        , deceleration_(deceleration)
        , steeringRate_(steeringRate)
    {}

    RobotKinematicsState computeStep(
        RobotKinematicsState state,
        float throttle,
        float steeringInput,
        float dt
    ) override {
        // Clamp inputs
        float targetSteering = std::clamp(steeringInput * maxSteeringAngle_,
                                          -maxSteeringAngle_, maxSteeringAngle_);
        float targetSpeed = std::clamp(throttle, -1.0f, 1.0f) * maxSpeed_;

        // Smooth steering (simulate steering servo response)
        float steeringDiff = targetSteering - state.steeringAngle;
        state.steeringAngle += std::clamp(steeringDiff, -steeringRate_ * dt, steeringRate_ * dt);

        // Smooth acceleration/deceleration
        if (targetSpeed > state.speed) {
            state.speed = std::min(targetSpeed, state.speed + acceleration_ * dt);
        } else {
            state.speed = std::max(targetSpeed, state.speed - deceleration_ * dt);
        }

        // Ackermann integration (Euler method)
        float tanDelta = std::tan(state.steeringAngle);
        state.heading += (state.speed / wheelbase_) * tanDelta * dt;
        state.heading = normalizeAngle(state.heading);
        state.x += state.speed * std::cos(state.heading) * dt;
        state.y += state.speed * std::sin(state.heading) * dt;

        return state;
    }

    float turningRadius(float steeringAngle) const {
        if (std::abs(steeringAngle) < 1e-6f) {
            return std::numeric_limits<float>::infinity();
        }
        return wheelbase_ / std::tan(steeringAngle);
    }

    float getMaxSpeed() const { return maxSpeed_; }
    float getMaxSteeringAngle() const { return maxSteeringAngle_; }

private:
    float wheelbase_;
    float maxSteeringAngle_;
    float maxSpeed_;
    float acceleration_;
    float deceleration_;
    float steeringRate_;

    static float normalizeAngle(float a) {
        while (a > static_cast<float>(M_PI)) a -= 2.0f * static_cast<float>(M_PI);
        while (a < -static_cast<float>(M_PI)) a += 2.0f * static_cast<float>(M_PI);
        return a;
    }
};

} // namespace teleop
