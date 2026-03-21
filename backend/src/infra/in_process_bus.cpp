#include "infra/in_process_bus.hpp"

namespace teleop {

void InProcessBus::publish(Topic topic, std::span<const uint8_t> data) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = subscribers_.find(topic);
    if (it != subscribers_.end()) {
        for (auto& callback : it->second) {
            callback(data);
        }
    }
}

void InProcessBus::subscribe(Topic topic, MessageCallback callback) {
    std::lock_guard<std::mutex> lock(mutex_);
    subscribers_[topic].push_back(std::move(callback));
}

} // namespace teleop
