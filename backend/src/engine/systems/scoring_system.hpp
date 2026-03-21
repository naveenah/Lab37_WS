#pragma once

#include "interfaces/i_logger.hpp"
#include <entt/entt.hpp>

namespace teleop {

class ScoringSystem {
public:
    explicit ScoringSystem(ILogger& logger);
    void tick(entt::registry& registry);

private:
    ILogger& logger_;
};

} // namespace teleop
