#include "infra/scene_loader.hpp"
#include "components/transform.hpp"
#include "components/polygon_shape.hpp"
#include "components/velocity.hpp"
#include "components/robot_tag.hpp"
#include "components/static_tag.hpp"
#include "components/waypoint_path.hpp"
#include "components/collision_state.hpp"
#include "components/render_meta.hpp"
#include "components/score_state.hpp"
#include "components/session_state.hpp"
#include <nlohmann/json.hpp>
#include <fstream>
#include <cmath>
#include <stdexcept>
#include <chrono>

namespace teleop {

using json = nlohmann::json;

void SceneLoader::loadScene(entt::registry& registry, const std::string& configPath) {
    std::ifstream file(configPath);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open config file: " + configPath);
    }

    json config = json::parse(file);
    const auto& scene = config["scene"];

    // Create robot
    const auto& robotJson = scene["robot"];
    RobotDef robotDef;
    robotDef.position = {robotJson["position"]["x"], robotJson["position"]["y"]};
    robotDef.heading = robotJson.value("heading", 0.0f);
    for (const auto& v : robotJson["vertices"]) {
        robotDef.vertices.push_back({v["x"], v["y"]});
    }
    robotDef.color = std::stoul(robotJson.value("color", "0x00FF00FF"), nullptr, 16);
    robotDef.wheelbase = robotJson.value("wheelbase", 2.0f);
    robotDef.maxSpeed = robotJson.value("maxSpeed", 5.0f);
    robotDef.maxSteeringAngle = robotJson.value("maxSteeringAngle", 0.6f);
    createRobot(registry, robotDef);

    // Create static obstacles
    if (scene.contains("staticObstacles")) {
        for (const auto& obsJson : scene["staticObstacles"]) {
            ObstacleDef def;
            def.position = {obsJson["position"]["x"], obsJson["position"]["y"]};
            def.heading = obsJson.value("heading", 0.0f);
            for (const auto& v : obsJson["vertices"]) {
                def.vertices.push_back({v["x"], v["y"]});
            }
            def.color = std::stoul(obsJson.value("color", "0x888888FF"), nullptr, 16);
            createStaticObstacle(registry, def);
        }
    }

    // Create dynamic obstacles
    if (scene.contains("dynamicObstacles")) {
        for (const auto& dynJson : scene["dynamicObstacles"]) {
            DynamicObstacleDef def;
            def.position = {dynJson["position"]["x"], dynJson["position"]["y"]};
            def.heading = dynJson.value("heading", 0.0f);
            for (const auto& v : dynJson["vertices"]) {
                def.vertices.push_back({v["x"], v["y"]});
            }
            def.color = std::stoul(dynJson.value("color", "0xFF4444FF"), nullptr, 16);
            def.speed = dynJson.value("speed", 1.5f);
            def.loop = dynJson.value("loop", true);
            for (const auto& wp : dynJson["waypoints"]) {
                def.waypoints.push_back({wp["x"], wp["y"]});
            }
            createDynamicObstacle(registry, def);
        }
    }

    // Initialize singleton components
    auto now = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()
    ).count();
    registry.ctx().emplace<ScoreState>(ScoreState{0, static_cast<uint64_t>(now)});
    registry.ctx().emplace<SessionState>();
}

entt::entity SceneLoader::createRobot(entt::registry& registry, const RobotDef& def) {
    if (!validatePolygon(def.vertices)) {
        throw std::runtime_error("Invalid robot polygon: requires at least 3 non-degenerate vertices");
    }

    auto entity = registry.create();
    registry.emplace<Transform>(entity, def.position.x, def.position.y, def.heading);

    PolygonShape shape;
    shape.localVertices = def.vertices;
    shape.worldVertices.resize(def.vertices.size());
    registry.emplace<PolygonShape>(entity, std::move(shape));

    registry.emplace<Velocity>(entity);
    registry.emplace<RobotTag>(entity, 0.0f, def.wheelbase, def.maxSpeed, def.maxSteeringAngle, 0.0f, 0.0f);
    registry.emplace<CollisionState>(entity);
    registry.emplace<RenderMeta>(entity, def.color, static_cast<uint8_t>(2));

    computeWorldVertices(registry, entity);
    return entity;
}

entt::entity SceneLoader::createStaticObstacle(entt::registry& registry, const ObstacleDef& def) {
    if (!validatePolygon(def.vertices)) {
        throw std::runtime_error("Invalid static obstacle polygon: requires at least 3 non-degenerate vertices");
    }

    auto entity = registry.create();
    registry.emplace<Transform>(entity, def.position.x, def.position.y, def.heading);

    PolygonShape shape;
    shape.localVertices = def.vertices;
    shape.worldVertices.resize(def.vertices.size());
    registry.emplace<PolygonShape>(entity, std::move(shape));

    registry.emplace<StaticTag>(entity);
    registry.emplace<RenderMeta>(entity, def.color, static_cast<uint8_t>(1));

    computeWorldVertices(registry, entity);
    return entity;
}

entt::entity SceneLoader::createDynamicObstacle(entt::registry& registry, const DynamicObstacleDef& def) {
    if (!validatePolygon(def.vertices)) {
        throw std::runtime_error("Invalid dynamic obstacle polygon: requires at least 3 non-degenerate vertices");
    }

    auto entity = registry.create();
    registry.emplace<Transform>(entity, def.position.x, def.position.y, def.heading);

    PolygonShape shape;
    shape.localVertices = def.vertices;
    shape.worldVertices.resize(def.vertices.size());
    registry.emplace<PolygonShape>(entity, std::move(shape));

    registry.emplace<Velocity>(entity, def.speed, 0.0f);
    registry.emplace<WaypointPath>(entity, def.waypoints, size_t{0}, def.speed, def.loop);
    registry.emplace<RenderMeta>(entity, def.color, static_cast<uint8_t>(1));

    computeWorldVertices(registry, entity);
    return entity;
}

void SceneLoader::computeWorldVertices(entt::registry& registry, entt::entity entity) {
    auto& transform = registry.get<Transform>(entity);
    auto& shape = registry.get<PolygonShape>(entity);

    float cosH = std::cos(transform.heading);
    float sinH = std::sin(transform.heading);

    for (size_t i = 0; i < shape.localVertices.size(); ++i) {
        const auto& local = shape.localVertices[i];
        shape.worldVertices[i] = {
            transform.x + local.x * cosH - local.y * sinH,
            transform.y + local.x * sinH + local.y * cosH,
        };
    }

    shape.aabb = AABB::fromVertices(shape.worldVertices.data(), shape.worldVertices.size());
}

bool SceneLoader::validatePolygon(const std::vector<Vec2>& vertices) {
    if (vertices.size() < 3) return false;

    // Check for degenerate (zero-area) polygon by checking if all points are collinear
    for (size_t i = 2; i < vertices.size(); ++i) {
        Vec2 edge1 = vertices[1] - vertices[0];
        Vec2 edge2 = vertices[i] - vertices[0];
        if (std::abs(edge1.cross(edge2)) > 1e-6f) {
            return true; // Found a non-collinear point
        }
    }
    return false; // All points are collinear
}

} // namespace teleop
