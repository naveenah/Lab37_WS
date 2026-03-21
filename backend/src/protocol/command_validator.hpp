#pragma once

#include "protocol/command_message.hpp"
#include <string>
#include <cstdint>

namespace teleop {

struct ValidationResult {
    bool valid = false;
    std::string errorCode;
    std::string errorMessage;
};

class CommandValidator {
public:
    static constexpr float MAX_THROTTLE = 1.0f;
    static constexpr float MIN_THROTTLE = -1.0f;
    static constexpr float MAX_STEERING = 1.0f;
    static constexpr float MIN_STEERING = -1.0f;
    static constexpr uint32_t MAX_SEQUENCE_GAP = 1000;

    static ValidationResult validate(
        const CommandMessage& cmd,
        uint32_t lastSequence,
        uint64_t lastTimestamp
    ) {
        if (cmd.throttle < MIN_THROTTLE || cmd.throttle > MAX_THROTTLE) {
            return {false, "INVALID_THROTTLE", "Throttle out of range"};
        }
        if (cmd.steering < MIN_STEERING || cmd.steering > MAX_STEERING) {
            return {false, "INVALID_STEERING", "Steering out of range"};
        }
        if (lastSequence > 0 && cmd.sequence <= lastSequence) {
            return {false, "STALE_COMMAND", "Sequence number not monotonic"};
        }
        if (lastSequence > 0 && cmd.sequence - lastSequence > MAX_SEQUENCE_GAP) {
            return {false, "SEQUENCE_GAP", "Suspicious sequence gap"};
        }
        if (lastTimestamp > 0 && cmd.timestamp < lastTimestamp) {
            return {false, "TIMESTAMP_REGRESSION", "Command timestamp went backwards"};
        }
        return {true, "", ""};
    }
};

} // namespace teleop
