#include "engine/systems/scoring_system.hpp"
#include "components/robot_tag.hpp"
#include "components/velocity.hpp"
#include "components/collision_state.hpp"
#include "components/transform.hpp"
#include "components/score_state.hpp"
#include <nlohmann/json.hpp>

namespace teleop {

ScoringSystem::ScoringSystem(ILogger& logger)
    : logger_(logger) {}

void ScoringSystem::tick(entt::registry& registry) {
    if (!registry.ctx().contains<ScoreState>()) return;
    auto& score = registry.ctx().get<ScoreState>();

    auto view = registry.view<RobotTag, Velocity, CollisionState, Transform>();
    for (auto [entity, robot, vel, collision, transform] : view.each()) {
        if (collision.isColliding && !collision.wasCollidingLastTick) {
            // New collision event
            vel.speed = 0.0f;
            vel.angularVelocity = 0.0f;
            robot.throttle = 0.0f;
            score.impactCount++;

            // Push robot out of obstacle by MTV
            transform.x += collision.mtv.x;
            transform.y += collision.mtv.y;

            // Log the collision event
            nlohmann::json eventData = {
                {"impact_count", score.impactCount},
                {"robot_x", transform.x},
                {"robot_y", transform.y},
                {"obstacle_id", static_cast<uint32_t>(collision.collidedWith)}
            };
            logger_.event("collision", eventData.dump());
        }
    }
}

} // namespace teleop
