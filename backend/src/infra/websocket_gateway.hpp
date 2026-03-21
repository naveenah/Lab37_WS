#pragma once

#include "infra/spsc_command_source.hpp"
#include "interfaces/i_message_bus.hpp"
#include "interfaces/i_logger.hpp"
#include "infra/token_bucket.hpp"
#include "infra/config_loader.hpp"
#include "protocol/command_validator.hpp"
#include <atomic>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>
#include <mutex>

// Forward declarations for uWebSockets
struct us_listen_socket_t;
struct us_timer_t;

namespace teleop {

class WebSocketGateway {
public:
    WebSocketGateway(
        uint16_t port,
        SPSCCommandSource& commandSource,
        IMessageBus& messageBus,
        ILogger& logger,
        const Config& config,
        std::function<void()> onReset = nullptr
    );

    ~WebSocketGateway();

    void run();   // Blocks — run on network thread
    void stop();

private:
    void setupBroadcastSubscription();

    uint16_t port_;
    SPSCCommandSource& commandSource_;
    IMessageBus& messageBus_;
    ILogger& logger_;
    TokenBucket rateLimiter_;

    int tickRate_;
    int broadcastRate_;

    std::atomic<bool> running_{false};
    struct us_listen_socket_t* listenSocket_ = nullptr;
    struct us_timer_t* broadcastTimer_ = nullptr;

    // Broadcast buffer
    // Broadcast buffer (written by sim thread, read by network thread timer)
    std::vector<uint8_t> broadcastBuffer_;
    std::mutex broadcastMutex_;
    std::atomic<bool> hasPendingBroadcast_{false};

    // Reset callback
    std::function<void()> onReset_;

    // Pointer to app used for publish — only valid during run()
    void* app_ = nullptr;
};

} // namespace teleop
