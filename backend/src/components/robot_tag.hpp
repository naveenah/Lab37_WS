#pragma once

namespace teleop {

struct RobotTag {
    float steeringAngle = 0.0f;   // Current steering angle (radians)
    float wheelbase = 2.0f;       // Distance between axles (meters)
    float maxSpeed = 5.0f;        // Maximum forward speed (m/s)
    float maxSteeringAngle = 0.6f;// Maximum steering angle (radians, ~34 degrees)
    float throttle = 0.0f;        // Current throttle input (-1.0 to 1.0)
    float steeringInput = 0.0f;   // Current steering input (-1.0 to 1.0)
};

} // namespace teleop
