#pragma once

#include "math/vec2.hpp"
#include "infra/config_loader.hpp"
#include <entt/entt.hpp>
#include <string>
#include <vector>

namespace teleop {

struct RobotDef {
    Vec2 position;
    float heading = 0.0f;
    std::vector<Vec2> vertices;
    uint32_t color = 0x00FF00FF;
    float wheelbase = 2.0f;
    float maxSpeed = 5.0f;
    float maxSteeringAngle = 0.6f;
};

struct ObstacleDef {
    Vec2 position;
    float heading = 0.0f;
    std::vector<Vec2> vertices;
    uint32_t color = 0x888888FF;
};

struct DynamicObstacleDef {
    Vec2 position;
    float heading = 0.0f;
    std::vector<Vec2> vertices;
    uint32_t color = 0xFF4444FF;
    float speed = 1.5f;
    bool loop = true;
    std::vector<Vec2> waypoints;
};

class SceneLoader {
public:
    static void loadScene(entt::registry& registry, const std::string& configPath);

    static entt::entity createRobot(entt::registry& registry, const RobotDef& def);
    static entt::entity createStaticObstacle(entt::registry& registry, const ObstacleDef& def);
    static entt::entity createDynamicObstacle(entt::registry& registry, const DynamicObstacleDef& def);

private:
    static void computeWorldVertices(entt::registry& registry, entt::entity entity);
    static bool validatePolygon(const std::vector<Vec2>& vertices);
};

} // namespace teleop
