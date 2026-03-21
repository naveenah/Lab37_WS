#pragma once

#include "math/vec2.hpp"
#include <vector>
#include <cstddef>

namespace teleop {

struct WaypointPath {
    std::vector<Vec2> waypoints;      // Ordered path waypoints
    size_t currentIndex = 0;          // Current target waypoint
    float speed = 1.5f;              // Movement speed (m/s)
    bool loop = true;                 // Loop back to start?
};

} // namespace teleop
