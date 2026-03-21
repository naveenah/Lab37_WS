#pragma once

#include <cstdint>

namespace teleop {

struct ScoreState {
    uint32_t impactCount = 0;         // Total collision events
    uint64_t sessionStartMs = 0;      // Session start timestamp
};

} // namespace teleop
