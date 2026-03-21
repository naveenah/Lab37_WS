#include "engine/systems/input_system.hpp"
#include "components/robot_tag.hpp"
#include "components/session_state.hpp"

namespace teleop {

InputSystem::InputSystem(ICommandSource& commandSource, ILogger& logger)
    : commandSource_(commandSource), logger_(logger) {}

void InputSystem::tick(entt::registry& registry) {
    CommandMessage cmd;
    while (commandSource_.tryDequeue(cmd)) {
        if (cmd.type == CommandType::Control) {
            auto view = registry.view<RobotTag>();
            for (auto [entity, robot] : view.each()) {
                robot.throttle = cmd.throttle;
                robot.steeringInput = cmd.steering;
            }

            if (registry.ctx().contains<SessionState>()) {
                auto& session = registry.ctx().get<SessionState>();
                session.commandSequence = cmd.sequence;
                session.lastCommandTimestamp = cmd.timestamp;
            }
        }
    }
}

} // namespace teleop
