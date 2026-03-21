#pragma once

#include <cstdint>

namespace teleop {

enum class CommandType : uint8_t {
    Control = 0,
    Handshake = 1,
    Ping = 2,
    Reset = 3,
};

struct CommandMessage {
    CommandType type = CommandType::Control;
    float throttle = 0.0f;        // -1.0 (full reverse) to 1.0 (full forward)
    float steering = 0.0f;        // -1.0 (full left) to 1.0 (full right)
    uint32_t sequence = 0;        // Monotonically increasing per session
    uint64_t timestamp = 0;       // Client-side Unix ms timestamp
};

} // namespace teleop
