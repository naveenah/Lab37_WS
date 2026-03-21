#pragma once

#include <string>
#include <cstdint>

namespace teleop {

struct ServerConfig {
    uint16_t port = 9001;
    int tickRate = 60;
    int broadcastRate = 30;
};

struct LoggingConfig {
    std::string level = "INFO";
    std::string logDir = "./logs";
    size_t maxFileSize = 100 * 1024 * 1024;
    int maxFiles = 10;
};

struct WorldConfig {
    float minX = -50.0f;
    float maxX = 50.0f;
    float minY = -50.0f;
    float maxY = 50.0f;
};

struct PhysicsConfig {
    float spatialHashCellSize = 5.0f;
};

struct RateLimitConfig {
    float maxTokens = 20.0f;
    float refillRate = 15.0f;
};

struct Config {
    ServerConfig server;
    LoggingConfig logging;
    WorldConfig world;
    PhysicsConfig physics;
    RateLimitConfig rateLimiting;
    std::string scenePath;  // Path to scene data (or empty if inline)
};

class ConfigLoader {
public:
    static Config fromFile(const std::string& path);
    static Config fromEnv();
    static bool validate(const Config& config);
};

} // namespace teleop
