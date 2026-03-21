#pragma once

#include "interfaces/i_command_source.hpp"
#include "interfaces/i_logger.hpp"
#include <entt/entt.hpp>

namespace teleop {

class InputSystem {
public:
    InputSystem(ICommandSource& commandSource, ILogger& logger);
    void tick(entt::registry& registry);

private:
    ICommandSource& commandSource_;
    ILogger& logger_;
};

} // namespace teleop
