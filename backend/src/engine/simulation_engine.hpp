#pragma once

#include "interfaces/i_message_bus.hpp"
#include "interfaces/i_state_store.hpp"
#include "interfaces/i_command_source.hpp"
#include "interfaces/i_collision_detector.hpp"
#include "interfaces/i_robot_kinematics.hpp"
#include "interfaces/i_logger.hpp"
#include "engine/systems/input_system.hpp"
#include "engine/systems/kinematics_system.hpp"
#include "engine/systems/movement_system.hpp"
#include "engine/systems/transform_system.hpp"
#include "engine/systems/collision_system.hpp"
#include "engine/systems/scoring_system.hpp"
#include "engine/systems/broadcast_system.hpp"
#include "engine/systems/logging_system.hpp"
#include "infra/config_loader.hpp"
#include <entt/entt.hpp>
#include <atomic>
#include <memory>

namespace teleop {

class SimulationEngine {
public:
    SimulationEngine(
        entt::registry& registry,
        ICommandSource& commandSource,
        IStateStore& stateStore,
        IMessageBus& messageBus,
        IRobotKinematics& kinematics,
        ILogger& logger,
        const Config& config
    );

    void run();  // Blocks until stop() is called
    void stop();
    void setConfigPath(const std::string& path) { configPath_ = path; }
    void requestReset() { resetRequested_.store(true, std::memory_order_release); }

    entt::registry& getRegistry() { return registry_; }
    uint64_t getTickNumber() const { return tickNumber_; }

private:
    void tick(float dt);
    void resetScene();

    entt::registry& registry_;
    ILogger& logger_;
    std::string configPath_;

    InputSystem inputSystem_;
    KinematicsSystem kinematicsSystem_;
    MovementSystem movementSystem_;
    TransformSystem transformSystem_;
    CollisionSystem collisionSystem_;
    ScoringSystem scoringSystem_;
    BroadcastSystem broadcastSystem_;
    LoggingSystem loggingSystem_;

    float targetTickMs_;
    int broadcastInterval_;
    int broadcastCounter_ = 0;
    uint64_t tickNumber_ = 0;
    std::atomic<bool> running_{false};
    std::atomic<bool> resetRequested_{false};

    WorldConfig worldConfig_;
};

} // namespace teleop
