#include "engine/systems/kinematics_system.hpp"
#include "components/transform.hpp"
#include "components/velocity.hpp"
#include "components/robot_tag.hpp"

namespace teleop {

KinematicsSystem::KinematicsSystem(IRobotKinematics& kinematics)
    : kinematics_(kinematics) {}

void KinematicsSystem::tick(entt::registry& registry, float dt) {
    auto view = registry.view<Transform, Velocity, RobotTag>();
    for (auto [entity, transform, velocity, robot] : view.each()) {
        RobotKinematicsState state;
        state.x = transform.x;
        state.y = transform.y;
        state.heading = transform.heading;
        state.speed = velocity.speed;
        state.steeringAngle = robot.steeringAngle;

        auto result = kinematics_.computeStep(state, robot.throttle, robot.steeringInput, dt);

        transform.x = result.x;
        transform.y = result.y;
        transform.heading = result.heading;
        velocity.speed = result.speed;
        robot.steeringAngle = result.steeringAngle;
    }
}

} // namespace teleop
