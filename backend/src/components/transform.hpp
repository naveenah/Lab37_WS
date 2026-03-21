#pragma once

namespace teleop {

struct Transform {
    float x = 0.0f;          // World position X (meters)
    float y = 0.0f;          // World position Y (meters)
    float heading = 0.0f;    // Rotation in radians (0 = East, CCW positive)
};

} // namespace teleop
