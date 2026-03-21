#pragma once

#include <cstdint>

namespace teleop {

struct SessionState {
    bool operatorConnected = false;
    uint64_t lastCommandTimestamp = 0;
    uint32_t commandSequence = 0;
    uint32_t stateSequence = 0;
};

} // namespace teleop
