#pragma once

#include "interfaces/i_message_bus.hpp"
#include <unordered_map>
#include <vector>
#include <mutex>

namespace teleop {

class InProcessBus : public IMessageBus {
public:
    InProcessBus() = default;
    ~InProcessBus() override = default;

    void publish(Topic topic, std::span<const uint8_t> data) override;
    void subscribe(Topic topic, MessageCallback callback) override;

private:
    std::unordered_map<Topic, std::vector<MessageCallback>> subscribers_;
    std::mutex mutex_;
};

} // namespace teleop
