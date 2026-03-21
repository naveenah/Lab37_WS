#pragma once

#include "math/vec2.hpp"
#include <vector>
#include <cstdint>

namespace teleop {

enum class EntityType : uint8_t {
    Robot = 0,
    StaticObstacle = 1,
    DynamicObstacle = 2,
};

struct EntitySnapshot {
    uint32_t entityId = 0;
    EntityType entityType = EntityType::Robot;
    float x = 0.0f;
    float y = 0.0f;
    float heading = 0.0f;
    std::vector<Vec2> vertices;
    uint32_t color = 0xFFFFFFFF;
};

} // namespace teleop
