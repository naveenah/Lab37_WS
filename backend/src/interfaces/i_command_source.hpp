#pragma once

#include "protocol/command_message.hpp"

namespace teleop {

class ICommandSource {
public:
    virtual ~ICommandSource() = default;

    virtual bool tryDequeue(CommandMessage& cmd) = 0;
};

} // namespace teleop
