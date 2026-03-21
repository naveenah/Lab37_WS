#pragma once

#include "physics/spatial_hash_grid.hpp"
#include "physics/sat_detector.hpp"
#include "interfaces/i_logger.hpp"
#include <entt/entt.hpp>

namespace teleop {

class CollisionSystem {
public:
    CollisionSystem(float spatialHashCellSize, ILogger& logger);
    void tick(entt::registry& registry);

private:
    SpatialHashGrid grid_;
    SATDetector satDetector_;
    ILogger& logger_;

    bool isRobotInitiatedCollision(
        entt::registry& registry,
        entt::entity robotEntity,
        const Vec2& collisionNormal
    ) const;
};

} // namespace teleop
