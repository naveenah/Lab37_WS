// main.cpp — Composition root
// The ONLY file that knows about concrete implementations

#include "infra/config_loader.hpp"
#include "infra/spdlog_logger.hpp"
#include "infra/in_process_bus.hpp"
#include "infra/spsc_command_source.hpp"
#include "infra/ecs_state_store.hpp"
#include "infra/scene_loader.hpp"
#include "infra/websocket_gateway.hpp"
#include "physics/ackermann_model.hpp"
#include "engine/simulation_engine.hpp"
#include <entt/entt.hpp>
#include <cstdlib>
#include <thread>
#include <iostream>
#include <string>

int main(int argc, char* argv[]) {
    try {
        // Parse command-line arguments
        std::string configPath = "config/default.json";
        for (int i = 1; i < argc; ++i) {
            std::string arg = argv[i];
            if (arg == "--config" && i + 1 < argc) {
                configPath = argv[++i];
            }
        }

        // Load configuration
        auto config = teleop::ConfigLoader::fromFile(configPath);

        // Override from environment variables
        auto envConfig = teleop::ConfigLoader::fromEnv();
        if (const char* port = std::getenv("TELEOP_PORT"))
            config.server.port = envConfig.server.port;
        if (const char* level = std::getenv("TELEOP_LOG_LEVEL"))
            config.logging.level = envConfig.logging.level;
        if (const char* tickRate = std::getenv("TELEOP_TICK_RATE"))
            config.server.tickRate = envConfig.server.tickRate;
        if (const char* broadcastRate = std::getenv("TELEOP_BROADCAST_RATE"))
            config.server.broadcastRate = envConfig.server.broadcastRate;

        if (!teleop::ConfigLoader::validate(config)) {
            std::cerr << "Invalid configuration" << std::endl;
            return 1;
        }

        // Create infrastructure
        auto logger = std::make_unique<teleop::SpdlogLogger>(
            "teleop", config.logging.level, config.logging.logDir
        );
        logger->info("Robot Teleoperation Simulator v1.0.0");
        logger->info("Loading configuration from: " + configPath);

        auto bus = std::make_unique<teleop::InProcessBus>();
        auto commandSource = std::make_unique<teleop::SPSCCommandSource>(1024);
        auto kinematics = std::make_unique<teleop::AckermannModel>(2.0f, 0.6f, 5.0f);

        // Create ECS registry and load scene
        entt::registry registry;
        auto stateStore = std::make_unique<teleop::ECSStateStore>(registry);

        logger->info("Loading scene...");
        teleop::SceneLoader::loadScene(registry, configPath);
        logger->info("Scene loaded successfully");

        // Create simulation engine
        teleop::SimulationEngine engine(
            registry,
            *commandSource,
            *stateStore,
            *bus,
            *kinematics,
            *logger,
            config
        );

        engine.setConfigPath(configPath);

        // Create and start WebSocket gateway on network thread
        teleop::WebSocketGateway gateway(
            config.server.port,
            *commandSource,
            *bus,
            *logger,
            config,
            [&engine]() { engine.requestReset(); }
        );

        std::thread networkThread([&gateway]() {
            gateway.run();
        });

        logger->info("WebSocket gateway started on port " + std::to_string(config.server.port));
        logger->info("Simulation starting...");

        // Run simulation on main thread (blocks until SIGINT/SIGTERM)
        engine.run();

        // Shutdown
        logger->info("Shutting down...");
        gateway.stop();
        if (networkThread.joinable()) {
            networkThread.join();
        }
        logger->info("Shutdown complete");

        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return 1;
    }
}
