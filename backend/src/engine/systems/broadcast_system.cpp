#include "engine/systems/broadcast_system.hpp"
#include "components/robot_tag.hpp"
#include "components/velocity.hpp"

namespace teleop {

BroadcastSystem::BroadcastSystem(IStateStore& stateStore, IMessageBus& messageBus)
    : stateStore_(stateStore), messageBus_(messageBus) {}

void BroadcastSystem::tick(entt::registry& registry, uint64_t tickNumber) {
    auto entities = stateStore_.getEntitySnapshots();
    auto score = stateStore_.getScoreState();

    // Get robot speed and steering for the broadcast
    float robotSpeed = 0.0f;
    float robotSteering = 0.0f;
    auto robotView = registry.view<const RobotTag, const Velocity>();
    for (auto [entity, robot, vel] : robotView.each()) {
        robotSpeed = vel.speed;
        robotSteering = robot.steeringAngle;
        break; // Only one robot
    }

    auto [data, size] = serializer_.serialize(
        entities, score, stateSequence_++, tickNumber, robotSpeed, robotSteering
    );

    messageBus_.publish(Topic::StateBroadcast, {data, size});
}

} // namespace teleop
