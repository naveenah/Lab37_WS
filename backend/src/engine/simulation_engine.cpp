#include "engine/simulation_engine.hpp"
#include "components/transform.hpp"
#include "infra/scene_loader.hpp"
#include <chrono>
#include <thread>
#include <csignal>
#include <sstream>
#include <cmath>
#include <algorithm>

namespace teleop {

// Global pointer for signal handler
static std::atomic<bool>* g_running = nullptr;

static void signalHandler(int signal) {
    if (g_running) {
        g_running->store(false, std::memory_order_release);
    }
}

SimulationEngine::SimulationEngine(
    entt::registry& registry,
    ICommandSource& commandSource,
    IStateStore& stateStore,
    IMessageBus& messageBus,
    IRobotKinematics& kinematics,
    ILogger& logger,
    const Config& config
)
    : registry_(registry)
    , logger_(logger)
    , inputSystem_(commandSource, logger)
    , kinematicsSystem_(kinematics)
    , collisionSystem_(config.physics.spatialHashCellSize, logger)
    , scoringSystem_(logger)
    , broadcastSystem_(stateStore, messageBus)
    , loggingSystem_(logger, config.server.tickRate * 5)  // Log every 5 seconds
    , targetTickMs_(1000.0f / static_cast<float>(config.server.tickRate))
    , broadcastInterval_(config.server.tickRate / config.server.broadcastRate)
    , worldConfig_(config.world)
{
    if (broadcastInterval_ < 1) broadcastInterval_ = 1;
}

void SimulationEngine::run() {
    running_.store(true, std::memory_order_release);
    g_running = &running_;

    std::signal(SIGINT, signalHandler);
    std::signal(SIGTERM, signalHandler);

    logger_.info("Simulation engine starting at " +
                 std::to_string(static_cast<int>(1000.0f / targetTickMs_)) + " Hz");

    float dt = targetTickMs_ / 1000.0f;
    auto lastTime = std::chrono::steady_clock::now();
    float accumulator = 0.0f;

    while (running_.load(std::memory_order_acquire)) {
        auto currentTime = std::chrono::steady_clock::now();
        float frameTime = std::chrono::duration<float>(currentTime - lastTime).count();
        lastTime = currentTime;

        // Cap frame time to prevent spiral of death
        if (frameTime > 0.25f) {
            frameTime = 0.25f;
        }

        accumulator += frameTime;

        while (accumulator >= dt && running_.load(std::memory_order_acquire)) {
            tick(dt);
            accumulator -= dt;
        }

        // Sleep for remaining time
        auto tickEnd = std::chrono::steady_clock::now();
        float elapsed = std::chrono::duration<float>(tickEnd - currentTime).count();
        float sleepTime = dt - elapsed;
        if (sleepTime > 0.001f) {
            std::this_thread::sleep_for(
                std::chrono::microseconds(static_cast<int>(sleepTime * 1000000.0f))
            );
        }
    }

    logger_.info("Simulation engine stopped after " + std::to_string(tickNumber_) + " ticks");
}

void SimulationEngine::stop() {
    running_.store(false, std::memory_order_release);
}

void SimulationEngine::resetScene() {
    if (configPath_.empty()) {
        logger_.warn("Cannot reset: no config path set");
        return;
    }
    logger_.info("Resetting scene...");
    registry_.clear();
    SceneLoader::loadScene(registry_, configPath_);
    tickNumber_ = 0;
    broadcastCounter_ = 0;
    logger_.info("Scene reset complete");
}

void SimulationEngine::tick(float dt) {
    auto start = std::chrono::steady_clock::now();

    // Check for reset request
    if (resetRequested_.exchange(false, std::memory_order_acquire)) {
        resetScene();
        return; // Skip this tick, fresh scene next tick
    }

    // 1. Input
    inputSystem_.tick(registry_);

    // 2. Kinematics
    kinematicsSystem_.tick(registry_, dt);

    // 3. Movement
    movementSystem_.tick(registry_, dt);

    // 4. Transform
    transformSystem_.tick(registry_);

    // 5. Collision
    collisionSystem_.tick(registry_);

    // 6. Scoring
    scoringSystem_.tick(registry_);

    // 7. World bounds clamping
    auto clampView = registry_.view<Transform>();
    for (auto [entity, transform] : clampView.each()) {
        transform.x = std::clamp(transform.x,
            worldConfig_.minX + 0.1f, worldConfig_.maxX - 0.1f);
        transform.y = std::clamp(transform.y,
            worldConfig_.minY + 0.1f, worldConfig_.maxY - 0.1f);
    }

    // 8. Broadcast (at configured rate)
    broadcastCounter_++;
    if (broadcastCounter_ >= broadcastInterval_) {
        broadcastCounter_ = 0;
        broadcastSystem_.tick(registry_, tickNumber_);
    }

    auto elapsed = std::chrono::steady_clock::now() - start;
    float elapsedMs = std::chrono::duration<float, std::milli>(elapsed).count();

    // 9. Logging
    loggingSystem_.tick(registry_, elapsedMs, tickNumber_);

    // Tick overrun detection
    if (elapsedMs > targetTickMs_ * 1.5f) {
        logger_.warn("Tick overrun: " + std::to_string(elapsedMs) +
                     "ms (target: " + std::to_string(targetTickMs_) + "ms)");
    }

    tickNumber_++;
}

} // namespace teleop
