#include "infra/ecs_state_store.hpp"
#include "components/transform.hpp"
#include "components/polygon_shape.hpp"
#include "components/velocity.hpp"
#include "components/robot_tag.hpp"
#include "components/static_tag.hpp"
#include "components/waypoint_path.hpp"
#include "components/render_meta.hpp"
#include "components/score_state.hpp"

namespace teleop {

ECSStateStore::ECSStateStore(entt::registry& registry)
    : registry_(registry) {}

std::vector<EntitySnapshot> ECSStateStore::getEntitySnapshots() const {
    std::vector<EntitySnapshot> snapshots;

    auto view = registry_.view<const Transform, const PolygonShape, const RenderMeta>();
    snapshots.reserve(view.size_hint());

    for (auto [entity, transform, shape, meta] : view.each()) {
        EntitySnapshot snap;
        snap.entityId = static_cast<uint32_t>(entity);
        snap.x = transform.x;
        snap.y = transform.y;
        snap.heading = transform.heading;
        snap.vertices = shape.worldVertices;
        snap.color = meta.color;

        if (registry_.all_of<RobotTag>(entity)) {
            snap.entityType = EntityType::Robot;
        } else if (registry_.all_of<StaticTag>(entity)) {
            snap.entityType = EntityType::StaticObstacle;
        } else if (registry_.all_of<WaypointPath>(entity)) {
            snap.entityType = EntityType::DynamicObstacle;
        } else {
            snap.entityType = EntityType::StaticObstacle;
        }

        snapshots.push_back(std::move(snap));
    }

    return snapshots;
}

ScoreState ECSStateStore::getScoreState() const {
    if (registry_.ctx().contains<ScoreState>()) {
        return registry_.ctx().get<ScoreState>();
    }
    return {};
}

} // namespace teleop
