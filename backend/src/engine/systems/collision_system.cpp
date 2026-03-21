#include "engine/systems/collision_system.hpp"
#include "components/transform.hpp"
#include "components/polygon_shape.hpp"
#include "components/velocity.hpp"
#include "components/robot_tag.hpp"
#include "components/collision_state.hpp"
#include "components/static_tag.hpp"
#include "components/waypoint_path.hpp"
#include <cmath>

namespace teleop {

CollisionSystem::CollisionSystem(float spatialHashCellSize, ILogger& logger)
    : grid_(spatialHashCellSize), logger_(logger) {}

void CollisionSystem::tick(entt::registry& registry) {
    // Clear previous collision states for robot
    auto robotView = registry.view<RobotTag, CollisionState>();
    for (auto [entity, robot, collision] : robotView.each()) {
        collision.wasCollidingLastTick = collision.isColliding;
        collision.isColliding = false;
        collision.collidedWith = entt::null;
        collision.mtv = {0, 0};
    }

    // Build spatial hash grid
    grid_.clear();
    auto allShapes = registry.view<const PolygonShape>();
    for (auto [entity, shape] : allShapes.each()) {
        grid_.insert(entity, shape.aabb);
    }

    // Get candidate pairs
    auto candidatePairs = grid_.getCandidatePairs();

    // Narrow-phase SAT
    for (const auto& [entityA, entityB] : candidatePairs) {
        if (!registry.valid(entityA) || !registry.valid(entityB)) continue;

        // Skip obstacle-obstacle pairs (no resolution needed)
        bool aIsRobot = registry.all_of<RobotTag>(entityA);
        bool bIsRobot = registry.all_of<RobotTag>(entityB);
        if (!aIsRobot && !bIsRobot) continue;

        const auto& shapeA = registry.get<PolygonShape>(entityA);
        const auto& shapeB = registry.get<PolygonShape>(entityB);

        auto result = satDetector_.test(shapeA.worldVertices, shapeB.worldVertices);
        if (!result.colliding) continue;

        // Determine which is the robot
        entt::entity robotEntity = aIsRobot ? entityA : entityB;
        entt::entity obstacleEntity = aIsRobot ? entityB : entityA;

        // Check if dynamic obstacle can pass through robot (FR-11)
        if (registry.all_of<WaypointPath>(obstacleEntity)) {
            // Check if this is robot-initiated or obstacle-initiated collision
            Vec2 normal = aIsRobot ? result.normal : -result.normal;
            if (!isRobotInitiatedCollision(registry, robotEntity, normal)) {
                continue; // Dynamic obstacle passes through
            }
        }

        // Update robot collision state
        auto& collision = registry.get<CollisionState>(robotEntity);
        collision.isColliding = true;
        collision.collidedWith = obstacleEntity;
        // MTV should push robot away from obstacle
        collision.mtv = aIsRobot
            ? result.normal * result.penetration
            : -(result.normal * result.penetration);
    }
}

bool CollisionSystem::isRobotInitiatedCollision(
    entt::registry& registry,
    entt::entity robotEntity,
    const Vec2& collisionNormal
) const {
    if (!registry.all_of<Velocity, Transform>(robotEntity)) return false;

    const auto& velocity = registry.get<Velocity>(robotEntity);
    const auto& transform = registry.get<Transform>(robotEntity);

    if (std::abs(velocity.speed) < 0.01f) return false;

    Vec2 robotDir = Vec2::fromAngle(transform.heading);
    float approach = (robotDir * velocity.speed).dot(-collisionNormal);
    return approach > 0.0f;
}

} // namespace teleop
