#pragma once

namespace teleop {

struct RobotKinematicsState {
    float x = 0.0f;
    float y = 0.0f;
    float heading = 0.0f;
    float speed = 0.0f;
    float steeringAngle = 0.0f;
};

class IRobotKinematics {
public:
    virtual ~IRobotKinematics() = default;

    virtual RobotKinematicsState computeStep(
        RobotKinematicsState state,
        float throttle,
        float steeringInput,
        float dt
    ) = 0;
};

} // namespace teleop
