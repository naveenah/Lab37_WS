#pragma once

#include "interfaces/i_logger.hpp"
#include <entt/entt.hpp>
#include <chrono>

namespace teleop {

class LoggingSystem {
public:
    explicit LoggingSystem(ILogger& logger, int logIntervalTicks = 300);
    void tick(entt::registry& registry, float tickDurationMs, uint64_t tickNumber);

private:
    ILogger& logger_;
    int logIntervalTicks_;
    int tickCounter_ = 0;
};

} // namespace teleop
