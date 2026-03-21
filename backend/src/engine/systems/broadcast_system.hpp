#pragma once

#include "interfaces/i_state_store.hpp"
#include "interfaces/i_message_bus.hpp"
#include "protocol/state_serializer.hpp"
#include <entt/entt.hpp>

namespace teleop {

class BroadcastSystem {
public:
    BroadcastSystem(IStateStore& stateStore, IMessageBus& messageBus);
    void tick(entt::registry& registry, uint64_t tickNumber);

private:
    IStateStore& stateStore_;
    IMessageBus& messageBus_;
    StateSerializer serializer_;
    uint32_t stateSequence_ = 0;
};

} // namespace teleop
