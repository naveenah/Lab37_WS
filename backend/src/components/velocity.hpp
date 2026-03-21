#pragma once

namespace teleop {

struct Velocity {
    float speed = 0.0f;           // Linear speed (m/s), positive = forward
    float angularVelocity = 0.0f; // Rotational velocity (rad/s)
};

} // namespace teleop
