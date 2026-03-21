#include "infra/config_loader.hpp"
#include <nlohmann/json.hpp>
#include <fstream>
#include <cstdlib>
#include <stdexcept>

namespace teleop {

using json = nlohmann::json;

Config ConfigLoader::fromFile(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open config file: " + path);
    }

    json j = json::parse(file);
    Config config;

    if (j.contains("server")) {
        auto& s = j["server"];
        config.server.port = s.value("port", 9001);
        config.server.tickRate = s.value("tickRate", 60);
        config.server.broadcastRate = s.value("broadcastRate", 30);
    }

    if (j.contains("logging")) {
        auto& l = j["logging"];
        config.logging.level = l.value("level", "INFO");
        config.logging.logDir = l.value("logDir", "./logs");
        config.logging.maxFileSize = l.value("maxFileSize", 100 * 1024 * 1024);
        config.logging.maxFiles = l.value("maxFiles", 10);
    }

    if (j.contains("world")) {
        auto& w = j["world"];
        config.world.minX = w.value("minX", -50.0f);
        config.world.maxX = w.value("maxX", 50.0f);
        config.world.minY = w.value("minY", -50.0f);
        config.world.maxY = w.value("maxY", 50.0f);
    }

    if (j.contains("physics")) {
        auto& p = j["physics"];
        config.physics.spatialHashCellSize = p.value("spatialHashCellSize", 5.0f);
    }

    if (j.contains("rateLimiting")) {
        auto& r = j["rateLimiting"];
        config.rateLimiting.maxTokens = r.value("maxTokens", 20.0f);
        config.rateLimiting.refillRate = r.value("refillRate", 15.0f);
    }

    config.scenePath = path;

    return config;
}

Config ConfigLoader::fromEnv() {
    Config config;

    if (const char* port = std::getenv("TELEOP_PORT"))
        config.server.port = static_cast<uint16_t>(std::stoi(port));
    if (const char* tickRate = std::getenv("TELEOP_TICK_RATE"))
        config.server.tickRate = std::stoi(tickRate);
    if (const char* broadcastRate = std::getenv("TELEOP_BROADCAST_RATE"))
        config.server.broadcastRate = std::stoi(broadcastRate);
    if (const char* logLevel = std::getenv("TELEOP_LOG_LEVEL"))
        config.logging.level = logLevel;

    return config;
}

bool ConfigLoader::validate(const Config& config) {
    if (config.server.port == 0) return false;
    if (config.server.tickRate <= 0 || config.server.tickRate > 240) return false;
    if (config.server.broadcastRate <= 0 || config.server.broadcastRate > config.server.tickRate) return false;
    if (config.world.minX >= config.world.maxX) return false;
    if (config.world.minY >= config.world.maxY) return false;
    if (config.physics.spatialHashCellSize <= 0.0f) return false;
    return true;
}

} // namespace teleop
