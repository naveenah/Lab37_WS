#pragma once

#include <cstdint>
#include <functional>
#include <span>
#include <string>

namespace teleop {

enum class Topic : uint8_t {
    StateBroadcast = 0,
    CollisionEvent = 1,
    ConnectionEvent = 2,
};

using MessageCallback = std::function<void(std::span<const uint8_t>)>;

class IMessageBus {
public:
    virtual ~IMessageBus() = default;

    virtual void publish(Topic topic, std::span<const uint8_t> data) = 0;
    virtual void subscribe(Topic topic, MessageCallback callback) = 0;
};

} // namespace teleop
