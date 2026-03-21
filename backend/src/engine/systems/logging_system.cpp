#include "engine/systems/logging_system.hpp"
#include "components/score_state.hpp"
#include "components/session_state.hpp"
#include "components/transform.hpp"
#include <nlohmann/json.hpp>
#include <sstream>

namespace teleop {

LoggingSystem::LoggingSystem(ILogger& logger, int logIntervalTicks)
    : logger_(logger), logIntervalTicks_(logIntervalTicks) {}

void LoggingSystem::tick(entt::registry& registry, float tickDurationMs, uint64_t tickNumber) {
    tickCounter_++;
    if (tickCounter_ < logIntervalTicks_) return;
    tickCounter_ = 0;

    nlohmann::json data;
    data["tick_number"] = tickNumber;
    data["tick_duration_ms"] = tickDurationMs;

    auto entityCount = registry.view<const Transform>().size();
    data["entity_count"] = entityCount;

    if (registry.ctx().contains<ScoreState>()) {
        data["impact_count"] = registry.ctx().get<ScoreState>().impactCount;
    }

    if (registry.ctx().contains<SessionState>()) {
        data["operator_connected"] = registry.ctx().get<SessionState>().operatorConnected;
    }

    logger_.event("physics", data.dump());
}

} // namespace teleop
