#pragma once

#include "protocol/entity_snapshot.hpp"
#include "components/score_state.hpp"
#include <flatbuffers/flatbuffers.h>
#include <vector>
#include <cstdint>

namespace teleop {

class StateSerializer {
public:
    // Serialize world state into a FlatBuffers buffer
    // Returns the buffer data and size
    std::pair<const uint8_t*, size_t> serialize(
        const std::vector<EntitySnapshot>& entities,
        const ScoreState& score,
        uint32_t sequence,
        uint64_t tickNumber,
        float robotSpeed,
        float robotSteering
    );

    // Get the underlying builder for advanced usage
    flatbuffers::FlatBufferBuilder& getBuilder() { return builder_; }

private:
    flatbuffers::FlatBufferBuilder builder_{1024};
};

} // namespace teleop
