#include <benchmark/benchmark.h>
#include <entt/entt.hpp>
#include "components/transform.hpp"
#include "components/polygon_shape.hpp"
#include "components/velocity.hpp"
#include "components/robot_tag.hpp"
#include "components/static_tag.hpp"
#include "components/collision_state.hpp"
#include "components/score_state.hpp"
#include "components/render_meta.hpp"
#include "components/waypoint_path.hpp"
#include "physics/ackermann_model.hpp"
#include "engine/systems/kinematics_system.hpp"
#include "engine/systems/movement_system.hpp"
#include "engine/systems/transform_system.hpp"
#include "engine/systems/collision_system.hpp"
#include "engine/systems/scoring_system.hpp"
#include "interfaces/i_logger.hpp"
#include <random>
#include <cmath>

using namespace teleop;

namespace {

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

void setupBenchmarkScene(entt::registry& reg, int entityCount) {
    std::mt19937 rng(42);
    std::uniform_real_distribution<float> posDist(-40.0f, 40.0f);

    // Create robot
    auto robot = reg.create();
    reg.emplace<Transform>(robot, 0.0f, 0.0f, 0.0f);
    PolygonShape robotShape;
    robotShape.localVertices = {{-1, -0.5f}, {1, -0.5f}, {1, 0.5f}, {-1, 0.5f}};
    robotShape.worldVertices = robotShape.localVertices;
    robotShape.aabb = AABB::fromVertices(robotShape.worldVertices.data(), 4);
    reg.emplace<PolygonShape>(robot, std::move(robotShape));
    reg.emplace<Velocity>(robot, 2.0f, 0.0f);
    reg.emplace<RobotTag>(robot, 0.0f, 2.0f, 5.0f, 0.6f, 0.5f, 0.1f);
    reg.emplace<CollisionState>(robot);
    reg.emplace<RenderMeta>(robot, uint32_t{0x00FF00FF}, uint8_t{2});

    // Create obstacles
    for (int i = 1; i < entityCount; ++i) {
        auto entity = reg.create();
        float x = posDist(rng);
        float y = posDist(rng);
        reg.emplace<Transform>(entity, x, y, 0.0f);

        PolygonShape shape;
        shape.localVertices = {{-0.5f, -0.5f}, {0.5f, -0.5f}, {0.5f, 0.5f}, {-0.5f, 0.5f}};
        shape.worldVertices = {
            {x - 0.5f, y - 0.5f}, {x + 0.5f, y - 0.5f},
            {x + 0.5f, y + 0.5f}, {x - 0.5f, y + 0.5f}
        };
        shape.aabb = AABB::fromVertices(shape.worldVertices.data(), 4);
        reg.emplace<PolygonShape>(entity, std::move(shape));
        reg.emplace<StaticTag>(entity);
        reg.emplace<RenderMeta>(entity, uint32_t{0x888888FF}, uint8_t{1});
    }

    reg.ctx().emplace<ScoreState>();
}

} // namespace

static void BM_FullPhysicsTick(benchmark::State& state) {
    int entityCount = static_cast<int>(state.range(0));
    NullLogger logger;
    AckermannModel model(2.0f, 0.6f, 5.0f);

    for (auto _ : state) {
        state.PauseTiming();
        entt::registry reg;
        setupBenchmarkScene(reg, entityCount);
        KinematicsSystem kinematicsSystem(model);
        MovementSystem movementSystem;
        TransformSystem transformSystem;
        CollisionSystem collisionSystem(5.0f, logger);
        ScoringSystem scoringSystem(logger);
        state.ResumeTiming();

        kinematicsSystem.tick(reg, 0.016f);
        movementSystem.tick(reg, 0.016f);
        transformSystem.tick(reg);
        collisionSystem.tick(reg);
        scoringSystem.tick(reg);
    }
    state.SetItemsProcessed(state.iterations() * entityCount);
}
BENCHMARK(BM_FullPhysicsTick)->Range(20, 2000);
