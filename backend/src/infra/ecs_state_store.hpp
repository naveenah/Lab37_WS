#pragma once

#include "interfaces/i_state_store.hpp"
#include <entt/entt.hpp>

namespace teleop {

class ECSStateStore : public IStateStore {
public:
    explicit ECSStateStore(entt::registry& registry);
    ~ECSStateStore() override = default;

    std::vector<EntitySnapshot> getEntitySnapshots() const override;
    ScoreState getScoreState() const override;

private:
    entt::registry& registry_;
};

} // namespace teleop
