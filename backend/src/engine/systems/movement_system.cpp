#include "engine/systems/movement_system.hpp"
#include "components/transform.hpp"
#include "components/velocity.hpp"
#include "components/waypoint_path.hpp"
#include <cmath>

namespace teleop {

void MovementSystem::tick(entt::registry& registry, float dt) {
    auto view = registry.view<Transform, Velocity, WaypointPath>();
    for (auto [entity, transform, velocity, path] : view.each()) {
        if (path.waypoints.empty()) continue;

        const Vec2& target = path.waypoints[path.currentIndex];
        Vec2 current{transform.x, transform.y};
        Vec2 direction = target - current;
        float distance = direction.length();

        if (distance < 0.1f) {
            // Reached waypoint — advance to next
            path.currentIndex++;
            if (path.currentIndex >= path.waypoints.size()) {
                if (path.loop) {
                    path.currentIndex = 0;
                } else {
                    path.currentIndex = path.waypoints.size() - 1;
                    velocity.speed = 0.0f;
                    continue;
                }
            }
            continue;
        }

        // Move toward waypoint
        Vec2 dir = direction.normalized();
        float moveDistance = path.speed * dt;
        if (moveDistance > distance) {
            moveDistance = distance;
        }

        transform.x += dir.x * moveDistance;
        transform.y += dir.y * moveDistance;
        transform.heading = std::atan2(dir.y, dir.x);
        velocity.speed = path.speed;
    }
}

} // namespace teleop
