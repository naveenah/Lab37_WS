#pragma once

#include "protocol/entity_snapshot.hpp"
#include "components/score_state.hpp"
#include <vector>

namespace teleop {

class IStateStore {
public:
    virtual ~IStateStore() = default;

    virtual std::vector<EntitySnapshot> getEntitySnapshots() const = 0;
    virtual ScoreState getScoreState() const = 0;
};

} // namespace teleop
