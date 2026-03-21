#include <gtest/gtest.h>
#include <entt/entt.hpp>
#include "components/transform.hpp"
#include "components/polygon_shape.hpp"
#include "components/velocity.hpp"
#include "components/robot_tag.hpp"
#include "components/static_tag.hpp"
#include "components/collision_state.hpp"
#include "components/score_state.hpp"
#include "components/render_meta.hpp"
#include "components/session_state.hpp"
#include "physics/ackermann_model.hpp"
#include "physics/sat_detector.hpp"
#include "physics/spatial_hash_grid.hpp"
#include "engine/systems/kinematics_system.hpp"
#include "engine/systems/transform_system.hpp"
#include "engine/systems/collision_system.hpp"
#include "engine/systems/scoring_system.hpp"
#include "infra/spdlog_logger.hpp"
#include <cmath>

using namespace teleop;

namespace {

// Null logger for tests
class NullLogger : public ILogger {
public:
    void trace(std::string_view) override {}
    void debug(std::string_view) override {}
    void info(std::string_view) override {}
    void warn(std::string_view) override {}
    void error(std::string_view) override {}
    void critical(std::string_view) override {}
    void event(std::string_view, std::string_view) override {}
};

entt::entity createTestRobot(entt::registry& reg, float x, float y) {
    auto entity = reg.create();
    reg.emplace<Transform>(entity, x, y, 0.0f);

    PolygonShape shape;
    shape.localVertices = {{-1, -0.5f}, {1, -0.5f}, {1, 0.5f}, {-1, 0.5f}};
    shape.worldVertices.resize(4);
    reg.emplace<PolygonShape>(entity, std::move(shape));

    reg.emplace<Velocity>(entity, 5.0f, 0.0f);
    reg.emplace<RobotTag>(entity, 0.0f, 2.0f, 5.0f, 0.6f, 1.0f, 0.0f);
    reg.emplace<CollisionState>(entity);
    reg.emplace<RenderMeta>(entity, uint32_t{0x00FF00FF}, uint8_t{2});
    return entity;
}

entt::entity createTestWall(entt::registry& reg, float x, float y) {
    auto entity = reg.create();
    reg.emplace<Transform>(entity, x, y, 0.0f);

    PolygonShape shape;
    shape.localVertices = {{-1, -5}, {1, -5}, {1, 5}, {-1, 5}};
    shape.worldVertices.resize(4);
    reg.emplace<PolygonShape>(entity, std::move(shape));

    reg.emplace<StaticTag>(entity);
    reg.emplace<RenderMeta>(entity, uint32_t{0x888888FF}, uint8_t{1});
    return entity;
}

void computeWorldVertices(entt::registry& reg, entt::entity entity) {
    auto& transform = reg.get<Transform>(entity);
    auto& shape = reg.get<PolygonShape>(entity);
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

} // namespace

TEST(CollisionFlowTest, RobotCollisionZerosVelocityAndIncrementsCounter) {
    entt::registry reg;
    NullLogger logger;

    // Robot at origin heading east, wall at x=5
    auto robot = createTestRobot(reg, 0, 0);
    auto wall = createTestWall(reg, 5, 0);

    // Initialize singleton
    reg.ctx().emplace<ScoreState>();

    // Compute initial world vertices
    computeWorldVertices(reg, robot);
    computeWorldVertices(reg, wall);

    AckermannModel model(2.0f, 0.6f, 5.0f);
    KinematicsSystem kinematicsSystem(model);
    TransformSystem transformSystem;
    CollisionSystem collisionSystem(5.0f, logger);
    ScoringSystem scoringSystem(logger);

    // Run ticks until collision happens
    bool collisionOccurred = false;
    for (int i = 0; i < 120; ++i) {
        kinematicsSystem.tick(reg, 0.016f);
        transformSystem.tick(reg);
        collisionSystem.tick(reg);
        scoringSystem.tick(reg);

        auto& vel = reg.get<Velocity>(robot);
        if (vel.speed == 0.0f && i > 5) {
            collisionOccurred = true;
            break;
        }
    }

    EXPECT_TRUE(collisionOccurred);
    EXPECT_FLOAT_EQ(reg.get<Velocity>(robot).speed, 0.0f);
    EXPECT_GE(reg.ctx().get<ScoreState>().impactCount, 1u);
}

TEST(CollisionFlowTest, ImpactCounterIncrementsOnlyOnce) {
    entt::registry reg;
    NullLogger logger;

    // Robot very close to wall
    auto robot = createTestRobot(reg, 2.5f, 0);
    auto wall = createTestWall(reg, 5, 0);

    reg.ctx().emplace<ScoreState>();
    computeWorldVertices(reg, robot);
    computeWorldVertices(reg, wall);

    AckermannModel model(2.0f, 0.6f, 5.0f);
    KinematicsSystem kinematicsSystem(model);
    TransformSystem transformSystem;
    CollisionSystem collisionSystem(5.0f, logger);
    ScoringSystem scoringSystem(logger);

    // Run many ticks
    for (int i = 0; i < 200; ++i) {
        kinematicsSystem.tick(reg, 0.016f);
        transformSystem.tick(reg);
        collisionSystem.tick(reg);
        scoringSystem.tick(reg);
    }

    // Impact counter should be exactly 1 (collision detected once)
    EXPECT_EQ(reg.ctx().get<ScoreState>().impactCount, 1u);
}
